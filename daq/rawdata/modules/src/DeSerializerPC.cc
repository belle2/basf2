/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/DeSerializerPC.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <rawdata/dataobjects/RawTLU.h>

#include <netdb.h>
#include <netinet/tcp.h>

//#define MAXEVTSIZE 400000000
#define CHECKEVT 5000



#define USE_DESERIALIZER_PREPC

//#define DEBUG
//#define NO_DATA_CHECK
//#define DUMHSLB

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerPC)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

#ifndef REDUCED_RAWCOPPER
#ifdef USE_DESERIALIZER_PREPC
//compile error
#endif
#endif

DeSerializerPCModule::DeSerializerPCModule() : DeSerializerModule()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  addParam("NumConn", m_num_connections, "Number of Connections", 0);
  addParam("HostNameFrom", m_hostname_from, "Hostnames of data sources");
  addParam("PortFrom", m_port_from, "port numbers of data sources");


  B2INFO("DeSerializerPC: Constructor done.");


}



DeSerializerPCModule::~DeSerializerPCModule()
{

}


void DeSerializerPCModule::initialize()
{
  B2INFO("DeSerializerPC: initialize() started.");

  // Set m_socket
  if (m_num_connections > (int)m_hostname_from.size() ||  m_num_connections > (int)m_port_from.size()) {
    B2FATAL("[FATAL] Hostname or port# is not specified for all connections. Please check a python script. Exiting... \n");
    exit(1);
  }
  for (int i = 0; i < m_num_connections; i++) {
    m_socket.push_back(-1);
  }

  // allocate buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];


  // initialize buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  raw_datablkarray.registerInDataStore();
  rawcprarray.registerInDataStore();
  raw_ftswarray.registerInDataStore();


  // Initialize Array of RawCOPPER

  if (m_dump_fname.size() > 0) {
    openOutputFile();
  }

  // Initialize arrays for time monitor
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));

  clearNumUsedBuf();

  // Shared memory
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      g_status.open(m_nodename, m_nodeid);
      g_status.reportReady();
    }
  }

  event_diff = 0;

  m_prev_copper_ctr = 0xFFFFFFFF;
  m_prev_evenum = 0xFFFFFFFF;

#ifdef NONSTOP
  openRunPauseNshm();
#endif

  B2INFO("DeSerializerPC: initialize() done.");
}


int DeSerializerPCModule::recvFD(int sock, char* buf, int data_size_byte, int flag)
{
  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = recv(sock, (char*)buf + n, data_size_byte - n , flag)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data received within SO_RCVTIMEO
#ifdef NONSTOP
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf(err_buf, "[WARNING] recv() returned error; ret = %d. : %s %s %d",
                read_size, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
        g_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);
#endif
        print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else if (read_size == 0) {
      char err_buf[500];
      sprintf(err_buf, "[WARNING] Connection is closed by peer(%s).:  %s %s %d",
              strerror(errno), __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
      g_run_error = 1;
      B2ERROR(err_buf);
      string err_str = "RUN_ERROR";
      throw (err_str);
#endif
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    } else {
      n += read_size;
      if (n == data_size_byte)break;
    }
  }
  return n;
}

