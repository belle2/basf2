//+
// File : Serializer.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/Serializer.h>

using namespace std;
using namespace Belle2;

#define NOT_USE_SOCKETLIB
//#define NOT_SEND
//#define DUMMY_DATA
#define TIME_MONITOR

//#define MULTIPLE_SEND
//#define MEMCPY_TO_ONE_BUFFER
#define SEND_BY_WRITEV

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Serializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SerializerModule::SerializerModule() : Module()
{
  //Set module properties

  setDescription("Encode DataStore into RingBuffer");
  addParam("DestPort", m_port_to, "Destination port", BASE_PORT_ROPC_COPPER);

  addParam("ProcessMethod", p_method, "Process method", string("COPPER"));

  addParam("LocalHostName", m_hostname_local, "local host", string(""));

#ifdef DUMMY
  addParam("EventDataBufferWords", BUF_SIZE_WORD, "DataBuffer words per event", 4800);
#endif
  n_basf2evt = -1;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Tx: Constructor done.");
}



SerializerModule::~SerializerModule()
{
}

void SerializerModule::initialize()
{



#ifdef DUMMY
  m_buffer = new int[ BUF_SIZE_WORD ];

#endif

  if (p_method == "COPPER") {
    p_method_val = 1;
  } else if (p_method == "ROPC") {
    p_method_val = 2;
  } else {
    printf("Please specify the data-handling");
    exit(1);
  }

  printf("METHOD %d %s %d\n", p_method_val, p_method.c_str(), m_port_to);


#ifndef NOT_SEND
#ifdef NOT_USE_SOCKETLIB

  Accept();
#else

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port_to);
  m_socket =  m_sock->socket()->sock();
#endif
#endif

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Tx initialized.");
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));



}


void SerializerModule::beginRun()
{
  B2INFO("beginRun called.");
}




void SerializerModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void SerializerModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//



void SerializerModule::FillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawCOPPER* rawcpr)
{

  int total_send_nwords =
    hdr->GetHdrNwords() +
    rawcpr->m_header.GetHdrNwords() +
    rawcpr->GetBodyNwords() +
    rawcpr->m_trailer.GetTrlNwords() +
    trl->GetTrlNwords();

  hdr->SetNwords(total_send_nwords);

  return;
}


void SerializerModule::SendOneBuffer(RawCOPPER* rawcpr)
{

  SendHeader send_header;
  SendTrailer send_trailer;

  int send_header_nwords = send_header.GetHdrNwords();

  int rawheader_nwords = rawcpr->m_header.GetHdrNwords();
  int rawtrailer_nwords = rawcpr->m_trailer.GetTrlNwords();
  int rawcopperbody_nwords =
    rawcpr->GetBodyNwords();
  int send_trailer_nwords = send_trailer.GetTrlNwords();

  int total_send_nwords = (send_header_nwords + rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords + send_trailer_nwords);
  send_header.SetNwords(total_send_nwords);

  int rawcopper_nwords = rawcpr->m_header.GetNwords();
  if (rawcopper_nwords != rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords) {
    printf("invalid data length of RawCOPEPR. Exiting... : %d %d %d %d\n",
           rawcopper_nwords,
           rawheader_nwords,
           rawcopperbody_nwords,
           rawtrailer_nwords
          );
    exit(-1);
  }



  int total_send_bytes = total_send_nwords * sizeof(int);
  char* send_buf = new char[ total_send_bytes ];
  memcpy(send_buf, send_header.GetBuffer(), send_header_nwords * sizeof(int));
  memcpy(send_buf + send_header_nwords * sizeof(int),
         rawcpr->m_header.GetBuffer(),
         rawheader_nwords * sizeof(int));

  memcpy(send_buf + (send_header_nwords + rawheader_nwords) * sizeof(int),
         rawcpr->GetBuffer(),
         rawcopperbody_nwords * sizeof(int));
  memcpy(send_buf + (send_header_nwords + rawheader_nwords + rawcopperbody_nwords) * sizeof(int),
         rawcpr->m_trailer.GetBuffer(),
         rawtrailer_nwords * sizeof(int));
  memcpy(send_buf + (send_header_nwords + rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords) * sizeof(int),
         send_trailer.GetBuffer(),
         send_trailer_nwords * sizeof(int));


  // Send data to Readout CP
  int send_bytes = total_send_bytes;
  int sent_bytes = 0;
  int current_size = send_bytes;
  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)send_buf + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error3");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }

#ifdef DEBUG
  if (n_basf2evt == 0) {
    for (int k = 0; k < send_bytes / sizeof(int); k++) {
      printf("0x%.8x ", *((int*)send_buf + k));
      if ((k + 1) % 10 == 0) printf("\n %6d : ", k);
    }
    printf("\n");
    printf("\n");
  }
