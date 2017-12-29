//+
// File : DesSerPrePC.cc
// Description : Module to receive data from eb0 and send it to eb1tx
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/DesSerPrePC.h>
#include <rawdata/dataobjects/RawFTSWFormat.h>
#include <rawdata/dataobjects/RawTLUFormat.h>

//#define NO_DATA_CHECK
//#define DUMHSLB

using namespace std;
using namespace Belle2;

//----------------------------------------------------------------
//                 Implementation
//----------------------------------------------------------------
DesSerPrePC::DesSerPrePC(string host_recv, int port_recv, string host_send, int port_send, int shmflag,
                         const std::string& nodename, int nodeid)
{

  for (int i = 0 ; i < m_num_connections; i++) {
    //  m_hostname_from.push_back( "localhost");
    m_hostname_from.push_back(host_recv);
    //  m_port_from.push_back(33000);
    m_port_from.push_back(port_recv) ;
    m_socket_recv.push_back(-1);
  }

  //  m_port_to = 34001;
  m_port_to = port_send;
  //  m_hostname_local = "localhost";
  m_hostname_local = host_send;
  m_nodename = nodename;

  m_shmflag = shmflag;

  //  B2INFO("DeSerializerPrePC: Constructor done.");
  printf("[INFO] DeSerializerPrePC: Constructor done.\n"); fflush(stdout);
}



DesSerPrePC::~DesSerPrePC()
{
}



int DesSerPrePC::recvFD(int sock, char* buf, int data_size_byte, int flag)
{
  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = recv(sock, (char*)buf + n, data_size_byte - n , flag)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
#ifdef NONSTOP
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;
      } else {
        perror("[WARNING]");
        char err_buf[500];
        sprintf(err_buf, "recv() returned error; ret = %d. : %s %s %d",
                read_size, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
        m_run_error = 1;
        //        B2ERROR(err_buf);
        printf("[WARNING] %s\n", err_buf); fflush(stdout);
        string err_str = "RUN_ERROR";
        printf("AIUEO********************\n"); fflush(stdout);
        throw (err_str);
#endif
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else if (read_size == 0) {
      // Connection is closed ( error )
      char err_buf[500];
      sprintf(err_buf, "[WARNING] Connection is closed by peer(%s). readsize = %d %d. : %s %s %d",
              strerror(errno), read_size, errno, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
      m_run_error = 1;
      //      B2ERROR(err_buf);
      printf("%s\n", err_buf); fflush(stdout);
      string err_str = "RUN_ERROR";
      throw (err_str);
#else
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
#endif
    } else {
      n += read_size;
      if (n == data_size_byte)break;
    }
  }
  return n;
}


int DesSerPrePC::Connect()
{

  for (int i = 0; i < m_num_connections; i++) {

    if (m_socket_recv[ i ] >= 0) continue;   // Already have an established socket

    //
    // Connect to a downstream node
    //
    struct hostent* host;
    host = gethostbyname(m_hostname_from[ i ].c_str());
    if (host == NULL) {
      char err_buf[100];
      sprintf(err_buf, "[FATAL] hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...", m_hostname_from[ i ].c_str(),
              strerror(errno));
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    }

    struct sockaddr_in socPC;
    socPC.sin_family = AF_INET;
    socPC.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    socPC.sin_port = htons(m_port_from[ i ]);
    int sd = socket(PF_INET, SOCK_STREAM, 0);
    int val1 = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, (socklen_t)sizeof(timeout));

    printf("[DEBUG] Connecting to %s port %d\n" , m_hostname_from[ i ].c_str(), m_port_from[ i ]); fflush(stdout);

    while (1) {
      if (connect(sd, (struct sockaddr*)(&socPC), sizeof(socPC)) < 0) {
        perror("Failed to connect. Retrying...");
        usleep(500000);
      } else {
        //        B2INFO("Done");
        printf("[DEBUG] Done\n"); fflush(stdout);
        break;
      }
    }
    //    m_socket_recv.push_back(sd);
    m_socket_recv[ i ] = sd;

    // check socket paramters
    int val, len;
    len = sizeof(val);
    getsockopt(m_socket_recv[ i ], SOL_SOCKET, SO_RCVBUF, &val, (socklen_t*)&len);
    //    B2INFO("SO_RCVBUF" << val);
    getsockopt(m_socket_recv[ i ], SOL_SOCKET, SO_SNDBUF, &val, (socklen_t*)&len);
    //    B2DEBUG("SO_SNDBUF" <<  val);
    getsockopt(m_socket_recv[ i ], IPPROTO_TCP, TCP_MAXSEG, &val, (socklen_t*)&len);
    //    B2DEBUG("TCP_MAXSEG" <<  val);
    getsockopt(m_socket_recv[ i ], IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t*)&len);
    //    B2DEBUG("TCP_NODELAY" <<  val);

    if (m_status.isAvailable()) {
      sockaddr_in sa;
      memset(&sa, 0, sizeof(sockaddr_in));
      socklen_t sa_len = sizeof(sa);
      if (getsockname(m_socket_recv[i], (struct sockaddr*)&sa, (socklen_t*)&sa_len) == 0) {
        m_status.setInputPort(ntohs(sa.sin_port));
        m_status.setInputAddress(sa.sin_addr.s_addr);
      }
    }

  }
  //  B2INFO("[DEBUG] Initialization finished");
  printf("[DEBUG] Initialization finished\n"); fflush(stdout);
  return 0;
}



