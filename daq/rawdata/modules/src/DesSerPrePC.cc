//+
// File : DeSerializerPC.cc
// Description : Module to receive data from eb0 and send it to eb1tx
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DesSerPrePC.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <sys/mman.h>
//#define MAXEVTSIZE 400000000
#define CHECKEVT 5000

//#define DEBUG
//#define NO_DATA_CHECK
//#define DUMHSLB

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------


//----------------------------------------------------------------
//                 Implementation
//----------------------------------------------------------------

DesSerPrePC::DesSerPrePC(string host_recv, int port_recv, string host_send, int port_send, int shmflag,
                         const std::string& nodename, int nodeid)
{


  m_num_connections = 1;

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

  m_shmflag = shmflag;

  m_nodename = nodename;

  m_nodeid = nodeid;

  m_exprunsubrun_no = 0; // will obtain info from data

  m_prev_exprunsubrun_no = 0xFFFFFFFF;

#ifdef NONSTOP
  m_run_pause = 0;

  m_run_error = 0;
#endif

  B2INFO("DeSerializerPrePC: Constructor done.");
}



DesSerPrePC::~DesSerPrePC()
{

}


int* DesSerPrePC::getPreAllocBuf()
{
  int* tempbuf = 0;
  if (m_num_usedbuf < NUM_PREALLOC_BUF) {
    tempbuf = m_bufary[ m_num_usedbuf  ];
    m_num_usedbuf++;
  } else {
    char err_buf[500];
    sprintf(err_buf,
            "No pre-allocated buffers are left. %d > %d. Not enough buffers are allocated or "
            "memory leak or forget to call ClearNumUsedBuf every event loop. Exting...",
            m_num_usedbuf, NUM_PREALLOC_BUF);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }
  return tempbuf;
}


int* DesSerPrePC::getNewBuffer(int nwords, int* delete_flag)
{

  int* temp_buf = NULL;
  // Prepare buffer
  if (nwords >  BUF_SIZE_WORD) {
    *delete_flag = 1;
    temp_buf = new int[ nwords ];
  } else {
    if ((temp_buf = getPreAllocBuf()) == 0x0) {
      char err_buf[500];
      sprintf(err_buf, "Null pointer from GetPreALlocBuf(). Exting...\n");
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    } else {
      *delete_flag = 0;
    }
  }

  return temp_buf;

}

void DesSerPrePC::initialize()
{
  B2INFO("DesSerPrePC: initialize() started.");

  signal(SIGPIPE , SIG_IGN);
  //
  // initialize Rx part from DeSerializer**.cc
  //

  // allocate buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

  // initialize buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Open message handler
  clearNumUsedBuf();

  // Shared memory
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_status.open(m_nodename, m_nodeid);
      m_status.reportReady();
    }
  }

  event_diff = 0;

  m_prev_copper_ctr = 0xFFFFFFFF;
  m_prev_evenum = 0xFFFFFFFF;


  //
  // initialize Rx part from DeSerializer**.cc
  //
  m_start_flag = 0;
  n_basf2evt = -1;
  m_compressionLevel = 0;

#ifdef DUMMY
  m_buffer = new int[ BUF_SIZE_WORD ];
#endif

  Accept();

#ifdef NONSTOP
  openRunPauseNshm();
#endif

  B2INFO("DesSerPrePC: initialize() was done.");

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
        char err_buf[500];
        sprintf(err_buf, "recv() returned error; ret = %d. : %s %s %d",
                read_size, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
        g_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);