int DeSerializerPCModule::Connect()
{
  for (int i = 0; i < m_num_connections; i++) {
    if (m_socket[ i ] >= 0) continue;     // Already have an established socket
    //
    // Connect to a downstream node
    //
    struct sockaddr_in socPC;
    socPC.sin_family = AF_INET;

    struct hostent* host;
    host = gethostbyname(m_hostname_from[ i ].c_str());
    if (host == NULL) {
      char err_buf[100];
      sprintf(err_buf, "[FATAL] hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...", m_hostname_from[ i ].c_str(),
              strerror(errno));
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    }
    socPC.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    socPC.sin_port = htons(m_port_from[ i ]);
    int sd = socket(PF_INET, SOCK_STREAM, 0);

    int val1 = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, (socklen_t)sizeof(timeout));

    printf("[DEBUG] Connecting to %s port %d ...\n", m_hostname_from[ i ].c_str(), m_port_from[ i ]);
    while (1) {
      if (connect(sd, (struct sockaddr*)(&socPC), sizeof(socPC)) < 0) {
        perror("Failed to connect. Retrying...");
        usleep(500000);
      } else {
        printf("[DEBUG] Done\n");
        break;
      }
    }

    //    m_socket.push_back(sd);
    m_socket[ i ] = sd;

    // check socket paramters
    int val, len;
    len = sizeof(val);
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_RCVBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("[DEBUG] SO_RCVBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_SNDBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("[DEBUG] SO_SNDBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_MAXSEG, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("[DEBUG] TCP_MAXSEG %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("[DEBUG] TCP_NODELAY %d\n", val);
#endif
    if (g_status.isAvailable()) {
      sockaddr_in sa;
      memset(&sa, 0, sizeof(sockaddr_in));
      socklen_t sa_len = sizeof(sa);
      if (getsockname(m_socket[i], (struct sockaddr*)&sa, (socklen_t*)&sa_len) == 0) {
        g_status.setInputPort(ntohs(sa.sin_port));
        g_status.setInputAddress(sa.sin_addr.s_addr);
      }
    }

  }
  printf("[DEBUG] Initialization finished\n");
  return 0;
}



int* DeSerializerPCModule::recvData(int* delete_flag, int* total_buf_nwords, int* num_events_in_sendblock,
                                    int* num_nodes_in_sendblock)
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
  for (int i = 0; i < (int)(m_socket.size()); i++) {

    recvFD(m_socket[ i ], (char*)send_hdr_buf, sizeof(int)*SendHeader::SENDHDR_NWORDS, flag);

    SendHeader send_hdr;
    send_hdr.SetBuffer(send_hdr_buf);

    temp_num_events = send_hdr.GetNumEventsinPacket();
    temp_num_nodes = send_hdr.GetNumNodesinPacket();



    if (i == 0) {
      *num_events_in_sendblock = temp_num_events;
    } else if (*num_events_in_sendblock != temp_num_events) {

#ifndef NO_DATA_CHECK
      printf("[DEBUG] *******HDR**********\n");
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      char err_buf[500];
      sprintf(err_buf,
              "[FATAL] CORRUPTED DATA: Different # of events or nodes in SendBlocks( # of eve : %d(socket 0) %d(socket %d), # of nodes: %d(socket 0) %d(socket %d). Exiting...\n",
              *num_events_in_sendblock , temp_num_events, i,  *num_nodes_in_sendblock , temp_num_nodes, i);
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
      printf("[DEBUG] *******HDR**********\n");
      //      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      printData(send_hdr_buf, 100);
      printASCIIData(send_hdr_buf, 100);
      char err_buf[500];
      sprintf(err_buf, "[FATAL] CORRUPTED DATA: Too large event : Header %d %d %d %d\n", i, temp_num_events, temp_num_nodes,
              send_hdr.GetTotalNwords());
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
  for (int i = 0; i < (int)(m_socket.size()); i++) {

    try {
      total_recvd_byte += recvFD(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
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
    if (temp_length != (int)(each_buf_nwords[ i ] * sizeof(int))) {
      printf("[DEBUG]*******SENDHDR*********** \n");
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      printf("[DEBUG]*******BODY***********\n ");
      printData(temp_buf, (int)(total_recvd_byte / sizeof(int)));
      char err_buf[500];
      sprintf(err_buf, "[FATAL] CORRUPTED DATA: Length written on SendHeader(%d) is invalid. Actual data size is %d. Exting...",
              (int)(*total_buf_nwords * sizeof(int)), temp_length);
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }

  }

  if ((int)(*total_buf_nwords * sizeof(int)) != total_recvd_byte) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] CORRUPTED DATA: Received data size (%d byte) is not same as expected one (%d) from Sendheader. Exting...",
            total_recvd_byte, (int)(*total_buf_nwords * sizeof(int)));
    print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }

  // Read Traeiler
  int send_trl_buf[(unsigned int)(SendTrailer::SENDTRL_NWORDS) ];
  for (int i = 0; i < (int)(m_socket.size()); i++) {
    try {
      recvFD(m_socket[ i ], (char*)send_trl_buf, SendTrailer::SENDTRL_NWORDS * sizeof(int), flag);
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


void DeSerializerPCModule::setRecvdBuffer(RawDataBlock* temp_raw_datablk, int* delete_flag)
{
  //
  // Get data from socket
  //
  int total_buf_nwords = 0 ;
  int num_events_in_sendblock = 0;
  int num_nodes_in_sendblock = 0;

  if (m_start_flag == 0) B2INFO("DeSerializerPC: Reading the 1st packet from eb0...");

  int* temp_buf = recvData(delete_flag, &total_buf_nwords, &num_events_in_sendblock,
                           &num_nodes_in_sendblock);
  if (m_start_flag == 0) {
    B2INFO("DeSerializerPC: Done. the size of the 1st packet " << total_buf_nwords << " words");
    m_start_flag = 1;
  }
  m_totbytes += total_buf_nwords * sizeof(int);

  // Fixed for glibc error at Jan. 2017, reported in "Re: data taking with the new firmware".
  // for temp_raw_datablk, delete_flag should be 0. raw_datablk will take care of deleting buffer
  //  temp_raw_datablk->SetBuffer((int*)temp_buf, total_buf_nwords, *delete_flag,
  temp_raw_datablk->SetBuffer((int*)temp_buf, total_buf_nwords, 0,

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
    print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }
  return;

}



void DeSerializerPCModule::checkData(RawDataBlock* raw_datablk, unsigned int* exp_copper_0, unsigned int* run_copper_0,
                                     unsigned int* subrun_copper_0, unsigned int* eve_copper_0, unsigned int* error_bit_flag)
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
          print_err.PrintError(m_shmflag, &g_status, err_str);
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
          print_err.PrintError(m_shmflag, &g_status, err_str);
          exit(1);
        }
#endif
        delete temp_rawtlu;
      } else {

        //
        // RawCOPPER
        //


        RawCOPPER* temp_rawcopper = new RawCOPPER;
        temp_rawcopper->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                  raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);

#ifdef DUMHSLB
        int block_id = 0;
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
          print_err.PrintError(m_shmflag, &g_status, err_str);
          exit(1);
        }
#endif

        utime_array[ entry_id ] = temp_rawcopper->GetTTUtime(0);
        ctime_type_array[ entry_id ] = temp_rawcopper->GetTTCtimeTRGType(0);

        if (cpr_num == 0) {
          //          data_size_copper_0 = raw_datablk->GetBlockNwords(entry_id);
          //          *eve_copper_0 = (raw_datablk->GetBuffer(entry_id))[ 3 ];
          *eve_copper_0 = temp_rawcopper->GetEveNo(0);
          *exp_copper_0 = temp_rawcopper->GetExpNo(0);
          *run_copper_0 = temp_rawcopper->GetRunNo(0);
          *subrun_copper_0 = temp_rawcopper->GetSubRunNo(0);
        } else if (cpr_num == 1) {
          //          data_size_copper_1 = raw_datablk->GetBlockNwords(entry_id);
        }
        cpr_num++;

        // Check Error bit flag
        *error_bit_flag |= temp_rawcopper->GetErrorBitFlag(0);

        delete temp_rawcopper;
      }
    }

