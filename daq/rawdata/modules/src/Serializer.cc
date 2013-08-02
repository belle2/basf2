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

#define MULTIPLE_SEND
//#define MEMCPY_TO_ONE_BUFFER
//#define SEND_BY_WRITEV

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
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  addParam("DestHostName", m_dest, "Destination host", string("localhost"));
  addParam("DestPort", m_port, "Destination port", BASE_PORT_ROPC_COPPER);
  addParam("ProcessMethod", p_method, "Process method", string("COPPER"));

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

  printf("METHOD %d %s %s %d\n", p_method_val, p_method.c_str(), m_dest.c_str(), m_port);


#ifndef NOT_SEND
#ifdef NOT_USE_SOCKETLIB

  Connect(m_dest.c_str(), m_port);

#else

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port);
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
    hdr->get_hdr_nwords() +
    rawcpr->m_header.get_hdr_nwords() +
    rawcpr->get_body_nwords() +
    rawcpr->m_trailer.get_trl_nwords() +
    trl->get_trl_nwords();

  hdr->set_nwords(total_send_nwords);

  return;
}


void SerializerModule::SendOneBuffer(RawCOPPER* rawcpr)
{

  SendHeader send_header;
  SendTrailer send_trailer;

  int send_header_nwords = send_header.get_hdr_nwords();

  int rawheader_nwords = rawcpr->m_header.get_hdr_nwords();
  int rawtrailer_nwords = rawcpr->m_trailer.get_trl_nwords();
  int rawcopperbody_nwords =
    rawcpr->get_body_nwords();
  int send_trailer_nwords = send_trailer.get_trl_nwords();

  int total_send_nwords = (send_header_nwords + rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords + send_trailer_nwords);
  send_header.set_nwords(total_send_nwords);

  int rawcopper_nwords = rawcpr->m_header.get_nwords();
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
  memcpy(send_buf, send_header.header(), send_header_nwords * sizeof(int));
  memcpy(send_buf + send_header_nwords * sizeof(int),
         rawcpr->m_header.header(),
         rawheader_nwords * sizeof(int));

  memcpy(send_buf + (send_header_nwords + rawheader_nwords) * sizeof(int),
         rawcpr->buffer(),
         rawcopperbody_nwords * sizeof(int));
  memcpy(send_buf + (send_header_nwords + rawheader_nwords + rawcopperbody_nwords) * sizeof(int),
         rawcpr->m_trailer.trailer(),
         rawtrailer_nwords * sizeof(int));
  memcpy(send_buf + (send_header_nwords + rawheader_nwords + rawcopperbody_nwords + rawtrailer_nwords) * sizeof(int),
         send_trailer.trailer(),
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

  int send_header_nwords = send_header.get_hdr_nwords();
  int rawheader_nwords = rawcpr->m_header.get_hdr_nwords();
  int rawcopperbody_nwords = rawcpr->get_body_nwords();
  int rawtrailer_nwords = rawcpr->m_trailer.get_trl_nwords();
  int rawcopper_nwords = rawcpr->m_header.get_nwords();
  int send_trailer_nwords = send_trailer.get_trl_nwords();

  int total_send_nwords =
    send_header_nwords + rawheader_nwords + rawcopperbody_nwords
    + rawtrailer_nwords + send_trailer_nwords;

  send_header.set_nwords(total_send_nwords);

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
    if ((n = send(m_socket, (char*)(send_header.header()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
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
    if ((n = send(m_socket, (char*)(rawcpr->m_header.header()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
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
    if ((n = send(m_socket, (char*)(rawcpr->buffer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
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
    if ((n = send(m_socket, (char*)(rawcpr->m_trailer.trailer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
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
    if ((n = send(m_socket, (char*)(send_trailer.trailer()) + sent_bytes, current_size, MSG_NOSIGNAL)) < 0) {
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
  int rawheader_nwords = rawcpr->m_header.get_hdr_nwords();
  int rawcopperbody_nwords = rawcpr->get_body_nwords();
  int rawtrailer_nwords = rawcpr->m_trailer.get_trl_nwords();
  int rawcopper_nwords = rawcpr->m_header.get_nwords();

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
  iov[0].iov_base = (char*)send_header.header();
  iov[0].iov_len = sizeof(int) * send_header.get_hdr_nwords();

  iov[1].iov_base = (char*)rawcpr->m_header.header();
  iov[1].iov_len = sizeof(int) * rawheader_nwords;

  iov[2].iov_base = (char*)rawcpr->buffer();
  iov[2].iov_len = sizeof(int) * rawcopperbody_nwords;

  iov[3].iov_base = (char*)rawcpr->m_trailer.trailer();
  iov[3].iov_len = sizeof(int) * rawtrailer_nwords;

  iov[4].iov_base = (char*)send_trailer.trailer();
  iov[4].iov_len = sizeof(int) * send_trailer.get_trl_nwords();


  // Send Multiple buffers
  int n = 0;
  if ((n = writev(m_socket, iov, NUM_BUFFER)) < 0) {
    perror("SEND error1");
    exit(1);
  }

  int total_send_bytes = sizeof(int) * send_header.get_nwords();
  if (n != total_send_bytes) {
    printf("Sent data length is not consistent. %d %d : Exiting...", n, total_send_bytes);
    exit(1);
  }


  return;

}


void SerializerModule::Connect(const char* hostname, const int port)
{

  //
  // Connect to a downstream node
  //
  struct sockaddr_in socORG;
  //  char ipad_org[16] = "192.168.10.1";
  //  socORG.sin_addr.s_addr = inet_addr( ipad_org );
  socORG.sin_family = AF_INET;

  struct hostent* host;
  host = gethostbyname(hostname);
  if (host == NULL) {
    perror("hostname cannot be resolved. Exiting...");
    exit(1);
  }
  socORG.sin_addr.s_addr =
    *(unsigned int*)host->h_addr_list[0];
  socORG.sin_port = htons(port);

  m_socket = socket(PF_INET, SOCK_STREAM, 0);

  int val1 = 0;
  setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &val1, sizeof(val1));

  printf("Connecting to %s port %d ...\n", hostname, port, m_socket);
  while (1) {
    if (connect(m_socket, (struct sockaddr*)(&socORG), sizeof(socORG)) < 0) {
      perror("Faield to connect. Retrying...");
      usleep(200000);
    } else {
      printf("Done\n");
      break;
    }
  }

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
    buf = rawcprarray[ j ]->buffer();
    m_size_byte = rawcprarray[ j ]->size() * sizeof(int);
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