int* DesSerPrePC::recvData(int* delete_flag, int* total_buf_nwords, int* num_events_in_sendblock, int* num_nodes_in_sendblock)
{
  int* temp_buf = NULL; // buffer for data-body
  int flag = 0;

  vector <int> each_buf_nwords;
  each_buf_nwords.clear();
  vector <int> each_buf_nodes;
  each_buf_nodes.clear();
  vector <int> each_buf_events;
  each_buf_events.clear();

  *total_buf_nwords = 0;
  *num_nodes_in_sendblock = 0;
  *num_events_in_sendblock = 0;

  //
  // Read Header and obtain data size
  //
  int send_hdr_buf[ SendHeader::SENDHDR_NWORDS ];
  int temp_num_events = 0;
  int temp_num_nodes = 0;

  // Read header
  for (int i = 0; i < (int)(m_socket_recv.size()); i++) {

    recvFD(m_socket_recv[ i ], (char*)send_hdr_buf, sizeof(int)*SendHeader::SENDHDR_NWORDS, flag);

    SendHeader send_hdr;
    send_hdr.SetBuffer(send_hdr_buf);

    temp_num_events = send_hdr.GetNumEventsinPacket();
    temp_num_nodes = send_hdr.GetNumNodesinPacket();

    if (i == 0) {
      *num_events_in_sendblock = temp_num_events;
    } else if (*num_events_in_sendblock != temp_num_events) {
#ifndef NO_DATA_CHECK
      char err_buf[500];
      sprintf(err_buf,
              "[FATAL] CORRUPTED DATA: Different # of events or nodes in SendBlocks( # of eve : %d(socket 0) %d(socket %d), # of nodes: %d(socket 0) %d(socket %d). Exiting...\n",
              *num_events_in_sendblock , temp_num_events, i,  *num_nodes_in_sendblock , temp_num_nodes, i);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
#endif
    }

    *num_nodes_in_sendblock += temp_num_nodes;

    int rawblk_nwords = send_hdr.GetTotalNwords()
                        - SendHeader::SENDHDR_NWORDS
                        - SendTrailer::SENDTRL_NWORDS;
    *total_buf_nwords += rawblk_nwords;

    //
    // Data size check1
    //
    if (rawblk_nwords > (int)(2.5e6) || rawblk_nwords <= 0) {
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      char err_buf[500];
      sprintf(err_buf, "CORRUPTED DATA: Too large event : Header %d %d %d %d :block size %d words\n", i, temp_num_events, temp_num_nodes,
              send_hdr.GetTotalNwords(), rawblk_nwords);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(123456);
      exit(1);

    }

    each_buf_nwords.push_back(rawblk_nwords);
    each_buf_events.push_back(temp_num_events);
    each_buf_nodes.push_back(temp_num_nodes);

  }


  temp_buf = getNewBuffer(*total_buf_nwords, delete_flag); // this include only data body
  //
  // Read body
  //
  int total_recvd_byte = 0;
  for (int i = 0; i < (int)(m_socket_recv.size()); i++) {

    try {
      total_recvd_byte += recvFD(m_socket_recv[ i ], (char*)temp_buf + total_recvd_byte,
                                 each_buf_nwords[ i ] * sizeof(int), flag);
    } catch (string err_str) {
      if (*delete_flag) {
        //        B2WARNING("Delete buffer before going to Run-pause state");
        printf("[WARNING] Delete buffer before going to Run-pause state\n"); fflush(stdout);
        delete temp_buf;
      }
      throw (err_str);
    }
    //
    // Data length check
    //
    int temp_length = 0;
    for (int j = 0; j < each_buf_nodes[ i ] * each_buf_events[ i ]; j++) {
      int this_length = *((int*)((char*)temp_buf + total_recvd_byte - each_buf_nwords[ i ] * sizeof(int) + temp_length));
      temp_length += this_length * sizeof(int);
    }
    if (temp_length != (int)(each_buf_nwords[ i ] * sizeof(int))) {
      printf("[DEBUG]*******SENDHDR*********** \n");
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      printf("[DEBUG]*******BODY***********\n ");
      printData(temp_buf, (int)(total_recvd_byte / sizeof(int)));
      char err_buf[500];
      sprintf(err_buf, "CORRUPTED DATA: Length written on SendHeader(%d) is invalid. Actual data size is %d. Exting...",
              (int)(*total_buf_nwords * sizeof(int)), temp_length);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }

  }

  if ((int)(*total_buf_nwords * sizeof(int)) != total_recvd_byte) {
    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Received data size (%d byte) is not same as expected one (%d) from Sendheader. Exting...",
            total_recvd_byte, (int)(*total_buf_nwords * sizeof(int)));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }

  // Read Traeiler
  int send_trl_buf[(unsigned int)(SendTrailer::SENDTRL_NWORDS) ];
  for (int i = 0; i < (int)(m_socket_recv.size()); i++) {
    try {
      recvFD(m_socket_recv[ i ], (char*)send_trl_buf, SendTrailer::SENDTRL_NWORDS * sizeof(int), flag);
    } catch (string err_str) {
      if (*delete_flag) {
        //        B2WARNING("Delete buffer before going to Run-pause state");
        printf("[WARNING] Delete buffer before going to Run-pause state\n"); fflush(stdout);
        delete temp_buf;
      }
      throw (err_str);
    }
  }

  return temp_buf;
}


