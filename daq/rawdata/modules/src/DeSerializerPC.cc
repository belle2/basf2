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

//#define DEBUG
//#define NO_DATA_CHECK

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
      m_status.open(m_nodename, m_nodeid);
    }
  }

  event_diff = 0;

  m_prev_copper_ctr = 0xFFFFFFFF;
  m_prev_evenum = 0xFFFFFFFF;

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
      } else {
        char err_buf[500];
        sprintf(err_buf, "Failed to receive data(%s). Exiting...", strerror(errno));
        print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        sleep(1234567);
        exit(-1);
      }
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
      char err_buf[100];
      sprintf(err_buf, "hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...", m_hostname_from[ i ].c_str(), strerror(errno));
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    }
    socPC.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    socPC.sin_port = htons(m_port_from[ i ]);
    int sd = socket(PF_INET, SOCK_STREAM, 0);

    int val1 = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

    fprintf(stderr, "Connecting to %s port %d ...\n", m_hostname_from[ i ].c_str(), m_port_from[ i ]);
    while (1) {
      if (connect(sd, (struct sockaddr*)(&socPC), sizeof(socPC)) < 0) {
        perror("Failed to connect. Retrying...");
        usleep(500000);
      } else {
        fprintf(stderr, "Done\n");
        break;
      }
    }
    m_socket.push_back(sd);

    // check socket paramters
    int val, len;
    len = sizeof(val);
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_RCVBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    fprintf(stderr, "SO_RCVBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_SNDBUF, &val, (socklen_t*)&len);
#ifdef DEBUG
    fprintf(stderr, "SO_SNDBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_MAXSEG, &val, (socklen_t*)&len);
#ifdef DEBUG
    fprintf(stderr, "TCP_MAXSEG %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t*)&len);
#ifdef DEBUG
    fprintf(stderr, "TCP_NODELAY %d\n", val);
#endif

  }
  fprintf(stderr, "Initialization finished\n");
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
      sprintf(err_buf, "DATA CORRUPTION: Different # of events or nodes over data sources( %d %d %d %d ). Exiting...\n",
              *num_events_in_sendblock , temp_num_events , *num_nodes_in_sendblock , temp_num_nodes);
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
      fprintf(stderr, "*******HDR**********\n");
      printData(send_hdr_buf, SendHeader::SENDHDR_NWORDS);
      char err_buf[500];
      sprintf(err_buf, "DATA CORRUPTION: Too large event : Header %d %d %d %d\n", i, temp_num_events, temp_num_nodes, send_hdr.GetTotalNwords());
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    char err_buf[500];
    sprintf(err_buf, "DATA CORRUPTION: Received data size (%d byte) is not same as expected one (%d) from Sendheader. Exting...",
            total_recvd_byte, (int)(*total_buf_nwords * sizeof(int)));
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(-1);
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

  int data_size_copper_0 = -1;
  int data_size_copper_1 = -1;


  unsigned int eve_copper_0 = 0;

  clearNumUsedBuf();

  if (m_start_flag == 0) {
    //
    // Connect to eb0: This should be here because we want Serializer first to accept connection from eb1tx
    //
    Connect();
    if (m_shmflag > 0) {
      B2INFO("DeSerializerPC: Waiting for Start...\n");
      m_status.reportRunning();
    }
    m_start_time = getTimeSec();
    n_basf2evt = 0;

  }

  // Make rawdatablk array
  raw_datablkarray.create();
  raw_cdcarray.create();
  raw_ftswarray.create();
  rawcprarray.create();
  raw_svdarray.create();
  raw_bpidarray.create();
  raw_epidarray.create();
  raw_eclarray.create();
  raw_klmarray.create();

  //
  // Mail loop
  //
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {

    eve_copper_0 = 0;

    //
    // Get a record from socket
    //
    int total_buf_nwords = 0 ;
    int malloc_flag = 0;
    int num_events_in_sendblock = 0;
    int num_nodes_in_sendblock = 0;


    if (m_start_flag == 0) {
      B2INFO("DeSerializerPC: Reading the 1st packet from eb0...");
    }
    int* temp_buf = recvData(&malloc_flag, &total_buf_nwords, &num_events_in_sendblock, &num_nodes_in_sendblock);
    if (m_start_flag == 0) {
      B2INFO("DeSerializerPC: Done. the size of the 1st packet " << total_buf_nwords << " words");
      m_start_flag = 1;
    }
    m_totbytes += total_buf_nwords * sizeof(int);

    //
    // Set buffer to the RawData class stored in DataStore
    //
    RawDataBlock* raw_datablk = raw_datablkarray.appendNew();
    raw_datablk->SetBuffer((int*)temp_buf, total_buf_nwords, malloc_flag,
                           num_events_in_sendblock, num_nodes_in_sendblock);

    //
    // check even # and node # in one Sendblock
    //
    int num_entries = raw_datablk->GetNumEntries();
    if (num_entries != num_events_in_sendblock * num_nodes_in_sendblock) {
      char err_buf[500];
      sprintf(err_buf, "DATA CORRUPTION: Inconsistent SendHeader value. # of nodes(%d) times # of events(%d) differs from # of entries(%d). Exiting...",
              num_nodes_in_sendblock, num_events_in_sendblock, num_entries);
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }

    //
    // Data check
    //
    int cpr_num = 0;
    unsigned int cur_evenum = 0, cur_copper_ctr = 0;
    unsigned int eve_array[32]; // # of noeds is less than 17
    unsigned int utime_array[32];// # of noeds is less than 17
    unsigned int ctime_type_array[32];// # of noeds is less than 17

    for (int k = 0; k < num_events_in_sendblock; k++) {
      memset(eve_array, 0, sizeof(eve_array));
      memset(utime_array, 0, sizeof(utime_array));
      memset(ctime_type_array, 0, sizeof(ctime_type_array));

      for (int l = 0; l < num_nodes_in_sendblock; l++) {
        int entry_id = l + k * num_nodes_in_sendblock;

        //
        // RawFTSW
        //
        if (raw_datablk->CheckFTSWID(entry_id)) {
          RawFTSW* temp_rawftsw = new RawFTSW;
          temp_rawftsw->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                  raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);
          if (temp_rawftsw->GetEveNo(0) < 10
             ) {
            fprintf(stderr, "######FTSW#########\n");
            printData((int*)temp_buf + raw_datablk->GetBufferPos(entry_id), raw_datablk->GetBlockNwords(entry_id));
          }


#ifndef NO_DATA_CHECK
          try {
            temp_rawftsw->CheckData(0, m_prev_evenum, &cur_evenum, m_prev_runsubrun_no, &m_runsubrun_no);
            eve_array[ entry_id ] = cur_evenum;
          } catch (string err_str) {
            print_err.PrintError(m_shmflag, &m_status, err_str);
            exit(1);
          }
#endif
          utime_array[ entry_id ] = temp_rawftsw->GetTTUtime(0);
          ctime_type_array[ entry_id ] = temp_rawftsw->GetTTCtimeTRGType(0);
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
            fprintf(stderr, "######TLU#########\n");
            printData((int*)temp_buf + raw_datablk->GetBufferPos(entry_id), raw_datablk->GetBlockNwords(entry_id));
          }

#ifndef NO_DATA_CHECK
          try {
            temp_rawtlu->CheckData(0, m_prev_evenum, &cur_evenum);
            eve_array[ entry_id ] = cur_evenum;
          } catch (string err_str) {
            print_err.PrintError(m_shmflag, &m_status, err_str);
            exit(1);
          }
#endif
          delete temp_rawtlu;
          //
          // RawCOPPER
          //
        } else {
          RawCOPPER* temp_rawcopper = new RawCOPPER;
          temp_rawcopper->SetBuffer((int*)temp_buf + raw_datablk->GetBufferPos(entry_id),
                                    raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);


#ifndef NO_DATA_CHECK
          try {
            temp_rawcopper->CheckData(0, m_prev_evenum, &cur_evenum,
                                      m_prev_copper_ctr, &cur_copper_ctr,
                                      m_prev_runsubrun_no, &m_runsubrun_no);
            eve_array[ entry_id ] = cur_evenum;
          } catch (string err_str) {
            print_err.PrintError(m_shmflag, &m_status, err_str);
            exit(1);
          }
#endif
          utime_array[ entry_id ] = temp_rawcopper->GetTTUtime(0);
          ctime_type_array[ entry_id ] = temp_rawcopper->GetTTCtimeTRGType(0);

          if (cpr_num == 0) {
            data_size_copper_0 = raw_datablk->GetBlockNwords(entry_id);
            eve_copper_0 = (raw_datablk->GetBuffer(entry_id))[ 3 ];
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
            fprintf(stderr, "node %d eve # %d utime %x ctime %x\n",
                    m,  eve_array[ m ], utime_array[ m ], ctime_type_array[ m ]);
          }
          sprintf(err_buf, "DATA CORRUPTION: Event or Time record mismatch. Exiting...");
          print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          sleep(1234567);
          exit(-1);
        }
      }
#endif

      // Event # monitor in runchange
      if (m_prev_runsubrun_no != m_runsubrun_no) {
        fprintf(stderr, "##############################################\n");
        for (int m = 0; m < num_entries ; m++) {
          fprintf(stderr, "%d eve %u prev %u\n", m, eve_array[ m ], m_prev_evenum);
        }
        fprintf(stderr, "##############################################\n");
        fflush(stderr);
      }
      m_prev_evenum = cur_evenum;
      m_prev_copper_ctr = cur_copper_ctr;
      m_prev_runsubrun_no = m_runsubrun_no;
    }
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
  //   if (m_shmflag != 0) {
  //     if (n_basf2evt % 10 == 0) {
  //       if (m_status.isStopped()) {
  //         fprintf( stderr, "\033[34m");
  //         fprintf( stderr, "[INFO] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
  //         fprintf( stderr, "\033[0m");
  //         m_eventMetaDataPtr->setEndOfData();
  //       }
  //     }
  //   }

  //
  // Monitor
  //
  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
        || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
      fprintf(stderr, "\033[34m");
      fprintf(stderr, "[INFO] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      fprintf(stderr, "\033[0m");
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 2000 == 0 || n_basf2evt < 10) {
    RateMonitor(eve_copper_0);
  }

  n_basf2evt++;

  return;
}
