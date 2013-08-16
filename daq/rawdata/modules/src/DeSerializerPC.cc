//+
// File : DeSerializerPC.cc
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DeSerializerPC.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

//#define MAXEVTSIZE 400000000
#define CHECKEVT 5000

#define NOT_USE_SOCKETLIB
#define CLONE_ARRAY
//#define DISCARD_DATA
//#define CHECK_SUM

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

  //Parameter definition
  B2INFO("DeSerializerPC: Constructor done.");
}


DeSerializerPCModule::~DeSerializerPCModule()
{

}



void DeSerializerPCModule::initialize()
{

  // Accept requests for connections
  Connect();

  // allocate buffer
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
    m_bufary_body[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];


  // initialize buffer
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
    memset(m_bufary_body[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  // Initialize Array of RawCOPPER
#ifdef CLONE_ARRAY
  rawcprarray.registerPersistent();
#else
  m_rawcopper.registerPersistent();
#endif

  if (dump_fname.size() > 0) {
    OpenOutputFile();
  }

  B2INFO("Rx initialized.");

  // Initialize arrays for time monitor
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));


}











int DeSerializerPCModule::Recv(int sock, char* buf, int data_size_byte, int flag)
{
  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = recv(sock, (char*)buf + n, data_size_byte - n , flag)) < 0) {
      perror("Failed to read header");
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
      perror("hostname cannot be resolved. Exiting...");
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
        perror("Faield to connect. Retrying...");
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
#ifdef debug
    printf("SO_RCVBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], SOL_SOCKET, SO_SNDBUF, &val, (socklen_t*)&len);
#ifdef debug
    printf("SO_SNDBUF %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_MAXSEG, &val, (socklen_t*)&len);
#ifdef debug
    printf("TCP_MAXSEG %d\n", val);
#endif
    getsockopt(m_socket[ i ], IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t*)&len);
#ifdef debug
    printf("TCP_NODELAY %d\n", val);
#endif

  }
  return 0;

}



int* DeSerializerPCModule::RecvDatafromCOPPER(int* malloc_flag, int* total_buf_nwords, int* default_buf)
{

  int* temp_buf = NULL;
  int flag = 0;
  vector <int> each_buf_nwords;
  *total_buf_nwords = 0;

  each_buf_nwords.clear();
#ifdef NOT_USE_SOCKETLIB
  //
  // Read Size
  //

  int read_size = 0;
  int recv_size = sizeof(int);

  int send_hdr_buf[ SendHeader::SENDHDR_NWORDS ];

  for (int i = 0; i < m_socket.size(); i++) {
    int recvd_size = 0;
    Recv(m_socket[ i ], (char*)send_hdr_buf, sizeof(int)*SendHeader::SENDHDR_NWORDS, flag);
    SendHeader send_hdr;
    send_hdr.SetBuffer(send_hdr_buf);
    int rawcpr_nwords = send_hdr.GetTotalNwords()
                        - SendHeader::SENDHDR_NWORDS
                        - SendTrailer::SENDTRL_NWORDS;
    *total_buf_nwords += rawcpr_nwords;
    each_buf_nwords.push_back(rawcpr_nwords);

  }

  if (*total_buf_nwords >  BUF_SIZE_WORD) {
    *malloc_flag = 1;
    temp_buf = new int[ *total_buf_nwords ];
  } else {
    *malloc_flag = 0;
    temp_buf = default_buf;
  }


  // Read body
  int total_recvd_byte = 0;
  for (int i = 0; i < m_socket.size(); i++) {
    total_recvd_byte += Recv(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
                             each_buf_nwords[ i ] * sizeof(int), flag);
  }
  if (*total_buf_nwords * sizeof(int) != total_recvd_byte) {
    perror("Receiving data in an invalid unit. Exting...");
    exit(-1);
  }

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
      cerr << "EoD is found without updating EventMetaData.";
      exit(-1); // Exit if EoD is found without updating EventMetaData
    } else if (tot_stat >  BUF_SIZE_WORD * sizeof(int)) {
      printf("Too large data( %d bytes ). Exiting...", stat);
      exit(-1);
    } else if (stat % sizeof(int) != 0) {
      printf("Recvd data size is not multple of sizeof(int). Exiting... : %d\n", stat);
      exit(-1);
    }

  }

  *total_buf_nwords = tot_stat / sizeof(int);

#endif // NOT_USE_SOCKETLIB

  return temp_buf;

}




