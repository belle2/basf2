//+
// File : DeSerializerPC.cc
// Description : Module to receive data from eb0 and send it to eb1tx
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerPC.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <sys/mman.h>

//#define MAXEVTSIZE 400000000
#define CHECKEVT 5000

#define NOT_USE_SOCKETLIB
#define CLONE_ARRAY
//#define DISCARD_DATA
//#define CHECK_SUM

//#define TIME_MONITOR
//#define DEBUG

#define NO_DATA_CHECK

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerPC)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

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
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  raw_datablkarray.registerPersistent();
  rawcprarray.registerPersistent();
  raw_cdcarray.registerPersistent();
  raw_svdarray.registerPersistent();
  raw_bpidarray.registerPersistent();
  raw_epidarray.registerPersistent();
  raw_eclarray.registerPersistent();
  raw_klmarray.registerPersistent();
  raw_ftswarray.registerPersistent();

  // Initialize Array of RawCOPPER

  if (dump_fname.size() > 0) {
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
      m_status.open(m_nodename, m_nodeid);
      m_status.reportReady();
    }
  }

  event_diff = 0;
  B2INFO("DeSerializerPC: initialize() done.");
}


int DeSerializerPCModule::recvFD(int sock, char* buf, int data_size_byte, int flag)
{
  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = recv(sock, (char*)buf + n, data_size_byte - n , flag)) < 0) {
      char temp_char[100];
      sprintf(temp_char, "Failed to read header");
      print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
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
    //
    // Connect to a downstream node
    //
    struct sockaddr_in socPC;
    socPC.sin_family = AF_INET;

    struct hostent* host;
    host = gethostbyname(m_hostname_from[ i ].c_str());
    if (host == NULL) {
      char temp_char[100];
      sprintf(temp_char, "hostname cannot be resolved. Check /etc/hosts. Exiting...");
      print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    }
    socPC.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    socPC.sin_port = htons(m_port_from[ i ]);
    int sd = socket(PF_INET, SOCK_STREAM, 0);

    int val1 = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

    printf("Connecting to %s port %d ...\n", m_hostname_from[ i ].c_str(), m_port_from[ i ]);
    while (1) {
      if (connect(sd, (struct sockaddr*)(&socPC), sizeof(socPC)) < 0) {
        perror("Failed to connect. Retrying...");
        usleep(500000);
      } else {
        printf("Done\n");
        break;
      }
    }
    m_socket.push_back(sd);

    // check socket paramters
    int val, len;
    len = sizeof(val);
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_RCVBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("SO_RCVBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_SNDBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("SO_SNDBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_MAXSEG, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("TCP_MAXSEG %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t*)&len);
#ifdef DEBUG
    printf("TCP_NODELAY %d\n", val);
#endif

  }
  printf("Initialization finished\n");
  return 0;

}



int* DeSerializerPCModule::recvData(int* malloc_flag, int* total_buf_nwords, int* num_events_in_sendblock, int* num_nodes_in_sendblock)
{

  int* temp_buf = NULL; // buffer for data-body
  int flag = 0;

  vector <int> each_buf_nwords;
  each_buf_nwords.clear();
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
      char err_buf[500];
      sprintf(err_buf, "[ERROR] Different # of events or nodes over data sources( %d %d %d %d ). Exiting...\n",
              *num_events_in_sendblock , temp_num_events , *num_nodes_in_sendblock , temp_num_nodes);
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

    if (rawblk_nwords > (int)(2.5e6)) {

      printf("*******HDR**********\n");
      printf("\n%.8d : ", 0);
      for (int j = 0; j < SendHeader::SENDHDR_NWORDS ; j++) {
        printf("0x%.8x ", send_hdr_buf[ j ]);
        if ((j + 1) % 10 == 0) {
          printf("\n%.8d : ", j + 1);
        }
      }
      printf("\n");
      printf("\n");

      char err_buf[500];
      sprintf(err_buf, "Too large event : Header %d %d %d %d\n", i, temp_num_events, temp_num_nodes, send_hdr.GetTotalNwords());
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(123456);
      exit(1);

    }


    each_buf_nwords.push_back(rawblk_nwords);

  }


  temp_buf = getBuffer(*total_buf_nwords, malloc_flag); // this include only data body
  //
  // Read body
  //
  int total_recvd_byte = 0;
  for (int i = 0; i < (int)(m_socket.size()); i++) {
    total_recvd_byte += recvFD(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
                               each_buf_nwords[ i ] * sizeof(int), flag);
  }
  if ((int)(*total_buf_nwords * sizeof(int)) != total_recvd_byte) {
    char temp_char[100] = "Receiving data in an invalid unit. Exting...";
    print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
  }

  if (temp_buf[20] != 0x7fff0008) {
    //#ifdef DEBUG
    printf("*******HDR**********\n");
    printf("\n%.8d : ", 0);
    for (int i = 0; i < SendHeader::SENDHDR_NWORDS ; i++) {
      printf("0x%.8x ", send_hdr_buf[ i ]);
      if ((i + 1) % 10 == 0) {
        printf("\n%.8d : ", i + 1);
      }
    }
    printf("\n");
    printf("\n");
    //#endif
    //#ifdef DEBUG
    printf("*******BODY**********\n");
    printf("\n%.8d : ", 0);
    //      for (int i = 0; i < *total_buf_nwords; i++) {
    for (int i = 0; i < 100; i++) {
      printf("0x%.8x ", temp_buf[ i ]);
      if ((i + 1) % 10 == 0) {
        printf("\n%.8d : ", i + 1);
      }
    }
    printf("\n");
    printf("\n");
    exit(1);
    //#endif
  }




  // Read Traeiler
  int send_trl_buf[(unsigned int)(SendTrailer::SENDTRL_NWORDS) ];
  for (int i = 0; i < (int)(m_socket.size()); i++) {
    recvFD(m_socket[ i ], (char*)send_trl_buf, SendTrailer::SENDTRL_NWORDS * sizeof(int), flag);
  }

  return temp_buf;

}





