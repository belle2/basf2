//+
// File : DeSerializerPC.cc
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DeSerializerPC.h>

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
  addParam("RecvBasePort", m_base_port, "Receiver Port Base Number", 0);
  addParam("NumConn", num_connections, "Number of Connections", 0);
  addParam("LocalHostName", m_local, "local host", string(""));



  //Parameter definition
  B2INFO("DeSerializerPC: Constructor done.");
}


DeSerializerPCModule::~DeSerializerPCModule()
{
}



void DeSerializerPCModule::initialize()
{



  // Accept requests for connections
  Accept();

  // allocate buffer
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

  // initialize buffer
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
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
    int total_m_size_word = 0 ;
    int malloc_flag = 0;
    int* temp_buf = RecvSocketData(&malloc_flag, &total_m_size_word, m_bufary[ j ]);

#ifndef DISCARD_DATA
#ifdef CLONE_ARRAY
    temp_rawcopper =  rawcprarray.appendNew();
    temp_rawcopper->buffer(temp_buf, total_m_size_word, malloc_flag);
#else
    m_rawcopper->buffer(temp_buf, total_m_size_word, malloc_flag);
#endif
#endif // DISCARD_DATA

    //
    // Dump data
    //
    if (dump_fname.size() > 0) {
      DumpData((char*)temp_buf, total_m_size_word * sizeof(int));
    }

    m_totbytes += total_m_size_word * sizeof(int);
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
  return;
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




int DeSerializerPCModule::Accept()
{



  for (unsigned int i = 0; i < num_connections; i++) {
#ifdef NOT_USE_SOCKETLIB
    //
    // Connect to cprtb01
    //
    m_base_port = 33000;
    struct hostent* host;
    host = gethostbyname(m_local.c_str());
    if (host == NULL) {
      perror("hostname cannot be resolved. Exiting...");
      exit(1);
    }

//     char ipad_srv[16];
//     strncpy( ipad_srv, m_local.c_str(), sizeof(ipad_srv) ); //"192.168.10.1";
    //    char ipad_srv[16]="127.0.0.1";
    int m_base_port_temp = m_base_port + i;

    int flags = 1;
    int domain = PF_INET;
    int type = SOCK_STREAM;
    int backlog = num_connections;

    //
    // Bind and listen
    //
    int fd_listen;
    struct sockaddr_in sock_listen;
    sock_listen.sin_family = AF_INET;
    sock_listen.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
    //    sock_listen.sin_addr.s_addr = inet_addr( ipad_srv );
    socklen_t addrlen = sizeof(sock_listen);
    sock_listen.sin_port = htons(m_base_port_temp);

    fd_listen = socket(domain, type, 0);
    int ret = setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &flags, (socklen_t)sizeof(flags));
    if (ret < 0) {
      perror("Failed to set KEEPALIVE");
    }

    if (bind(fd_listen, (struct sockaddr*)&sock_listen, sizeof(struct sockaddr)) < 0) {
      printf("port %d : ", m_base_port);
      perror("Failed to bind");
      exit(1);
    }

    int val1 = 0;
    setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
    if (listen(fd_listen, backlog) < 0) {
      perror("Failed in listen:");
      exit(-1);
    }

    //
    // Accept
    //
    int fd_accept;
    struct sockaddr_in sock_accept;
    printf("Accepting... : %d port %d server %s\n", i, m_base_port_temp, m_local.c_str());
    fflush(stdout);
    if ((fd_accept = accept(fd_listen, (struct sockaddr*) & (sock_accept), &addrlen)) == 0) {
      perror("Failed to accept. Exiting...");
      exit(-1);
    } else {
      printf("Connection is established: port %d from adress %d %s\n",
             htons(sock_accept.sin_port), sock_accept.sin_addr.s_addr, inet_ntoa(sock_accept.sin_addr));

      // set timepout option
      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      ret = setsockopt(fd_accept, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(timeout));
      if (ret < 0) {
        perror("Failed to set TIMEOUT. Exiting...");
        exit(-1);
      }
    }
    m_socket.push_back(fd_accept);

#else
    // Open receiver sockets
    m_recv.push_back(new EvtSocketRecv(m_base_port + i));
    m_socket.push_back(m_recv[ i ]->socket()->sender());
#endif

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




int* DeSerializerPCModule::RecvSocketData(int* malloc_flag, int* total_m_size_word, int* default_buf)
{

  int* temp_buf = NULL;
  int flag = 0;
  vector <int> m_size_word;
  *total_m_size_word = 0;

  m_size_word.clear();
#ifdef NOT_USE_SOCKETLIB
  //
  // Read Size
  //

  int read_size = 0;
  int recv_size = sizeof(int);

  for (int i = 0; i < m_socket.size(); i++) {
    int temp_m_size_word = 0 ;
    int recvd_size = 0;
    Recv(m_socket[ i ], (char*)&temp_m_size_word, sizeof(int), flag);
    *total_m_size_word += temp_m_size_word;
    m_size_word.push_back(temp_m_size_word);
  }

  if (*total_m_size_word >  BUF_SIZE_WORD) {
    *malloc_flag = 1;
    temp_buf = new int[ *total_m_size_word ];
  } else {
    *malloc_flag = 0;
    temp_buf = default_buf;
  }


  //
  // Read body
  //
  int total_recvd_byte = 0;
  for (int i = 0; i < m_socket.size(); i++) {

    // Store data-size
    int recvd_byte = sizeof(int); // size info
    if (total_recvd_byte % sizeof(int) != 0) {
      perror("Receiving data in an invalid unit. Exting...");
      exit(-1);
    } else {
      temp_buf[ total_recvd_byte / sizeof(int) ] = m_size_word[ i ];
      total_recvd_byte += recvd_byte;
    }

    // Store data-size
    total_recvd_byte += Recv(m_socket[ i ], (char*)temp_buf + total_recvd_byte,
                             m_size_word[ i ] * sizeof(int) - recvd_byte, flag);
  }

  if (*total_m_size_word * sizeof(int) != total_recvd_byte) {
    perror("Receiving data in an invalid unit. Exting...");
    exit(-1);
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

  *total_m_size_word = tot_stat / sizeof(int);

#endif // NOT_USE_SOCKETLIB

  return temp_buf;


}