int* DeSerializerPCModule::RecvDatafromEvb0(int* malloc_flag, int* total_buf_nwords, int* default_buf)
{

  int* temp_buf = NULL;
  int flag = 0;
  vector <int> each_buf_nwords;
  each_buf_nwords.clear();

  *total_buf_nwords = 0;

  //
  // Read Size
  //
  for (int i = 0; i < m_socket.size(); i++) {
    int ropc_nwords;
    Recv(m_socket[ i ], (char*)&ropc_nwords, sizeof(int), flag);
    *total_buf_nwords = ropc_nwords;
    each_buf_nwords.push_back(ropc_nwords);
  }

  if (*total_buf_nwords > BUF_SIZE_WORD) {
    *malloc_flag = 1;
    temp_buf = new int[ *total_buf_nwords ];
  } else {
    *malloc_flag = 0;
    temp_buf = default_buf;
  }

  // Read body
  int total_recvd_byte = 0;
  for (int i = 0; i < m_socket.size(); i++) {
    if ((total_recvd_byte % sizeof(int)) != 0) {
      printf("recvd buffer size is in the unit of word. Exiting... : %d bytes \n"
             , total_recvd_byte);
      exit(1);
    }
    temp_buf[ total_recvd_byte / sizeof(int) ] = each_buf_nwords[ i ];
    total_recvd_byte += sizeof(int);
    total_recvd_byte += Recv(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
                             (each_buf_nwords[ i ] - 1) * sizeof(int), flag);
  }

  if (*total_buf_nwords * sizeof(int) != total_recvd_byte) {
    perror("Receiving data in an invalid unit. Exting...");
    exit(-1);
  }

  return temp_buf;

}






void DeSerializerPCModule::event()
{

#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array0);
#endif

  if (n_basf2evt < 0) {
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

  // Make rawcopper array
#ifdef CLONE_ARRAY
  rawcprarray.create();
  // DataStore interface
  RawCOPPER* temp_rawcopper;
#else
  m_rawcopper.create();
#endif


  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    // Get a record from socket
    int total_buf_nwords = 0 ;
    int malloc_flag = 0;
    //    int* temp_buf = RecvDatafromCOPPER(&malloc_flag, &total_buf_nwords, m_bufary[ j ]);
    int* temp_buf = RecvDatafromEvb0(&malloc_flag, &total_buf_nwords, m_bufary[ j ]);
    m_totbytes += total_buf_nwords * sizeof(int);

    // Dump binary data
    if (dump_fname.size() > 0) {
      DumpData((char*)temp_buf, total_buf_nwords * sizeof(int));
    }

#ifndef DISCARD_DATA
#ifdef CLONE_ARRAY
    temp_rawcopper =  rawcprarray.appendNew();
    temp_rawcopper->SetBuffer((int*)temp_buf + SendHeader::SENDHDR_NWORDS, total_buf_nwords, malloc_flag);

#else
    //    m_rawcopper->buffer(temp_buf_body, body_size_word, malloc_flag_body);
#endif
#endif // DISCARD_DATA

  }

  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(n_basf2evt);

  //
  // Printing Event rate
  //
  if (n_basf2evt  % (CHECKEVT / NUM_EVT_PER_BASF2LOOP) == 0) {
    double cur_time = GetTimeSec();
    double total_time = cur_time - m_start_time;
    double interval = cur_time - m_prev_time;

    printf("Event %d EvtRate %.4lf [kHz] RcvdRate %.4lf [MB/s] TotalRecvd %.1lf [MB] ElapsedTime %.2lf [s] \n",
           n_basf2evt * NUM_EVT_PER_BASF2LOOP,
           (n_basf2evt - m_prev_nevt) / interval / 1.e3 * NUM_EVT_PER_BASF2LOOP, (m_totbytes - m_prev_totbytes) / interval / 1.e6,
           m_totbytes / 1.e6,
           total_time);
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_totbytes = m_totbytes;
    m_prev_nevt = n_basf2evt;
  }

#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array1);
  if (n_basf2evt == 51000) {
    for (int i = 0; i < 500; i++) {
      printf("%d %lf %lf %lf\n", i + 50000, time_array0[ i ], time_array1[ i ], time_array2[ i ]);
    }
  }
#endif

  n_basf2evt++;


  if (n_basf2evt * NUM_EVT_PER_BASF2LOOP >= max_nevt && max_nevt > 0) {
    m_eventMetaDataPtr->setEndOfData();
  }

  return;
}