void DesSerPrePC::setRecvdBuffer(RawDataBlockFormat* temp_raw_datablk, int* delete_flag)
{
  //
  // Get data from socket
  //
  int total_buf_nwords = 0 ;
  int num_events_in_sendblock = 0;
  int num_nodes_in_sendblock = 0;

  if (m_start_flag == 0) {
    //    B2INFO("DeSerializerPrePC: Reading the 1st packet from eb0...");
    printf("DeSerializerPrePC: Reading the 1st packet from eb0...\n"); fflush(stdout);
  }
  int* temp_buf = recvData(delete_flag, &total_buf_nwords, &num_events_in_sendblock,
                           &num_nodes_in_sendblock);
  if (m_start_flag == 0) {
    //    B2INFO("DeSerializerPrePC: Done. the size of the 1st packet " << total_buf_nwords << " words");
    printf("DeSerializerPrePC: Done. the size of the 1st packet %d words\n", total_buf_nwords); fflush(stdout);
    m_start_flag = 1;
  }
  m_recvd_totbytes += total_buf_nwords * sizeof(int);

  temp_raw_datablk->SetBuffer((int*)temp_buf, total_buf_nwords, *delete_flag,
                              num_events_in_sendblock, num_nodes_in_sendblock);

  //
  // check even # and node # in one Sendblock
  //
  int num_entries = temp_raw_datablk->GetNumEntries();
  if (num_entries != num_events_in_sendblock * num_nodes_in_sendblock) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] CORRUPTED DATA: Inconsistent SendHeader value. # of nodes(%d) times # of events(%d) differs from # of entries(%d). Exiting...",
            num_nodes_in_sendblock, num_events_in_sendblock, num_entries);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }
  return;

}