#ifndef NO_DATA_CHECK
    // event #, ctime, utime over nodes
    for (int l = 1; l < num_nodes_in_sendblock; l++) {
      if (eve_array[ 0 ] != eve_array[ l ]) {
//       if (eve_array[ 0 ] != eve_array[ l ] ||
//           utime_array[ 0 ] != utime_array[ l ] ||
//           ctime_type_array[ 0 ] != ctime_type_array[ l ]) {
        char err_buf[500];
        for (int m = 0; m < num_nodes_in_sendblock; m++) {
          printf("[DEBUG] node %d eve # %d utime %x ctime %x\n",
                 m,  eve_array[ m ], utime_array[ m ], ctime_type_array[ m ]);
        }
        sprintf(err_buf, "[FATAL] CORRUPTED DATA: Event or Time record mismatch. Exiting...");
        print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        sleep(1234567);
        exit(-1);
      }
    }
#endif

    // Event # monitor in runchange
    if (m_prev_exprunsubrun_no != m_exprunsubrun_no) {
      printf("[DEBUG] ##############################################\n");
      for (int m = 0; m < raw_datablk->GetNumEntries(); m++) {
        printf("[DEBUG] %d eve %u prev %u\n", m, eve_array[ m ], m_prev_evenum);
      }
      printf("[DEBUG] ##############################################\n");
      fflush(stderr);
    }
    m_prev_evenum = cur_evenum;
    m_prev_copper_ctr = cur_copper_ctr;
    m_prev_exprunsubrun_no = m_exprunsubrun_no;
  }
  return;
}

#ifdef NONSTOP
void DeSerializerPCModule::waitResume()
{
  while (true) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Waiting for Resume ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    if (checkRunRecovery()) {
      g_run_pause = 0;
      for (int i = 0; i < m_num_connections; i++) {
        if (CheckConnection(m_socket[ i ]) < 0)  m_socket[ i ] = -1;
      }
      resumeRun();
      break;
    }
    sleep(1);
  }
  return;

}
#endif