void DeSerializerPCModule::event()
{
  // For data check
  int num_copper_ftsw = -1;
  int data_size_copper_0 = -1;
  int data_size_copper_1 = -1;
  int data_size_ftsw = -1;

  unsigned int eve_copper_0 = 0;
  unsigned int temp_copper_ctr = 0;


  clearNumUsedBuf();

  //   printf("EVE %d\n",n_basf2evt);
  //   fflush(stdout);
  if (n_basf2evt < 0) {
    // Accept requests for connections
    Connect();
    if (m_shmflag > 0 && m_status.isStopped()) {
      printf("Waiting for Start...\n");
      fflush(stdout);
      m_status.waitStarted();
      m_status.reportRunning();
    }

    B2INFO("DeSerializerPC: event() started.");
    m_start_time = getTimeSec();
    n_basf2evt = 0;

  }

  // Make rawdatablk array
#ifdef CLONE_ARRAY
  raw_datablkarray.create();
  raw_cdcarray.create();
  raw_ftswarray.create();
  rawcprarray.create();
  raw_svdarray.create();
  raw_bpidarray.create();
  raw_epidarray.create();
  raw_eclarray.create();
  raw_klmarray.create();

#else
  m_rawdatablk.create();
#endif


  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {

    unsigned int eve_ftsw = 0;
    eve_copper_0 = 0;
    unsigned int eve_copper_1 = 0;


#ifdef TIME_MONITOR
    recordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC + j, time_array0);
#endif


#ifdef DEBUG
    printf("Read loop : j %d\n", j);
    fflush(stdout);
#endif

    // Get a record from socket
    int total_buf_nwords = 0 ;
    int malloc_flag = 0;
    int num_events_in_sendblock = 0;
    int num_nodes_in_sendblock = 0;
    int* temp_buf = recvData(&malloc_flag, &total_buf_nwords, &num_events_in_sendblock, &num_nodes_in_sendblock);

    m_totbytes += total_buf_nwords * sizeof(int);
#ifdef DEBUG
    printf("recvd data : %d bytes\n", total_buf_nwords * sizeof(int));
    fflush(stdout);
#endif


    // Dump binary data
#ifdef DEBUG

    printf("********* checksum 0x%.8x : %d\n" , CalcSimpleChecksum(temp_buf, total_buf_nwords - 2), total_buf_nwords);
    printf("\n%.8d : ", 0);
    for (int i = 0; i < total_buf_nwords; i++) {
      printf("0x%.8x ", temp_buf[ i ]);
      if ((i + 1) % 10 == 0) {
        printf("\n%.8d : ", i + 1);
      }
    }
    printf("\n");
    printf("\n");
#endif


#ifdef TIME_MONITOR
    recordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC + j, time_array1);