void DesSerPrePC::checkData(RawDataBlockFormat* raw_datablk, unsigned int* eve_copper_0)
{
  //  int data_size_copper_0 = -1;
  //  int data_size_copper_1 = -1;

  //
  // Data check
  //
  int* temp_buf = raw_datablk->GetBuffer(0);
  int cpr_num = 0;
  unsigned int cur_evenum = 0, cur_copper_ctr = 0;
  unsigned int eve_array[32]; // # of noeds is less than 17
  unsigned int utime_array[32];// # of noeds is less than 17
  unsigned int ctime_type_array[32];// # of noeds is less than 17

#ifdef DUMHSLB
  unsigned int exp_run_ftsw = 0, ctime_trgtype_ftsw = 0, utime_ftsw = 0;
#endif

  for (int k = 0; k < raw_datablk->GetNumEvents(); k++) {
    memset(eve_array, 0, sizeof(eve_array));
    memset(utime_array, 0, sizeof(utime_array));
    memset(ctime_type_array, 0, sizeof(ctime_type_array));

    int num_nodes_in_sendblock = raw_datablk->GetNumNodes();
    for (int l = 0; l < num_nodes_in_sendblock; l++) {
      int entry_id = l + k * num_nodes_in_sendblock;

      //
      // RawFTSW
      //
      if (raw_datablk->CheckFTSWID(entry_id)) {
        RawFTSWFormat* temp_rawftsw = new RawFTSWFormat;
        int block_id = 0;
        temp_rawftsw->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);
        if (temp_rawftsw->GetEveNo(block_id) < 10) {
          printf("[DEBUG] ######FTSW#########\n");
          printData((int*)temp_buf + raw_datablk->GetBufferPos(entry_id), raw_datablk->GetBlockNwords(entry_id));
        }

#ifdef DUMHSLB
        exp_run_ftsw = temp_rawftsw->GetExpRunSubrun(block_id);
        ctime_trgtype_ftsw = temp_rawftsw->GetTTCtimeTRGType(block_id);
        utime_ftsw = temp_rawftsw->GetTTUtime(block_id);
#endif


#ifndef NO_DATA_CHECK
        try {
          temp_rawftsw->CheckData(0, m_prev_evenum, &cur_evenum, m_prev_exprunsubrun_no, &m_exprunsubrun_no);
          eve_array[ entry_id ] = cur_evenum;
        } catch (string err_str) {
          char err_buf[500];
          strcpy(err_buf, err_str.c_str());
          print_err.PrintError(err_buf , __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(1);
        }
#endif
        utime_array[ entry_id ] = temp_rawftsw->GetTTUtime(block_id);
        ctime_type_array[ entry_id ] = temp_rawftsw->GetTTCtimeTRGType(block_id);
        delete temp_rawftsw;

        //
        // RawTLU
        //
      } else if (raw_datablk->CheckTLUID(entry_id)) {

        RawTLUFormat* temp_rawtlu = new RawTLUFormat;
        temp_rawtlu->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                               raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);
        if (temp_rawtlu->GetEveNo(0) < 10
           ) {
          printf("[DEBUG] ######TLU#########\n");
          printData((int*)temp_buf + raw_datablk->GetBufferPos(entry_id), raw_datablk->GetBlockNwords(entry_id));
        }

#ifndef NO_DATA_CHECK
        try {
          temp_rawtlu->CheckData(0, m_prev_evenum, &cur_evenum);
          eve_array[ entry_id ] = cur_evenum;
        } catch (string err_str) {
          char err_buf[500];
          strcpy(err_buf, err_str.c_str());
          print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(1);
        }
#endif
        delete temp_rawtlu;
      } else {
        //
        // RawCOPPER
        //
        PreRawCOPPERFormat_latest* pre_rawcpr_fmt = new PreRawCOPPERFormat_latest;
        pre_rawcpr_fmt->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                  raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);

#ifdef DUMHSLB
        int block_id = 0;
        "do not use the following for actual DAQ"
        (pre_rawcpr_fmt->GetBuffer(block_id))[ RawHeader_latest::POS_EXP_RUN_NO ] = exp_run_ftsw;
        (pre_rawcpr_fmt->GetBuffer(block_id))[ RawHeader_latest::POS_TTCTIME_TRGTYPE ] = ctime_trgtype_ftsw;
        (pre_rawcpr_fmt->GetBuffer(block_id))[ RawHeader_latest::POS_TTUTIME ] = utime_ftsw;
#endif

#ifndef NO_DATA_CHECK
        try {
          pre_rawcpr_fmt->CheckData(0, m_prev_evenum, &cur_evenum,
                                    m_prev_copper_ctr, &cur_copper_ctr,
                                    m_prev_exprunsubrun_no, &m_exprunsubrun_no);
          eve_array[ entry_id ] = cur_evenum;
        } catch (string err_str) {
          exit(1); // Error in the contents of an event was detected
        }
#endif

        utime_array[ entry_id ] = pre_rawcpr_fmt->GetTTUtime(0);
        ctime_type_array[ entry_id ] = pre_rawcpr_fmt->GetTTCtimeTRGType(0);

        if (cpr_num == 0) {
          //          data_size_copper_0 = raw_datablk->GetBlockNwords(entry_id);
          *eve_copper_0 = (raw_datablk->GetBuffer(entry_id))[ 3 ];
        } else if (cpr_num == 1) {
          //          data_size_copper_1 = raw_datablk->GetBlockNwords(entry_id);
        }
        cpr_num++;
        delete pre_rawcpr_fmt;
      }
    }