#endif

  delete [] send_buf;

}


void SerializerModule::SendOneByOne(RawCOPPER* rawcpr)
{

  SendHeader send_header;
  SendTrailer send_trailer;

  int send_header_nwords = send_header.GetHdrNwords();
  int rawheader_nwords = rawcpr->m_header.GetHdrNwords();
  int rawcopperbody_nwords = rawcpr->GetBodyNwords();
  int rawtrailer_nwords = rawcpr->m_trailer.GetTrlNwords();
  int rawcopper_nwords = rawcpr->m_header.GetNwords();
  int send_trailer_nwords = send_trailer.GetTrlNwords();

  int total_send_nwords =
    send_header_nwords + rawheader_nwords + rawcopperbody_nwords
    + rawtrailer_nwords + send_trailer_nwords;

  send_header.SetNwords(total_send_nwords);

  if (rawcopper_nwords != rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords) {
    printf("invalid data length of RawCOPEPR. Exiting... : %d %d %d %d\n",
           rawcopper_nwords,
           rawheader_nwords,
           rawcopperbody_nwords,
           rawtrailer_nwords
          );
    exit(-1);
  }



  // Send SendHeader

  int send_bytes = sizeof(int) * send_header_nwords;
  int sent_bytes = 0;
  int current_size = send_bytes;
  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)(send_header.GetBuffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error1");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }

  // Send RawCopper Header
  send_bytes = sizeof(int) * rawheader_nwords;
  sent_bytes = 0;
  current_size = send_bytes;
  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)(rawcpr->m_header.GetBuffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error2");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }

  // Send Body
  send_bytes = sizeof(int) * rawcopperbody_nwords;
  sent_bytes = 0;
  current_size = send_bytes;

  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)(rawcpr->GetBuffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error3");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }

  // Send RawCOPPER Trailer
  send_bytes = sizeof(int) * rawtrailer_nwords;
  sent_bytes = 0;
  current_size = send_bytes;
  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)(rawcpr->m_trailer.GetBuffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error3");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }

  // Send SendTrailer

  send_bytes = sizeof(int) * send_trailer_nwords;
  sent_bytes = 0;
  current_size = send_bytes;
  while (send_bytes - sent_bytes > 0) {
    int n = 0;
    if ((n = send(m_socket, (char*)(send_trailer.GetBuffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
      perror("SEND error4");
      exit(1);
    }
    sent_bytes += n;
    current_size -= n;
  }
}



void SerializerModule::SendByWriteV(RawCOPPER* rawcpr)
{

  SendHeader send_header;
  SendTrailer send_trailer;
  FillSendHeaderTrailer(&send_header, &send_trailer, rawcpr);

  enum {
    NUM_BUFFER = 5
  };
  struct iovec iov[ NUM_BUFFER ];

  // check Body data size
  int rawheader_nwords = rawcpr->m_header.GetHdrNwords();
  int rawcopperbody_nwords = rawcpr->GetBodyNwords();
  int rawtrailer_nwords = rawcpr->m_trailer.GetTrlNwords();
  int rawcopper_nwords = rawcpr->m_header.GetNwords();

  if (rawcopper_nwords != rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords) {
    printf("invalid data length of RawCOPEPR. Exiting... : %d %d %d %d\n",
           rawcopper_nwords,
           rawheader_nwords,
           rawcopperbody_nwords,
           rawtrailer_nwords
          );
    exit(-1);
  }

  //Fill iov info.
  iov[0].iov_base = (char*)send_header.GetBuffer();
  iov[0].iov_len = sizeof(int) * send_header.GetHdrNwords();

  iov[1].iov_base = (char*)rawcpr->m_header.GetBuffer();
  iov[1].iov_len = sizeof(int) * rawheader_nwords;

  iov[2].iov_base = (char*)rawcpr->GetBuffer();
  iov[2].iov_len = sizeof(int) * rawcopperbody_nwords;

  iov[3].iov_base = (char*)rawcpr->m_trailer.GetBuffer();
  iov[3].iov_len = sizeof(int) * rawtrailer_nwords;

  iov[4].iov_base = (char*)send_trailer.GetBuffer();
  iov[4].iov_len = sizeof(int) * send_trailer.GetTrlNwords();


  // Send Multiple buffers
  int n = 0;
  if ((n = writev(m_socket, iov, NUM_BUFFER)) < 0) {
    perror("SEND error1");
    exit(1);
  }
  int total_send_bytes = sizeof(int) * send_header.GetTotalNwords();
  if (n != total_send_bytes) {
    perror("Failed to send all data");
    printf("Sent data length is not consistent. %d %d : Exiting...", n, total_send_bytes);
    exit(1);
  }


  return;

}


void SerializerModule::Accept()
{

#ifdef NOT_USE_SOCKETLIB
  //
  // Connect to cprtb01
  //

  struct hostent* host;
  host = gethostbyname(m_hostname_local.c_str());

  if (host == NULL) {
    printf("hostname cannot be resolved. Exiting...: %s \n", m_hostname_local.c_str());
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
    perror("Failed to set KEEPALIVE");
  }

  if (bind(fd_listen, (struct sockaddr*)&sock_listen, sizeof(struct sockaddr)) < 0) {
    printf("port %d : ", m_port_to);
    perror("Failed to bind");
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    perror("Failed in listen:");
    exit(-1);
  }

  //
  // Accept
  //
  int fd_accept;
  struct sockaddr_in sock_accept;
  printf("Accepting... : port %d server %s\n", m_port_to, m_hostname_local.c_str());
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

//   int flag = 1;
//   ret = setsockopt(fd_accept, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag) );

  m_socket = fd_accept;

#else
  // Open receiver sockets
  m_recv.push_back(new EvtSocketRecv(m_base_port + i));
  m_socket.push_back(m_recv[ i ]->socket()->sender());
#endif


  return;

}

double SerializerModule::GetTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void SerializerModule::RecordTime(int event, double* array)
{
  if (event >= 50000 && event < 50500) {
    array[ event - 50000 ] = GetTimeSec() - m_start_time;
  }
  return;
}


void SerializerModule::VerifyCheckSum(int* buf)     // Should be modified
{

  int check_sum = 0;
  for (int i = 0 ; i < buf[0]; i++) {
    if (i != 2)   check_sum += buf[i];
  }

  if (buf[2] != check_sum) {
    cout << "Invalid checksum : " << check_sum << " " << buf[1] << endl;
    exit(1);
  }
}


void SerializerModule::event()
{
  if (n_basf2evt <= 0) {
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

#ifdef TIME_MONITOR
  double cur_time;
  RecordTime(n_basf2evt, time_array0);
#endif

  StoreArray<RawCOPPER> rawcprarray;

  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    int* buf;
    int m_size_byte = 0;

#ifndef DUMMY_DATA
    //  StoreObjPtr<RawCOPPER> rawcopper;
    buf = rawcprarray[ j ]->GetBuffer();
    m_size_byte = rawcprarray[ j ]->Size() * sizeof(int);
#else
    m_size_byte = 1000;
    m_buffer[0] = (m_size_byte + 3) / 4;
    buf = m_buffer;
#endif

#ifdef TIME_MONITOR
    RecordTime(n_basf2evt, time_array1);
#endif

    //
    // Send data
    //

#ifdef DEBUG
    printf("Send loop : %d\n", j);
#endif

#ifndef NOT_SEND

#ifdef NOT_USE_SOCKETLIB

    switch (p_method_val) {
      case COPPER :

#ifdef MULTIPLE_SEND

        SendOneByOne(rawcprarray[ j ]);

#elif defined( MEMCPY_TO_ONE_BUFFER ) // MULIPLE_SEND

        SendOneBuffer(rawcprarray[ j ]);

#elif defined( SEND_BY_WRITEV ) // MULIPLE_SEND

        SendByWriteV(rawcprarray[ j ]);
#else
        perror("No SEND PARAMETER IS SPECIFIED.");
        exit(1);
#endif  // MULTIPLE_SEND
        break;

      case ROPC :

        // Send Body

        if (send(m_socket, (char*)buf, m_size_byte, MSG_NOSIGNAL) != m_size_byte) {
          perror("Failed to send data. Exiting...");
          exit(1);
        }

#ifdef DEBUG
        printf("size :: %d %d\n", m_size_byte, m_size_byte / sizeof(int));
        for (int i = 0; i < m_size_byte / sizeof(int); i++) {
          printf("0x%.8x ", *((int*)buf + i));
          if ((i + 1) % 10 == 0) printf("\n %6d :: ", i);
        }
        printf("\n");
        printf("\n");

#endif

        break;

      default :
        perror("Specify how to handle the data. Exiting...");
        exit(1);
    }
#else //NOT_USE_SOCKETLIB
    // Use basf2 send library
    m_sock->send_buffer(m_size_byte, (char*)buf);

#endif //NOT_USE_SOCKETLIB

    // Not send data

#endif //NOT_SEND


  }

#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array2);
#endif

  n_basf2evt++;

}
