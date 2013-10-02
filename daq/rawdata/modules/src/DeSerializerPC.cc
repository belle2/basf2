//+
// File : DeSerializerPC.cc
// Description : Module to receive data from outside and store it to DataStore
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
  // Accept requests for connections
  Connect();

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

  // Initialize Array of RawCOPPER
  raw_datablkarray.registerPersistent();


  if (dump_fname.size() > 0) {
    OpenOutputFile();
  }



  // Initialize arrays for time monitor
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));

  ClearNumUsedBuf();


  // Shared memory
  if (m_shmflag != 0) {
    ShmOpen("/ropc_config", "/ropc_status");
    m_cfg_buf = ShmGet(m_shmfd_cfg, 4);
    m_cfg_sta = ShmGet(m_shmfd_sta, 4);
  }
  B2INFO("DeSerializerPC: initialize() done.");
}











int DeSerializerPCModule::Recv(int sock, char* buf, int data_size_byte, int flag)
{
  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = recv(sock, (char*)buf + n, data_size_byte - n , flag)) < 0) {
      print_err.PrintError("Failed to read header", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
  for (unsigned int i = 0; i < m_num_connections; i++) {
    //
    // Connect to a downstream node
    //
    struct sockaddr_in socPC;
    socPC.sin_family = AF_INET;

    struct hostent* host;
    host = gethostbyname(m_hostname_from[ i ].c_str());
    if (host == NULL) {
      print_err.PrintError("hostname cannot be resolved. Check /etc/hosts. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(1);
    }
    socPC.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    socPC.sin_port = htons(m_port_from[ i ]);
    int sd = socket(PF_INET, SOCK_STREAM, 0);

    int val1 = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

    printf("Connecting to %s port %d ...\n", m_hostname_from[ i ].c_str(), m_port_from[ i ], sd);
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
  return 0;

}



int* DeSerializerPCModule::RecvData(int* malloc_flag, int* total_buf_nwords, int* num_events_in_sendblock, int* num_nodes_in_sendblock)
{

  int* temp_buf = NULL; // buffer for data-body
  int flag = 0;

  vector <int> each_buf_nwords;
  each_buf_nwords.clear();

  *total_buf_nwords = 0;
  *num_nodes_in_sendblock = 0;
  *num_events_in_sendblock = 0;

#ifdef NOT_USE_SOCKETLIB


  // Read Header and obtain data size
  int read_size = 0;
  int recv_size = sizeof(int);

  int send_hdr_buf[ SendHeader::SENDHDR_NWORDS ];

  int temp_num_events = 0;
  int temp_num_nodes = 0;

  // Read header
  for (int i = 0; i < m_socket.size(); i++) {
    int recvd_size = 0;
    Recv(m_socket[ i ], (char*)send_hdr_buf, sizeof(int)*SendHeader::SENDHDR_NWORDS, flag);
    SendHeader send_hdr;
    send_hdr.SetBuffer(send_hdr_buf);

    temp_num_events = send_hdr.GetNumEventsinPacket();
    temp_num_nodes = send_hdr.GetNumNodesinPacket();

    if (i == 0) {
      *num_events_in_sendblock = temp_num_events;
    } else if (*num_events_in_sendblock != temp_num_events) {
      char err_buf[500];
      sprintf(err_buf, "[ERROR] Different # of events or nodes over data sources( %d %d %d %d ). Exiting...\n", *num_events_in_sendblock , temp_num_events , *num_nodes_in_sendblock , temp_num_nodes);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(1);
    }
    *num_nodes_in_sendblock += temp_num_nodes;

    int rawblk_nwords = send_hdr.GetTotalNwords()
                        - SendHeader::SENDHDR_NWORDS
                        - SendTrailer::SENDTRL_NWORDS;
    *total_buf_nwords += rawblk_nwords;

    each_buf_nwords.push_back(rawblk_nwords);

#ifdef DEBUG
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
#endif
  }



  temp_buf = GetBuffer(*total_buf_nwords, malloc_flag);
  // Read body
  int total_recvd_byte = 0;
  for (int i = 0; i < m_socket.size(); i++) {
    total_recvd_byte += Recv(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
                             each_buf_nwords[ i ] * sizeof(int), flag);
  }
  if (*total_buf_nwords * sizeof(int) != total_recvd_byte) {
    print_err.PrintError("Receiving data in an invalid unit. Exting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

#ifdef DEBUG
  printf("*******BODY**********\n");
  printf("\n%.8d : ", 0);
  for (int i = 0; i < *total_buf_nwords; i++) {
    printf("0x%.8x ", temp_buf[ i ]);
    if ((i + 1) % 10 == 0) {
      printf("\n%.8d : ", i + 1);
    }
  }
  printf("\n");
  printf("\n");
#endif


  // Read Traeiler
  int send_trl_buf[ SendTrailer::SENDTRL_NWORDS ];
  for (int i = 0; i < m_socket.size(); i++) {
    Recv(m_socket[ i ], (char*)send_trl_buf, SendTrailer::SENDTRL_NWORDS * sizeof(int), flag);
  }


#else // NOT_USE_SOCKETLIB

  int tot_stat = 0;
  temp_buf = default_buf;
  for (int i = 0; i < m_socket.size(); i++) {
    int stat = m_recv[i]->recv_buffer((char*)temp_buf + tot_stat);
    tot_stat += stat;

    printf("connection %d STAT %d\n",  i,  stat);
    fflush(stdout);

    if (stat <= 0) {
      print_err.PrintError("EoD is found without updating EventMetaData.", __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1); // Exit if EoD is found without updating EventMetaData
    } else if (tot_stat >  BUF_SIZE_WORD * sizeof(int)) {
      char err_buf[500];
      sprintf(err_buf, "Too large data( %d bytes ). Exiting....", stat);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    } else if (stat % sizeof(int) != 0) {
      char err_buf[500];
      sprintf(err_buf, "Recvd data size is not multple of sizeof(int). Exiting... : %d\n", stat);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }

  }

  *total_buf_nwords = tot_stat / sizeof(int);

#endif // NOT_USE_SOCKETLIB

  return temp_buf;

}





void DeSerializerPCModule::event()
{
  ClearNumUsedBuf();


  if (n_basf2evt < 0) {
    B2INFO("DeSerializerPC: event() started.");
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

  // Make rawdatablk array
#ifdef CLONE_ARRAY
  raw_datablkarray.create();
  // DataStore interface
  RawDataBlock* temp_rawdatablk;
#else
  m_rawdatablk.create();
#endif



  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {

#ifdef TIME_MONITOR
    RecordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP + j, time_array0);
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
    int* temp_buf = RecvData(&malloc_flag, &total_buf_nwords, &num_events_in_sendblock, &num_nodes_in_sendblock);
    m_totbytes += total_buf_nwords * sizeof(int);

#ifdef DEBUG
    printf("Recvd data : %d bytes\n", total_buf_nwords * sizeof(int));
    fflush(stdout);
#endif

    // Dump binary data

#ifdef DEBUG

    printf("********* checksum 0x%.8x : %d\n" , CalcSimpleChecksum(temp_buf, total_buf_nwords - 2), total_buf_nwords);
    printf("eve %d nodes %d\n", num_events_in_sendblock, num_nodes_in_sendblock);
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
    RecordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP + j, time_array1);
#endif

#ifndef DISCARD_DATA
#ifdef CLONE_ARRAY
    temp_rawdatablk =  raw_datablkarray.appendNew();

    // NotRemove SendHeader
    //    printf("Node %d numblock %d \n", temp_buf[6], m_socket.size());
    temp_rawdatablk->SetBuffer((int*)temp_buf, total_buf_nwords, malloc_flag, num_events_in_sendblock, m_socket.size()*num_nodes_in_sendblock);


    // Remove SendHeader
    //    temp_rawdatablk->SetBuffer((int*)temp_buf + SendHeader::SENDHDR_NWORDS, total_buf_nwords, malloc_flag);
#else
    //    m_rawdatablk->buffer(temp_buf_body, body_size_word, malloc_flag_body);
#endif
#endif // DISCARD_DATA

#ifdef TIME_MONITOR
    RecordTime(n_basf2evt * NUM_EVT_PER_BASF2LOOP + j, time_array2);
#endif

//     {
//       RawHeader rawhdr;
//       SendHeader hdr;
//       SendTrailer trl;
//       rawhdr.SetBuffer( temp_buf );
//       int total_send_nwords =
//  hdr.GetHdrNwords() +
//  temp_rawdatablk->TotalBufNwords()*2 +
//  trl.GetTrlNwords();
//       int subsys_id = rawhdr.GetSubsysId();

//       hdr.SetNwords(total_send_nwords);
//       hdr.SetNumEventsinPacket(temp_rawdatablk->GetNumEvents());
//       hdr.SetNumNodesinPacket(temp_rawdatablk->GetNumNodes());
//       hdr.SetEventNumber(rawhdr.GetEveNo());
//       hdr.SetNodeID( 0x00010000 );

//       if (dump_fname.size() > 0) {
//  if( subsys_id == 1 ){
//  DumpData((char*)hdr.GetBuffer(), hdr.GetHdrNwords() * sizeof(int));
//  }
//  DumpData((char*)temp_buf, total_buf_nwords * sizeof(int));
//  if( subsys_id == 2 ){
//  DumpData((char*)trl.GetBuffer(), trl.GetTrlNwords() * sizeof(int));
//  }
//       }
//     }


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
      if (m_cfg_buf[ 0 ] == 0) m_eventMetaDataPtr->setEndOfData();
    }
  }

  //
  // Monitor
  //
  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP >= max_nevt && max_nevt > 0)
        || (GetTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {

      printf("################## check 5 maxevt %d  mattime %lf %d %lf\n", max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP, GetTimeSec() - m_start_time);
      m_eventMetaDataPtr->setEndOfData();
    }
  }
  //  if(n_basf2evt % 100 == 0 ){
//    printf("eve %d time %lf\n",n_basf2evt, GetTimeSec() - m_start_time );
//     fflush(stdout);
  //  }

  n_basf2evt++;

  return;
}