#ifndef NO_DATA_CHECK
    // event #, ctime, utime over nodes
    for (int l = 1; l < num_nodes_in_sendblock; l++) {
      if (eve_array[ 0 ] != eve_array[ l ] ||
          utime_array[ 0 ] != utime_array[ l ] ||
          ctime_type_array[ 0 ] != ctime_type_array[ l ]) {
        char err_buf[500];
        for (int m = 0; m < num_nodes_in_sendblock; m++) {
          printf("[DEBUG] node %d eve # %d utime %x ctime %x\n",
                 m,  eve_array[ m ], utime_array[ m ], ctime_type_array[ m ]);
        }
        sprintf(err_buf, "[FATAL] CORRUPTED DATA: Event or Time record mismatch. Exiting...");
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        sleep(1234567);
        exit(-1);
      }
    }
#endif

    // Event # monitor in runchange
//     if (m_prev_runsubrun_no != m_runsubrun_no) {
//       printf("[DEBUG] ##############################################\n");
//       for (int m = 0; m < raw_datablk->GetNumEntries(); m++) {
//         printf("[DEBUG] %d eve %u prev %u\n", m, eve_array[ m ], m_prev_evenum);
//       }
//       printf("[DEBUG] ##############################################\n");
//       fflush(stderr);
//     }
    m_prev_evenum = cur_evenum;
    m_prev_copper_ctr = cur_copper_ctr;
    m_prev_exprunsubrun_no = m_exprunsubrun_no;
  }
  return;
}



