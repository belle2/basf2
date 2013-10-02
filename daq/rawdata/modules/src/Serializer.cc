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
    print_err.PrintError("Please specify the data-handling",
                         __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
  const int num_cprblock = 0;
  RawHeader rawhdr;
  rawhdr.SetBuffer(rawcpr->GetRawHdrBufPtr(0));
  int total_send_nwords =
    hdr->GetHdrNwords() +
    rawcpr->TotalBufNwords() +
    //    rawhdr.GetNwords() +
    trl->GetTrlNwords();

  hdr->SetNwords(total_send_nwords);
  hdr->SetNumEventsinPacket(rawcpr->GetNumEvents());
  hdr->SetNumNodesinPacket(rawcpr->GetNumNodes());
  hdr->SetEventNumber(rawhdr.GetEveNo());
  hdr->SetNodeID(rawhdr.GetSubsysId());

  return;
}


void SerializerModule::SendByWriteV(RawCOPPER* rawcpr)
{
  const int num_cprblock = 0;

  SendHeader send_header;
  SendTrailer send_trailer;
  FillSendHeaderTrailer(&send_header, &send_trailer, rawcpr);

  enum {
    NUM_BUFFER = 3
  };
  struct iovec iov[ NUM_BUFFER ];

  // check Body data size
  int rawcopper_nwords = rawcpr->TotalBufNwords();

  //Fill iov info.
  iov[0].iov_base = (char*)send_header.GetBuffer();
  iov[0].iov_len = sizeof(int) * send_header.GetHdrNwords();

  iov[1].iov_base = (char*)rawcpr->GetWholeBuffer();
  iov[1].iov_len = sizeof(int) * rawcopper_nwords;

  iov[2].iov_base = (char*)send_trailer.GetBuffer();
  iov[2].iov_len = sizeof(int) * send_trailer.GetTrlNwords();


  //  int* temp_buf = new int[ iov[0].iov_len + iov[1].iov_len + iov[2].iov_len ];

  // Send Multiple buffers
  int n = 0;
  //  if (send(m_socket, (char*)temp_buf, iov[0].iov_len + iov[1].iov_len + iov[2].iov_len, MSG_NOSIGNAL) != iov[0].iov_len + iov[1].iov_len + iov[2].iov_len ) {
  if ((n = writev(m_socket, iov, NUM_BUFFER)) < 0) {
    print_err.PrintError("SEND error1", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int total_send_bytes = sizeof(int) * send_header.GetTotalNwords();
  if (n != total_send_bytes) {
    print_err.PrintError("Failed to send all data",
                         __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[ERROR] Sent data length is not consistent. %d %d : Exiting...", n, total_send_bytes);
    fflush(stdout);

//     for( int i = 0; i < total_send_bytes/4 ; i++){
//       printf("", );
//     }
    sleep(1234567);
    exit(1);
  }
  printf("n %d total %d\n", n, total_send_bytes);
  //  delete temp_buf;

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
    char temp_buf[500];
    sprintf(temp_buf, "[ERROR] hostname cannot be resolved. Check /etc/hosts. Exiting...: %s \n", m_hostname_local.c_str());
    print_err.PrintError(temp_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    print_err.PrintError("Failed to bind. Maybe other programs have already occupied this port. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    print_err.PrintError("Failed in listen", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    print_err.PrintError("Failed to accept. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
      print_err.PrintError("Failed to set TIMEOUT. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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


unsigned int SerializerModule::CalcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
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

  //  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
  for (int j = 0; j < rawcprarray.getEntries(); j++) {
    int* buf;
    int m_size_byte = 0;

#ifndef DUMMY_DATA
    //  StoreObjPtr<RawCOPPER> rawcopper;
    buf = rawcprarray[ j ]->GetWholeBuffer();
    m_size_byte = rawcprarray[ j ]->TotalBufNwords() * sizeof(int);
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
        print_err.PrintError("No SEND PARAMETER IS SPECIFIED.", __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
#endif  // MULTIPLE_SEND
        break;

      case ROPC :

        // Send Body
        if (send(m_socket, (char*)buf, m_size_byte, MSG_NOSIGNAL) != m_size_byte) {
          print_err.PrintError("Failed to send data. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
        print_err.PrintError("Specify how to handle the data. Exiting...", __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