#endif

#ifndef DISCARD_DATA
#ifdef CLONE_ARRAY
    int temp_malloc_flag = 0;
    RawDataBlock rawdatablk;
    rawdatablk.SetBuffer((int*)temp_buf, total_buf_nwords, temp_malloc_flag,
                         num_events_in_sendblock, num_nodes_in_sendblock);

    RawDataBlock* temp_rawdatablk = raw_datablkarray.appendNew();
    temp_rawdatablk->SetBuffer((int*)temp_buf, total_buf_nwords, malloc_flag,
                               num_events_in_sendblock, num_nodes_in_sendblock);

    num_copper_ftsw = rawdatablk.GetNumEntries();
    int cpr_num = 0;


    for (int i = 0; i < rawdatablk.GetNumEntries(); i++) {
      if (i == 0) {
        temp_malloc_flag = malloc_flag ;
      } else {
        temp_malloc_flag = 0;
      }
#ifdef DEBUG
      printf("==========BODY==========\n");
      for (int k = 0 ; k < rawdatablk.GetBlockNwords(i); k++) {
        printf("0x%.8x ", (rawdatablk.GetBuffer(i))[k]);
        if (k % 10 == 9)printf("\n");
      }
      printf("========================\n");
#endif


      if (rawdatablk.CheckFTSWID(i)) {
        //  temp_rawftsw = raw_ftswarray.appendNew();
        RawFTSW* temp_rawftsw = new RawFTSW;
        temp_rawftsw->SetBuffer((int*)temp_buf + rawdatablk.GetBufferPos(i),
                                rawdatablk.GetBlockNwords(i), 0, 1, 1);
        data_size_ftsw = rawdatablk.GetBlockNwords(i);
        eve_ftsw = (rawdatablk.GetBuffer(i))[ 7 ] & 0xFFFF;
        delete temp_rawftsw;
      } else if (rawdatablk.CheckTLUID(i)) {
        // No operation
      } else {
        RawCOPPER* temp_rawcopper = new RawCOPPER;
        temp_rawcopper->SetBuffer((int*)temp_buf + rawdatablk.GetBufferPos(i),
                                  rawdatablk.GetBlockNwords(i), 0, 1, 1);
        temp_copper_ctr = temp_rawcopper->GetEveNo(0);

#ifndef NO_DATA_CHECK
        if (n_basf2evt != 0) {
          if ((unsigned int)(m_prev_copper_ctr + 1) != temp_copper_ctr) {
            char err_buf[500];
            sprintf(err_buf, "Differet COPPER counter : i %d prev 0x%x cur 0x%x : Exiting...\n",
                    i, m_prev_copper_ctr, temp_rawcopper->GetEveNo(0));
            //        i, m_prev_copper_ctr, 0 );
            print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//             sleep(1234567);
//             exit(1);
          }
        }
#endif
        RawTrailer rawtrl;
        rawtrl.SetBuffer(temp_rawcopper->GetRawTrlBufPtr(0));


#ifdef DEBUG
        printf("eve %d %d %d %d %d\n",
               //               temp_rawcopper->GetEveNo(0),
               temp_rawcopper->Get1stDetectorNwords(0),
               temp_rawcopper->Get2ndDetectorNwords(0),
               temp_rawcopper->Get3rdDetectorNwords(0),
               temp_rawcopper->Get4thDetectorNwords(0)
              );

        printf("===COPPER BLOCK==============\n");
        for (int k = 0 ; k < temp_rawcopper->GetBlockNwords(0); k++) {
          printf("0x%.8x ", (temp_rawcopper->GetBuffer(0))[k]);
          if (k % 10 == 9)printf("\n");
        }

        printf("===FINNESSE A ==============\n");
        for (int k = 0 ; k < temp_rawcopper->Get1stDetectorNwords(0); k++) {
          printf("0x%.8x ", (temp_rawcopper->Get1stDetectorBuffer(0))[k]);
          if (k % 10 == 9)printf("\n");
        }

        printf("===FINNESSE B ==============\n");
        for (int k = 0 ; k < temp_rawcopper->Get2ndDetectorNwords(0); k++) {
          printf("0x%.8x ", (temp_rawcopper->Get2ndDetectorBuffer(0))[k]);
          if (k % 10 == 9)printf("\n");
        }

        printf("===FINNESSE C ==============\n");
        for (int k = 0 ; k < temp_rawcopper->Get3rdDetectorNwords(0); k++) {
          printf("0x%.8x ", (temp_rawcopper->Get3rdDetectorBuffer(0))[k]);
          if (k % 10 == 9)printf("\n");
        }

        printf("===FINNESSE D ==============\n");
        for (int k = 0 ; k < temp_rawcopper->Get4thDetectorNwords(0); k++) {
          printf("0x%.8x ", (temp_rawcopper->Get4thDetectorBuffer(0))[k]);
          if (k % 10 == 9)printf("\n");
        }
        printf("=== END ==============\n");

#endif

        if (temp_rawcopper->GetDriverChkSum(0) != temp_rawcopper->CalcDriverChkSum(0)) {
          //#ifndef NO_DATA_CHECK
          char err_buf[500];
          printf("==========temp_buf==========\n");
          for (int k = 0 ; k < 100; k++) {
            printf("0x%.8x ", temp_buf[k]);
            if (k % 10 == 9)printf("\n");
          }

          printf("==========Header==========\n");
          for (int k = 0 ; k < 100; k++) {
            printf("0x%.8x ", (temp_rawcopper->GetBuffer(0))[k]);
            if (k % 10 == 9)printf("\n");
          }

          printf("Trl 0 0x%.8x\n", (temp_rawcopper->GetRawTrlBufPtr(0))[0]);
          printf("Trl 1 0x%.8x\n", (temp_rawcopper->GetRawTrlBufPtr(0))[1]);
          sprintf(err_buf, "COPPER driver checkSum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n",
                  i,
                  temp_rawcopper->GetBlockNwords(0),
                  (rawdatablk.GetBuffer(i))[ 3 ],
                  temp_rawcopper->GetDriverChkSum(0),
                  temp_rawcopper->CalcDriverChkSum(0));

          print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          sleep(1234567);
          exit(-1);
          //#endif
        }


        if (rawtrl.GetChksum() !=
            calcXORChecksum(temp_rawcopper->GetBuffer(0),
                            temp_rawcopper->GetBlockNwords(0) - rawtrl.GetTrlNwords())) {

          //#ifndef NO_DATA_CHECK
          char err_buf[500];
          printf("==========temp_buf==========\n");
          for (int k = 0 ; k < 100; k++) {
            printf("0x%.8x ", temp_buf[k]);
            if (k % 10 == 9)printf("\n");
          }

          printf("==========Header==========\n");
          for (int k = 0 ; k < 100; k++) {
            printf("0x%.8x ", (temp_rawcopper->GetBuffer(0))[k]);
            if (k % 10 == 9)printf("\n");
          }

          printf("Trl 0 0x%.8x\n", (temp_rawcopper->GetRawTrlBufPtr(0))[0]);
          printf("Trl 1 0x%.8x\n", (temp_rawcopper->GetRawTrlBufPtr(0))[1]);
          sprintf(err_buf, "CheckSum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n",
                  i,
                  temp_rawcopper->GetBlockNwords(0),
                  (rawdatablk.GetBuffer(i))[ 3 ],
                  rawtrl.GetChksum(),
                  calcSimpleChecksum(temp_rawcopper->GetBuffer(0),
                                     temp_rawcopper->GetBlockNwords(0)
                                     - rawtrl.GetTrlNwords()));

          print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          sleep(1234567);
          exit(-1);
          //#endif
        }

        if (cpr_num == 0) {
          data_size_copper_0 = rawdatablk.GetBlockNwords(i);
          eve_copper_0 = (rawdatablk.GetBuffer(i))[ 3 ];
        } else if (cpr_num == 1) {
          data_size_copper_1 = rawdatablk.GetBlockNwords(i);
          eve_copper_1 = (rawdatablk.GetBuffer(i))[ 3 ];
        }
        cpr_num++;
        delete temp_rawcopper;
      }
    }