void DesSerPrePC::DataAcquisition()
{
  // For data check
  unsigned int eve_copper_0 = 0;
  //  B2INFO("initializing...");
  printf("[DEBUG] initializing...\n"); fflush(stdout);
  initialize();

  //  B2INFO("Done.");
  printf("[DEBUG] Done.\n"); fflush(stdout);

  if (m_start_flag == 0) {
    //
    // Connect to eb0: This should be here because we want Serializer first to accept connection from eb1tx
    //
    Connect();
    if (m_status.isAvailable()) {
      //      B2INFO("DeSerializerPrePC: Waiting for Start...\n");
      printf("[DEBUG] DeSerializerPrePC: Waiting for Start...\n"); fflush(stdout);
      m_status.reportRunning();
    }
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }

  //
  // Main loop
  //
  while (1) {
    //
    // Stand-by loop
    //
#ifdef NONSTOP
    if (m_run_pause > 0 || m_run_error > 0) {
      waitResume();
    }
#endif

    clearNumUsedBuf();
    //    RawDataBlock raw_datablk[ NUM_EVT_PER_BASF2LOOP_PC ];
    RawDataBlockFormat raw_datablk[ NUM_EVT_PER_BASF2LOOP_PC ];

    //
    // Recv loop
    //
    for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
      //
      // Receive data from COPPER
      //
      eve_copper_0 = 0;
      int delete_flag_from =
        0; // Delete flag for temp_rawdatablk.It can be set to 1 by setRecvdBuffer if the buffer size is larger than that of pre-allocated buffer.
      int delete_flag_to =
        0; // Delete flag for raw_datablk[i]. It can be set to 1 by getNewBuffer if the buffer size is larger than that of pre-allocated buffer.
      RawDataBlockFormat temp_rawdatablk;
      try {
        setRecvdBuffer(&temp_rawdatablk, &delete_flag_from);
        checkData(&temp_rawdatablk, &eve_copper_0);
      } catch (string err_str) {
        printf("Error was detected\n"); fflush(stdout);
        break;
      }
      //     PreRawCOPPERFormat_latest pre_rawcopper_latest;
      //     pre_rawcopper_latest.SetBuffer((int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(),
      //                                    0, temp_rawdatablk.GetNumEvents(), temp_rawdatablk.GetNumNodes());
      ////     pre_rawcopper_latest.CheckCRC16( 0, 0 );

      int temp_num_events, temp_num_nodes;
      int temp_nwords_to;
      int* buf_to = NULL;
#ifdef REDUCED_RAWCOPPER
      //
      // Copy reduced buffer
      //
      //      int* buf_to = getNewBuffer(m_pre_rawcpr.CalcReducedDataSize(&temp_rawdatablk), &delete_flag_to); // basf2-dependent style
      int* temp_bufin = temp_rawdatablk.GetWholeBuffer();
      int temp_nwords_from = temp_rawdatablk.TotalBufNwords();
      temp_num_events = temp_rawdatablk.GetNumEvents();
      temp_num_nodes = temp_rawdatablk.GetNumNodes();
      int calced_temp_nwords_to = m_pre_rawcpr.CalcReducedDataSize(temp_bufin, temp_nwords_from, temp_num_events, temp_num_nodes);
      buf_to = getNewBuffer(calced_temp_nwords_to, &delete_flag_to);

      //      m_pre_rawcpr.CopyReducedData(&temp_rawdatablk, buf_to, delete_flag_from); // basf2-dependent style
      m_pre_rawcpr.CopyReducedData(temp_bufin, temp_nwords_from, temp_num_events, temp_num_nodes, buf_to, &temp_nwords_to);
      if (calced_temp_nwords_to != temp_nwords_to) {
        char err_buf[500];
        sprintf(err_buf,
                "[FATAL] CORRUPTED DATA: Estimations of reduced event size are inconsistent. CalcReducedDataSize = %d. CopyReducedData = %d. Exiting...",
                calced_temp_nwords_to, temp_nwords_to);
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
      m_status.copyEventHeader(buf_to);
#else
      buf_to = temp_rawdatablk.GetWholeBuffer();
      temp_nwords_to = temp_rawdatablk.TotalBufNwords();
      temp_num_events = temp_rawdatablk.GetNumEvents();
      temp_num_nodes = temp_rawdatablk.GetNumNodes();
      delete_flag_to = delete_flag_from;
      delete_flag_from = 0; // to avoid double delete
#endif

      //
      // Set buffer to the RawData class stored in DataStore
      //
//       raw_datablk[ j ].SetBuffer( (int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(),
//                                  delete_flag_to, temp_rawdatablk.GetNumEvents(),
//                                  temp_rawdatablk.GetNumNodes());
      raw_datablk[ j ].SetBuffer(buf_to, temp_nwords_to, delete_flag_to, temp_num_events, temp_num_nodes);


      //
      // CRC16 check after data reduction
      //
#ifdef REDUCED_RAWCOPPER
      PostRawCOPPERFormat_latest post_rawcopper_latest;

//       post_rawcopper_latest.SetBuffer((int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(),
//                                       0, temp_rawdatablk.GetNumEvents(), temp_rawdatablk.GetNumNodes());
      post_rawcopper_latest.SetBuffer(raw_datablk[ j ].GetWholeBuffer(), raw_datablk[ j ].TotalBufNwords(),
                                      0, raw_datablk[ j ].GetNumEvents(), raw_datablk[ j ].GetNumNodes());

      for (int i_finesse_num = 0; i_finesse_num < 4; i_finesse_num ++) {
        int block_num = 0;
        if (post_rawcopper_latest.GetFINESSENwords(block_num, i_finesse_num) > 0) {
          post_rawcopper_latest.CheckCRC16(block_num, i_finesse_num);
        }
      }

#endif
    }

#ifdef NONSTOP
    // Goto Stand-by loop when run is paused or stopped by error
    if (m_run_pause != 0 || m_run_error != 0) continue;
#endif


    ///////////////////////////////////////////////////////////////
    // From Serializer.cc
    //////////////////////////////////////////////////////////////
    if (m_start_flag == 0) {
      m_start_time = getTimeSec();
      n_basf2evt = 0;
    }
    //  StoreArray<RawCOPPER> rawcprarray;
    //  StoreArray<RawDataBlock> raw_dblkarray;

    for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {

      //
      // Send data
      //
      if (m_start_flag == 0) {
        //        B2INFO("SerializerPC: Sending the 1st packet...");
        printf("[DEBUG] SerializerPC: Sending the 1st packet...\n"); fflush(stdout);
      }

      try {
        m_sent_totbytes += sendByWriteV(&(raw_datablk[ j ]));
      } catch (string err_str) {
#ifdef NONSTOP
        break;
#endif
        print_err.PrintError((char*)err_str.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
      if (m_start_flag == 0) {
        //        B2INFO("Done. ");
        printf("[DEBUG] Done.\n"); fflush(stdout);
        m_start_flag = 1;
      }
    }

#ifdef NONSTOP
    // Goto Stand-by loop when run is paused or stopped by error
    if (m_run_pause != 0 || m_run_error != 0) continue;
#endif

    //
    // Monitor
    //
    if (max_nevt >= 0 || max_seconds >= 0.) {
#ifdef AIUEO
      if (n_basf2evt % 10000 == 0) {
//     if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
//         || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
        printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n",
               max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      }
#endif
    }

    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC) % 100000 == 0) {
      double interval = cur_time - m_prev_time;
      double total_time = cur_time - m_start_time;
      printf("[DEBUG] Event %12d Rate %6.2lf[kHz] Recvd %6.2lf[MB/s] sent %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s]\n",
             n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC,
             (n_basf2evt  - m_prev_nevt)*NUM_EVT_PER_BASF2LOOP_PC / interval / 1.e3,
             (m_recvd_totbytes - m_recvd_prev_totbytes) / interval / 1.e6,
             (m_sent_totbytes - m_sent_prev_totbytes) / interval / 1.e6,
             total_time,
             interval);
      fflush(stdout);

      m_prev_time = cur_time;
      m_recvd_prev_totbytes = m_recvd_totbytes;
      m_sent_prev_totbytes = m_sent_totbytes;
      m_prev_nevt = n_basf2evt;
      cur_time = getTimeSec();
    }

    n_basf2evt++;

    if (m_status.isAvailable()) {
      m_status.setOutputNBytes(m_sent_totbytes);
      m_status.setOutputCount(n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC);
    }

  }

  return;
}