#endif
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else if (read_size == 0) {
      // Connection is closed ( error )
      char err_buf[500];
      sprintf(err_buf, "Connection is closed by peer(%s). readsize = %d %d. Exiting...: %s %s %d",
              strerror(errno), read_size, errno, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
      m_run_error = 1;
      B2ERROR(err_buf);
      string err_str = "RUN_ERROR";
      throw (err_str);
#endif
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
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
      sprintf(err_buf, "hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...", m_hostname_from[ i ].c_str(),
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

    B2INFO("[DEBUG] Connecting to " << m_hostname_from[ i ].c_str() << " port " << m_port_from[ i ]);
    while (1) {
      if (connect(sd, (struct sockaddr*)(&socPC), sizeof(socPC)) < 0) {
        perror("Failed to connect. Retrying...");
        usleep(500000);
      } else {
        B2INFO("Done");
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
  B2INFO("[DEBUG] Initialization finished");
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
              "CORRUPTED DATA: Different # of events or nodes in SendBlocks( # of eve : %d(socket 0) %d(socket %d), # of nodes: %d(socket 0) %d(socket %d). Exiting...\n",
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
    if (rawblk_nwords > (int)(2.5e6)) {
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      char err_buf[500];
      sprintf(err_buf, "CORRUPTED DATA: Too large event : Header %d %d %d %d\n", i, temp_num_events, temp_num_nodes,
              send_hdr.GetTotalNwords());
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
        B2WARNING("Delete buffer before going to Run-pause state");
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
    if (temp_length != each_buf_nwords[ i ] * sizeof(int)) {
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
        B2WARNING("Delete buffer before going to Run-pause state");
        delete temp_buf;
      }
      throw (err_str);
    }
  }

  return temp_buf;
}


void DesSerPrePC::setRecvdBuffer(RawDataBlock* temp_raw_datablk, int* delete_flag)
{
  //
  // Get data from socket
  //
  int total_buf_nwords = 0 ;
  int num_events_in_sendblock = 0;
  int num_nodes_in_sendblock = 0;

  if (m_start_flag == 0) B2INFO("DeSerializerPrePC: Reading the 1st packet from eb0...");
  int* temp_buf = recvData(delete_flag, &total_buf_nwords, &num_events_in_sendblock,
                           &num_nodes_in_sendblock);
  if (m_start_flag == 0) {
    B2INFO("DeSerializerPrePC: Done. the size of the 1st packet " << total_buf_nwords << " words");
    m_start_flag = 1;
  }
  m_totbytes += total_buf_nwords * sizeof(int);

  int temp_delete_flag = 0;
  temp_raw_datablk->SetBuffer((int*)temp_buf, total_buf_nwords, temp_delete_flag,
                              num_events_in_sendblock, num_nodes_in_sendblock);

  //
  // check even # and node # in one Sendblock
  //
  int num_entries = temp_raw_datablk->GetNumEntries();
  if (num_entries != num_events_in_sendblock * num_nodes_in_sendblock) {
    char err_buf[500];
    sprintf(err_buf,
            "CORRUPTED DATA: Inconsistent SendHeader value. # of nodes(%d) times # of events(%d) differs from # of entries(%d). Exiting...",
            num_nodes_in_sendblock, num_events_in_sendblock, num_entries);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }
  return;

}




void DesSerPrePC::checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0)
{
  int data_size_copper_0 = -1;
  int data_size_copper_1 = -1;

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
        RawFTSW* temp_rawftsw = new RawFTSW;
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

        RawTLU* temp_rawtlu = new RawTLU;
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
        int block_id = 0;
        RawCOPPER* temp_rawcopper = new RawCOPPER;
        temp_rawcopper->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                  raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);

#ifdef DUMHSLB
        "do not use the following for actual DAQ"
        (temp_rawcopper->GetBuffer(block_id))[ RawHeader_latest::POS_EXP_RUN_NO ] = exp_run_ftsw;
        (temp_rawcopper->GetBuffer(block_id))[ RawHeader_latest::POS_TTCTIME_TRGTYPE ] = ctime_trgtype_ftsw;
        (temp_rawcopper->GetBuffer(block_id))[ RawHeader_latest::POS_TTUTIME ] = utime_ftsw;
#endif

#ifndef NO_DATA_CHECK
        try {
          temp_rawcopper->CheckData(0, m_prev_evenum, &cur_evenum,
                                    m_prev_copper_ctr, &cur_copper_ctr,
                                    m_prev_exprunsubrun_no, &m_exprunsubrun_no);
          eve_array[ entry_id ] = cur_evenum;
        } catch (string err_str) {
          temp_rawcopper->PrintData(temp_rawcopper->GetWholeBuffer(), temp_rawcopper->TotalBufNwords());
          char err_buf[500];
          strcpy(err_buf, err_str.c_str());
          print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(1);
        }
#endif

        utime_array[ entry_id ] = temp_rawcopper->GetTTUtime(0);
        ctime_type_array[ entry_id ] = temp_rawcopper->GetTTCtimeTRGType(0);

        if (cpr_num == 0) {
          data_size_copper_0 = raw_datablk->GetBlockNwords(entry_id);
          *eve_copper_0 = (raw_datablk->GetBuffer(entry_id))[ 3 ];
        } else if (cpr_num == 1) {
          data_size_copper_1 = raw_datablk->GetBlockNwords(entry_id);
        }
        cpr_num++;
        delete temp_rawcopper;
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
        sprintf(err_buf, "CORRUPTED DATA: Event or Time record mismatch. Exiting...");
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
  B2INFO("initializing...");
  initialize();

  B2INFO("Done.");

  if (m_start_flag == 0) {
    //
    // Connect to eb0: This should be here because we want Serializer first to accept connection from eb1tx
    //
    Connect();
    if (m_status.isAvailable()) {
      B2INFO("DeSerializerPrePC: Waiting for Start...\n");
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
      waitResume();
    }
#endif

    clearNumUsedBuf();
    RawDataBlock raw_datablk[ NUM_EVT_PER_BASF2LOOP_PC ];

    //
    // Recv loop
    //
    for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
      //
      // Receive data from COPPER
      //
      eve_copper_0 = 0;
      int delete_flag_from = 0, delete_flag_to = 0;
      RawDataBlock temp_rawdatablk;
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

#ifdef REDUCED_RAWCOPPER
      //
      // Copy reduced buffer
      //
      int* buf_to = getNewBuffer(m_pre_rawcpr.CalcReducedDataSize(&temp_rawdatablk), &delete_flag_to);
      m_pre_rawcpr.CopyReducedData(&temp_rawdatablk, buf_to, delete_flag_from);
      m_status.copyEventHeader(buf_to);
#else
      delete_flag_to = delete_flag_from;
#endif

      //
      // Set buffer to the RawData class stored in DataStore
      //
      //    RawDataBlock* raw_datablk = raw_datablkarray.appendNew();

      raw_datablk[ j ].SetBuffer((int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(),
                                 delete_flag_to, temp_rawdatablk.GetNumEvents(),
                                 temp_rawdatablk.GetNumNodes());

      //
      // CRC16 check after data reduction
      //
#ifdef REDUCED_RAWCOPPER
      PostRawCOPPERFormat_latest post_rawcopper_latest;

      post_rawcopper_latest.SetBuffer((int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(),
                                      0, temp_rawdatablk.GetNumEvents(), temp_rawdatablk.GetNumNodes());

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
        B2INFO("SerializerPC: Sending the 1st packet...");
      }

      try {
        m_totbytes += sendByWriteV(&(raw_datablk[ j ]));
      } catch (string err_str) {
#ifdef NONSTOP
        break;
#endif
        print_err.PrintError((char*)err_str.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
      if (m_start_flag == 0) {
        B2INFO("Done. ");
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

    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC) % 10000 == 0) {
      double interval = cur_time - m_prev_time;
      double total_time = cur_time - m_start_time;
      printf("[INFO] Event %12d Rate %6.2lf[kHz] Recvd Flow %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s]\n",
             n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC,
             (n_basf2evt  - m_prev_nevt)*NUM_EVT_PER_BASF2LOOP_PC / interval / 1.e3,
             (m_totbytes - m_prev_totbytes) / interval / 1.e6,
             total_time,
             interval);
      fflush(stdout);

      m_prev_time = cur_time;
      m_prev_totbytes = m_totbytes;
      m_prev_nevt = n_basf2evt;
      cur_time = getTimeSec();
    }

    n_basf2evt++;

    if (m_status.isAvailable()) {
      m_status.setInputNBytes(m_totbytes);
      m_status.setInputCount(n_basf2evt);
    }

  }

  return;
}

/////////////////////////////////////////////////////
//   From Serializer.cc
/////////////////////////////////////////////////////

void DesSerPrePC::fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl,
                                        RawDataBlock* rawdblk)
{

  int total_send_nwords =
    hdr->GetHdrNwords() +
    rawdblk->TotalBufNwords() +
    //    rawhdr.GetNwords() +
    trl->GetTrlNwords();

  hdr->SetNwords(total_send_nwords);
  hdr->SetNumEventsinPacket(rawdblk->GetNumEvents());
  hdr->SetNumNodesinPacket(rawdblk->GetNumNodes());

  //
  // For bug check
  //
  if (rawdblk->GetNumEntries() == 1) {
    if (total_send_nwords != (rawdblk->GetBuffer(0))[ 0 ] + 8) {
      char err_buf[500];
      sprintf(err_buf, "Length error. total length %d rawdblk length %d. Exting...\n" ,
              total_send_nwords, (rawdblk->GetBuffer(0))[ 0 ]);
      printData(rawdblk->GetBuffer(0), rawdblk->TotalBufNwords());
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }
  }


  for (int i = 0; i < rawdblk->GetNumEntries(); i++) {

    //copy event # from a tonp COPPER block
    if (!(rawdblk->CheckFTSWID(i)) && !(rawdblk->CheckTLUID(i))) {
      tmp_header.SetBuffer(rawdblk->GetBuffer(i));
      hdr->SetEventNumber(tmp_header.GetEveNo());
      hdr->SetNodeID(tmp_header.GetNodeID());
      hdr->SetExpRunWord(tmp_header.GetExpRunSubrun());
      break;
    }

    //Error if you cannot find any COPPER block
    if (i == (rawdblk->GetNumEntries() - 1)) {
      printf("[DEBUG] i= %d : num entries %d : Tot words %d\n", i , rawdblk->GetNumEntries(), rawdblk->TotalBufNwords());
      printData(rawdblk->GetBuffer(0), rawdblk->TotalBufNwords());

      char err_buf[500] = "CORRUPTED DATA: No COPPER blocks in RawDataBlock. Exiting...";
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }
  }
  return;
}


int DesSerPrePC::sendByWriteV(RawDataBlock* rawdblk)
{
  SendHeader send_header;
  SendTrailer send_trailer;
  fillSendHeaderTrailer(&send_header, &send_trailer, rawdblk);

  enum {
    NUM_BUFFER = 3
  };
  struct iovec iov[ NUM_BUFFER ];

  // check Body data size
  int rawcopper_nwords = rawdblk->TotalBufNwords();

  //Fill iov info.
  iov[0].iov_base = (char*)send_header.GetBuffer();
  iov[0].iov_len = sizeof(int) * send_header.GetHdrNwords();

  iov[1].iov_base = (char*)rawdblk->GetWholeBuffer();
  iov[1].iov_len = sizeof(int) * rawcopper_nwords;

  iov[2].iov_base = (char*)send_trailer.GetBuffer();
  iov[2].iov_len = sizeof(int) * send_trailer.GetTrlNwords();

  // Send Multiple buffers
  int n = 0;
  while (true) {
    if ((n = writev(m_socket_send, iov, NUM_BUFFER)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
#ifdef NONSTOP
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf(err_buf, "WRITEV error.(%s) Exiting... : sent %d bytes, header %d bytes body %d trailer %d\n" ,
                strerror(errno), n, iov[0].iov_len, iov[1].iov_len, iov[2].iov_len);
#ifdef NONSTOP
        m_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);  // To exit this module, go to DeSerializer** and wait for run-resume.
#endif
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
    }
    break;
  }

#ifdef DEBUG
  printf("[DEBUG] *******BODY**********\n");
  printf("[DEBUG] \n%.8d : ", 0);
  printData((int*)(iov[1].iov_base), iov[1].iov_len);
#endif


  int total_send_bytes = sizeof(int) * send_header.GetTotalNwords();


  //
  // Retry sending
  //
  if (n != total_send_bytes) {
    B2WARNING("Serializer: Sent byte(" << n << "bytes) is not same as the event size (" << total_send_bytes << "bytes). Retryring...");
    double retry_start = getTimeSec();
    // Send Header
    if (n < (int)(iov[ 0 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 0 ].iov_base + n, iov[ 0 ].iov_len - n);
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 1 ].iov_base + (n - iov[ 0 ].iov_len), iov[ 1 ].iov_len - (n - iov[ 0 ].iov_len));
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len + iov[ 2 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 2 ].iov_base + (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len),
                iov[ 2 ].iov_len - (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len));
    }
    double retry_end = getTimeSec();
    B2WARNING("Resending ends. It takes " << retry_end - retry_start << "(s)");
  }
  //   printf( "[DEBUG] n %d total %d\n", n, total_send_bytes);
  //  delete temp_buf;

  return total_send_bytes;

}