#ifndef NO_DATA_CHECK
    if (eve_copper_1 != 0 && (eve_copper_0 != eve_copper_1)) {
      char err_buf[500];
      sprintf(err_buf, "Differet Event number over COPPERS COPPER0 %u COPPER1 %u\n", eve_copper_0, eve_copper_1);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//       sleep(1234567);
//       exit(-1);
    }

    if (eve_ftsw != 0 && ((eve_copper_0 & 0xFFFF) != eve_ftsw + (int)event_diff)) {
      char err_buf[500];
      event_diff = (eve_copper_0 & 0xFFFF) - eve_ftsw;
      sprintf(err_buf, "Different Event number c0 %u ftsw %u diff %d\n", (eve_copper_0 & 0xFFFF), eve_ftsw, event_diff);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//       sleep(1234567);
//       exit(-1);
    }
#endif

#else
    //    m_rawdatablk->buffer(temp_buf_body, body_size_word, malloc_flag_body);
#endif
#endif // DISCARD_DATA

#ifdef TIME_MONITOR
    recordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC + j, time_array2);
#endif

    m_prev_copper_ctr = temp_copper_ctr;
  }


  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(n_basf2evt);



  //
  // Shsared memory
  //
  if (m_shmflag != 0) {
    if (n_basf2evt % 10 == 0) {
      if (m_status.isStopped()) {
        printf("\033[34m");
        printf("[INFO] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
        printf("\033[0m");
        m_eventMetaDataPtr->setEndOfData();
      }
    }
  }

  //
  // Monitor
  //
  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
        || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
      printf("\033[34m");
      printf("[INFO] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      printf("\033[0m");
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 100 == 0) {
    //  if ( ( n_basf2evt - m_prev_nevt ) > monitor_numeve ) {
    double cur_time = getTimeSec();
    double interval = cur_time - m_prev_time;
    if (n_basf2evt != 0) {
      double multieve = (1. / interval);
      if (multieve > 2.) multieve = 2.;
      monitor_numeve = (int)(multieve * monitor_numeve) + 1;
    }
    printf("loop %d eve %u time %lf : # of nodes %d : ftsw words %d  copper0 words %d copper1 words %d\n",
           n_basf2evt,
           eve_copper_0,
           getTimeSec() - m_start_time,
           num_copper_ftsw,
           data_size_ftsw,
           data_size_copper_0,
           data_size_copper_1);
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_nevt = n_basf2evt;
  }

  n_basf2evt++;

  return;
}