/////////////////////////////////////////////////////
//   From Serializer.cc
/////////////////////////////////////////////////////


#ifdef NONSTOP
void DesSerPrePC::waitResume()
{
  if (m_run_pause == 0) {
    while (true) {
      if (checkRunPause()) break;
#ifdef NONSTOP_DEBUG
      printf("\033[31m");
      printf("###########(DesSerPrePC) Waiting for Runstop()  ###############\n");
      fflush(stdout);
      printf("\033[0m");
#endif
      sleep(1);
    }
  }

  while (true) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Waiting for Resume ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    if (checkRunRecovery()) {
      m_run_pause = 0;
      break;
    }
    sleep(1);
  }


  printf("Done!\n"); fflush(stdout);

  printf("Checking connection to eb0\n"); fflush(stdout);
  if (CheckConnection(m_socket_send) < 0) {
    printf("Trying Accept1\n"); fflush(stdout);
    Accept();
    printf("Trying Accept2\n"); fflush(stdout);
  }

  printf("Checking connection to COPPER\n"); fflush(stdout);
  for (int i = 0; i < m_num_connections; i++) {
    if (CheckConnection(m_socket_recv[ i ]) < 0)  m_socket_recv[ i ] = -1;
  }
  Connect();

  resumeRun();
  return;
}



#endif