int DesSerPrePC::Send(int socket, char* buf, int size_bytes)
{
  int sent_bytes = 0;
  while (true) {
    int ret = 0;
    if ((ret = send(socket,
                    buf + sent_bytes, size_bytes - sent_bytes,  MSG_NOSIGNAL)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
#ifdef NONSTOP
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf(err_buf, "SEND ERROR.(%s) Exiting...", strerror(errno));
#ifdef NONSTOP
        m_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);  // To exit this module, go to DeSerializer** and wait for run-resume.
#endif
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
    }
    sent_bytes += ret;
    if (sent_bytes == size_bytes) break;
  }
  return sent_bytes;
}

void DesSerPrePC::Accept()
{
  //
  // Connect to cprtb01
  //
  struct hostent* host;
  host = gethostbyname(m_hostname_local.c_str());
  if (host == NULL) {
    char temp_buf[500];
    sprintf(temp_buf, "[ERROR] hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...\n",
            m_hostname_local.c_str(), strerror(errno));
    print_err.PrintError(temp_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }

  //
  // Bind and listen
  //
  int fd_listen;
  struct sockaddr_in sock_listen;
  sock_listen.sin_family = AF_INET;
  sock_listen.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];

  socklen_t addrlen = sizeof(sock_listen);
  sock_listen.sin_port = htons(m_port_to);
  fd_listen = socket(PF_INET, SOCK_STREAM, 0);

  int flags = 1;
  int ret = setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &flags, (socklen_t)sizeof(flags));
  if (ret < 0) {
    perror("Failed to set REUSEADDR");
  }

  if (bind(fd_listen, (struct sockaddr*)&sock_listen, sizeof(struct sockaddr)) < 0) {
    char temp_char[500];
    sprintf(temp_char, "Failed to bind.(%s) Maybe other programs have already occupied this port(%d). Exiting...",
            strerror(errno), m_port_to);
    print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    char err_buf[500];
    sprintf(err_buf, "Failed in listen(%s). Exting...", strerror(errno));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  //
  // Accept
  //
  int fd_accept;
  struct sockaddr_in sock_accept;
  printf("Accepting... : port %d server %s\n", m_port_to, m_hostname_local.c_str());
  fflush(stdout);
  //  B2INFO("Accepting... : port " << m_port_to << " server " << m_hostname_local.c_str() );
  B2INFO("Accepting...");
  if ((fd_accept = accept(fd_listen, (struct sockaddr*) & (sock_accept), &addrlen)) == 0) {
    char err_buf[500];
    sprintf(err_buf, "Failed to accept(%s). Exiting...", strerror(errno));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  } else {
    //    B2INFO("Connection is established: port " << htons(sock_accept.sin_port) << " address " <<  sock_accept.sin_addr.s_addr );
    B2INFO("Done.");

    //    set timepout option
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    ret = setsockopt(fd_accept, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(timeout));
    if (ret < 0) {
      char temp_char[100] = "Failed to set TIMEOUT. Exiting...";
      print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
  }
  close(fd_listen);

  //   int flag = 1;
  //   ret = setsockopt(fd_accept, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag) );
  m_socket_send = fd_accept;
  if (m_status.isAvailable()) {
    m_status.setOutputPort(ntohs(sock_listen.sin_port));
    m_status.setOutputAddress(sock_listen.sin_addr.s_addr);
    printf("%d %x\n", (int)ntohs(sock_listen.sin_port), (int)sock_listen.sin_addr.s_addr);
  }

  return;

}

double DesSerPrePC::getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void DesSerPrePC::recordTime(int event, double* array)
{
  if (event >= 50000 && event < 50500) {
    array[ event - 50000 ] = getTimeSec() - m_start_time;
  }
  return;
}


unsigned int DesSerPrePC::calcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


void DesSerPrePC::printData(int* buf, int nwords)
{
  printf("[DEBUG]");
  for (int i = 0; i < nwords; i++) {
    printf("%.8x ", buf[ i ]);
    if (i % 10 == 9) printf("\n[DEBUG]");
  }
  printf("\n[DEBUG]");
  printf("\n");
  return;
}


#ifdef NONSTOP
void DesSerPrePC::openRunPauseNshm()
{
  char path_shm[100] = "/cpr_pause_resume";
  int fd = shm_open(path_shm, O_RDONLY, 0666);
  if (fd < 0) {
    printf("[DEBUG] %s\n", path_shm);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  m_ptr = (int*)mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
  return;
}

int DesSerPrePC::checkRunPause()
{
  if (*m_ptr) {
    return 1;
  } else {
    return 0;
  }
}

int DesSerPrePC::checkRunRecovery()
{
  if (*m_ptr) {
    return 0;
  } else {
    return 1;
  }
}

void DesSerPrePC::resumeRun()
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(Ser) the 1st event sicne the resume  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  m_run_error = 0;
  m_run_pause = 0;
  return;
}


void DesSerPrePC::pauseRun()
{
  m_run_pause = 1;
#ifdef NONSTOP_DEBUG
  printf("###########(Ser) Pause the run ###############\n");
  fflush(stdout);
#endif
  return;
}


void DesSerPrePC::waitResume()
{

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

  if (CheckConnection(m_socket_send) < 0) {
    printf("Trying Accept1\n"); fflush(stdout);
    Accept();
    printf("Trying Accept2\n"); fflush(stdout);
  }

  for (int i = 0; i < m_num_connections; i++) {
    if (CheckConnection(m_socket_recv[ i ]) < 0)  m_socket_recv[ i ] = -1;
  }
  Connect();

  resumeRun();
  return;
}


void DesSerPrePC::callCheckRunPause(string& err_str)
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(DesSer) TIMEOUT.  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  if (checkRunPause()) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(DesSer) Pause is detected. ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    m_run_pause = 1;
    throw (err_str);
  }
  return;
}