void DeSerializerPCModule::setErrorFlag(unsigned int error_flag, StoreObjPtr<EventMetaData> evtmetadata)
{
  //  RawHeader_latest raw_hdr;
  int error_set = 0;
  if (error_flag & RawHeader_latest::B2LINK_PACKET_CRC_ERROR) {
    evtmetadata->addErrorFlag(EventMetaData::c_B2LinkPacketCRCError);
    error_set = 1;
  }
  if (error_flag & RawHeader_latest::B2LINK_EVENT_CRC_ERROR) {
    evtmetadata->addErrorFlag(EventMetaData::c_B2LinkEventCRCError);
    error_set = 1;
  }
  if (error_set)  B2INFO("Raw2Ds: Error flag was set in EventMetaData.");
}


void DeSerializerPCModule::event()
{

  // For data check
  unsigned int exp_copper_0 = 0;
  unsigned int run_copper_0 = 0;
  unsigned int subrun_copper_0 = 0;
  unsigned int eve_copper_0 = 0;
  unsigned int error_bit_flag = 0;

  clearNumUsedBuf();

  if (m_start_flag == 0) {
    //
    // Connect to eb0: This should be here because we want Serializer first to accept connection from eb1tx
    //
    Connect();
    if (g_status.isAvailable()) {
      B2INFO("DeSerializerPC: Waiting for Start...\n");
      g_status.reportRunning();
    }
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }


#ifdef NONSTOP
  if (g_run_pause > 0 ||  g_run_error > 0) {
    if (g_run_pause == 0) {
      while (true) {
        if (checkRunPause()) break;
#ifdef NONSTOP_DEBUG
        printf("\033[31m");
        printf("###########(DeserializerPC) Waiting for Runpause()  ###############\n");
        fflush(stdout);
        printf("\033[0m");
#endif
        sleep(1);
      }
    }
    waitResume();
    m_eventMetaDataPtr.create(); // Otherwise endRun() is called.
    return;
  }
#endif

  // Make rawdatablk array


  //
  // Main loop
  //
  int* buf_rc = NULL;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
    eve_copper_0 = 0;
    //
    // Set buffer to the RawData class stored in DataStore
    //
    int delete_flag = 0;
    RawDataBlock temp_rawdatablk;
    try {
      setRecvdBuffer(&temp_rawdatablk, &delete_flag);
      checkData(&temp_rawdatablk, &exp_copper_0, &run_copper_0, &subrun_copper_0, &eve_copper_0, &error_bit_flag);
    } catch (string err_str) {
#ifdef NONSTOP
      // Update EventMetaData otherwise basf2 stops.
      if (err_str == "RUN_PAUSE" || err_str == "RUN_ERROR") {
        m_eventMetaDataPtr.create();
        return;
      }
#endif
      print_err.PrintError((char*)err_str.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(1);
    }

    RawDataBlock* raw_datablk = raw_datablkarray.appendNew();
    raw_datablk->SetBuffer((int*)temp_rawdatablk.GetWholeBuffer(), temp_rawdatablk.TotalBufNwords(), delete_flag,
                           temp_rawdatablk.GetNumEvents(), temp_rawdatablk.GetNumNodes());
    buf_rc = temp_rawdatablk.GetWholeBuffer();
  }
  if (buf_rc != NULL) {
    g_status.copyEventHeader(buf_rc);
  }

  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(exp_copper_0);
  m_eventMetaDataPtr->setRun(run_copper_0);
  m_eventMetaDataPtr->setSubrun(subrun_copper_0);
  m_eventMetaDataPtr->setEvent(eve_copper_0);

  setErrorFlag(error_bit_flag, m_eventMetaDataPtr);
  if (error_bit_flag != 0) {
    m_eventMetaDataPtr->addErrorFlag(EventMetaData::c_B2LinkEventCRCError);
    printf("[ERROR] error bit was detected. exp %d run %d eve %d count = %d\n",
           exp_copper_0, run_copper_0, eve_copper_0, error_bit_flag);
  }


  //
  // Run stop via NSM (Already obsolete. Need to ask Konno-san about implementation)
  //
  //     if (m_shmflag != 0) {
  //       if (n_basf2evt % 10 == 0) {
  //  if (g_status.isStopped()) {
  //           printf("[DEBUG] [INFO] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
  //           m_eventMetaDataPtr->setEndOfData();
  //         }
  //       }
  //     }

  //
  // Monitor
  //
  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
        || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n",
             max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 20000 == 0 || n_basf2evt < 10) {
    RateMonitor(eve_copper_0, subrun_copper_0, run_copper_0);
  }
  n_basf2evt++;
  if (g_status.isAvailable()) {
    g_status.setInputNBytes(m_totbytes);
    g_status.setInputCount(n_basf2evt);
  }

  return;
}