int DesSerPrePC::CheckConnection(int socket)
{
  // Modify Yamagata-san's eb/iseof.cc


  int ret;
  char buffer[100000];
  int eagain_cnt = 0;
  int tot_ret = 0;
  printf("CC1\n"); fflush(stdout);
  ret = send(socket, buffer, 0, MSG_DONTWAIT);
  printf("CC2\n"); fflush(stdout);
  switch (ret) {
    case 0:
      break;
    case -1:
      if (errno == EAGAIN) {
        printf("EAGAIN %d cnt %d recvd %d\n", socket, eagain_cnt, tot_ret); fflush(stdout);
        /* not EOF, no data in queue */
        if (eagain_cnt > 100) {
          return 0;
        }
        usleep(10000);
        eagain_cnt++;
      } else {
        printf("ERROR %d errno %d err %s\n", socket , errno, strerror(errno)); fflush(stdout);
        close(socket);
        return -1;
      }
    default:
      printf("Return value %d of send is strange. Exting...\n",  ret);
      fflush(stdout);
      exit(1);
  }

  while (true) {

    //
    // Extract data in the socket buffer of a peer
    //
    //    ret = recv( socket, buffer, sizeof(buffer), MSG_PEEK|MSG_DONTWAIT );
    ret = recv(socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    switch (ret) {
      case 0: /* EOF */
        printf("EOF %d\n", socket); fflush(stdout);
        close(socket);
        return -1;
      case -1:
        if (errno == EAGAIN) {
          printf("EAGAIN %d cnt %d recvd %d\n", socket, eagain_cnt, tot_ret); fflush(stdout);
          /* not EOF, no data in queue */
          if (eagain_cnt > 100) {
            return 0;
          }
          usleep(10000);
          eagain_cnt++;
        } else {
          printf("ERROR %d errno %d err %s\n", socket , errno, strerror(errno)); fflush(stdout);
          close(socket);
          return -1;
        }
        break;
      default:
        tot_ret += ret;
        printf("Flushing data in socket buffer : sockid = %d %d bytes tot %d bytes\n", socket, ret, tot_ret); fflush(stdout);
    }
  }
}


#endif

void DesSerPrePC::shmOpen(char*, char*)
//(char* path_cfg, char* path_sta)
{
  errno = 0;
  /*m_shmfd_cfg = shm_open( "/cpr_config2", O_CREAT | O_EXCL | O_RDWR, 0666);
  if (m_shmfd_cfg < 0) {
    if (errno != EEXIST) {
      perror("shm_open1");
      exit(1);
    }
  m_shmfd_cfg = shm_open(path_cfg, O_RDWR, 0666);
  if (m_shmfd_cfg < 0) {
    printf( "[DEBUG] %s\n", path_cfg);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  */
  //}
  /*
  m_shmfd_sta = shm_open( "/cpr_status2", O_CREAT | O_EXCL | O_RDWR, 0666);
  if (m_shmfd_sta < 0) {
    if (errno != EEXIST) {
      perror("shm_open1");
      exit(1);
    }
  m_shmfd_sta = shm_open(path_sta , O_RDWR, 0666);
  if (m_shmfd_sta < 0) {
    printf( "[DEBUG] %s\n", path_sta);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  //}
  int size = 4 * sizeof(int);
  ftruncate(m_shmfd_cfg, size);
  ftruncate(m_shmfd_sta, size);
    */
}



