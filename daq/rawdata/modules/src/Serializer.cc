/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/Serializer.h>
#include <daq/rawdata/modules/DeSerializer.h>

#include <netinet/tcp.h>

#include <sys/uio.h>

#include <csignal>
#include <fcntl.h>

using namespace std;
using namespace Belle2;

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
  addParam("LocalHostName", m_hostname_local, "local host", string(""));
#ifdef DUMMY
  addParam("EventDataBufferWords", BUF_SIZE_WORD, "DataBuffer words per event", 4800);
#endif
  addParam("use Shared Memory", m_shmflag, "m_shmflag", 0);

  m_start_flag = 0;
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
  signal(SIGPIPE , SIG_IGN);

#ifdef DUMMY
  m_buffer = new int[ BUF_SIZE_WORD ];
#endif


  if (m_shmflag != 0) {
    char temp_char1[100] = "/cpr_config";
    char temp_char2[100] = "/cpr_status";
    shmOpen(temp_char1, temp_char2);
    // Status format : status_flag
    m_cfg_buf = shmGet(m_shmfd_cfg, 4);
    m_cfg_sta = shmGet(m_shmfd_sta, 4);
    m_cfg_sta[ 0 ] = 1; // Status bit is 1 : ready before accept()
  }

  // Create Message Handler
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));

  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.isAvailable()) {
    status.setOutputNBytes(0);
    status.setOutputCount(0);
  }

  Accept();

#ifdef NONSTOP
  openRunPauseNshm();
#endif

  B2INFO("Tx initialized.");
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



int* SerializerModule::shmGet(int fd, int size_words)
{
  int offset = 0;
  return (int*)mmap(NULL, size_words * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
}

void SerializerModule::shmOpen(char* path_cfg, char* path_sta)
{
  errno = 0;
  m_shmfd_cfg = shm_open(path_cfg, O_RDWR, 0666);
  if (m_shmfd_cfg < 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to shm_open (%s). Exiting... : path %s\n" ,
            strerror(errno), path_cfg);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }

  m_shmfd_sta = shm_open(path_sta , O_RDWR, 0666);
  if (m_shmfd_sta < 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to shm_open (%s). Exiting... : path %s\n" ,
            strerror(errno), path_sta);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }

  int size = 4 * sizeof(int);
  ftruncate(m_shmfd_cfg, size);
  ftruncate(m_shmfd_sta, size);
}



void SerializerModule::fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl,
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
      sprintf(err_buf, "[FATAL] Length error. total length %d rawdblk length %d. Exting...\n" ,
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

      char err_buf[500] = "[FATAL] CORRUPTED DATA: No COPPER blocks in RawDataBlock. Exiting...";
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(-1);
    }
  }
  return;
}


int SerializerModule::sendByWriteV(RawDataBlock* rawdblk)
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
    if ((n = writev(m_socket, iov, NUM_BUFFER)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {

#ifdef NONSTOP
        // check run-pause request
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf(err_buf, "[WARNING] WRITEVa error.(%s) : sent %d bytes, header %d bytes body %d trailer %d : %s %s %d\n" ,
                strerror(errno), n, iov[0].iov_len, iov[1].iov_len, iov[2].iov_len,
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
        g_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);  // Go to DeSerializer** and wait for run-resume.
#else
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
#endif
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
      n += Send(m_socket, (char*)iov[ 0 ].iov_base + n, iov[ 0 ].iov_len - n);
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len)) {
      n += Send(m_socket, (char*)iov[ 1 ].iov_base + (n - iov[ 0 ].iov_len), iov[ 1 ].iov_len - (n - iov[ 0 ].iov_len));
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len + iov[ 2 ].iov_len)) {
      n += Send(m_socket, (char*)iov[ 2 ].iov_base + (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len),
                iov[ 2 ].iov_len - (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len));
    }

    double retry_end = getTimeSec();
    B2WARNING("Resending ends. It takes " << retry_end - retry_start << "(s)");
  }

  return total_send_bytes;

}



int SerializerModule::Send(int socket, char* buf, int size_bytes)
{
  int sent_bytes = 0;
  while (true) {
    int ret = 0;
    if ((ret = send(socket, buf + sent_bytes, size_bytes - sent_bytes,  MSG_NOSIGNAL)) < 0) {
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
        sprintf(err_buf, "[ERROR] Send Error. (%s) : %s %s %d", strerror(errno), __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifdef NONSTOP
        g_run_error = 1;
        B2ERROR(err_buf);
        string err_str = "RUN_ERROR";
        throw (err_str);
#else
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
#endif
      }
    }
    sent_bytes += ret;
    if (sent_bytes == size_bytes) break;
  }
  return sent_bytes;
}

void SerializerModule::Accept()
{

  //
  // Connect to cprtb01
  //

  struct hostent* host;
  host = gethostbyname(m_hostname_local.c_str());
  if (host == NULL) {
    char temp_buf[500];
    sprintf(temp_buf, "[FATAL] hostname(%s) cannot be resolved(%s). Check /etc/hosts. Exiting...\n",
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
    sprintf(temp_char, "[FATAL] Failed to bind.(%s) Maybe other programs have already occupied this port(%d). Exiting...",
            strerror(errno), m_port_to);
    print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed in listen(%s). Exting...", strerror(errno));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  //
  // Accept
  //
  int fd_accept;
  struct sockaddr_in sock_accept;
  //  printf( "[DEBUG] Accepting... : port %d server %s\n", m_port_to, m_hostname_local.c_str());
  //  fflush(stderr);
  //  B2INFO("Accepting... : port " << m_port_to << " server " << m_hostname_local.c_str() );
  B2INFO("Accepting...");
  if ((fd_accept = accept(fd_listen, (struct sockaddr*) & (sock_accept), &addrlen)) == 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to accept(%s). Exiting...", strerror(errno));
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
      char temp_char[100] = "[FATAL] Failed to set TIMEOUT. Exiting...";
      print_err.PrintError(temp_char, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
  }
  close(fd_listen);

  //   int flag = 1;
  //   ret = setsockopt(fd_accept, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag) );
  m_socket = fd_accept;
  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.isAvailable()) {
    //status.setOutputPort(ntohs(sock_accept.sin_port));
    //status.setOutputAddress(sock_accept.sin_addr.s_addr);
    status.setOutputPort(ntohs(sock_listen.sin_port));
    status.setOutputAddress(sock_listen.sin_addr.s_addr);
    printf("%d %x\n", (int)ntohs(sock_listen.sin_port), (int)sock_listen.sin_addr.s_addr);
  }

  return;

}

double SerializerModule::getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void SerializerModule::recordTime(int event, double* array)
{
  if (event >= 50000 && event < 50500) {
    array[ event - 50000 ] = getTimeSec() - m_start_time;
  }
  return;
}


unsigned int SerializerModule::calcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


void SerializerModule::printData(int* buf, int nwords)
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
void SerializerModule::openRunPauseNshm()
{
  char path_shm[100] = "/cpr_pause_resume";
  int fd = shm_open(path_shm, O_RDONLY, 0666);
  if (fd < 0) {
    printf("[DEBUG] %s\n", path_shm);
    perror("[FATAL] Failed to open shm_open");
    exit(1);
  }
  m_ptr = (int*)mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
  return;
}

int SerializerModule::checkRunPause()
{

#ifdef NONSTOP_SLC
  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.getState() == status.PAUSING) {
#else
  if (*m_ptr) {
#endif
    return 1;
  } else {
    return 0;
  }
}

void SerializerModule::resumeRun()
{
  if (CheckConnection(m_socket) < 0) Accept();
  g_run_resuming = 0; // run_resuming phase is over.
  return;
}


int SerializerModule::CheckConnection(int socket)
{
  // Modify Yamagata-san's eb/iseof.cc
  int ret;
  char buffer[1000];
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
          printf("EAGAIN %d\n", socket); fflush(stdout);
          /* not EOF, no data in queue */
          return 0;
        } else {
          printf("ERROR %d errno %d err %s\n", socket , errno, strerror(errno)); fflush(stdout);
          close(socket);
          return -1;
        }
        break;
      default:
        printf("Flushing data in socket buffer (%d bytes) : sockid = %d\n", ret, socket); fflush(stdout);
    }
  }
}

void SerializerModule::callCheckRunPause(string& err_str)
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(Ser) TIMEOUT during send() ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  if (checkRunPause()) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Stop is detected after return from send ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    err_str = "RUN_PAUSE";
    g_run_pause = 1;
    throw (err_str);
  }
  return;
}

#endif


void SerializerModule::event()
{

#ifdef NONSTOP
  if (g_run_pause == 1) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Go back to Deseializer()  ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    return; // Nothing to do here
  } else if (g_run_resuming == 1) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Run resuming...()  ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    resumeRun();
    return;
  }
#endif

  if (m_start_flag == 0) {
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }

#ifdef TIME_MONITOR
  recordTime(n_basf2evt, time_array0);
#endif

  //  StoreArray<RawCOPPER> rawcprarray;
  StoreArray<RawDataBlock> raw_dblkarray;


  for (int j = 0; j < raw_dblkarray.getEntries(); j++) {
    //
    // Send data
    //
    if (m_start_flag == 0) {
      B2INFO("SerializerPC: Sending the 1st packet...");
    }

    try {
      m_totbytes += sendByWriteV(raw_dblkarray[ j ]);
      //    } catch (string err_str) {
    } catch (string err_str) {

#ifdef NONSTOP
      if (err_str == "RUN_PAUSE" || err_str == "RUN_ERROR") {
        return; // Go to DeSerializer***() to wait for run-resume.
      }
#endif
      print_err.PrintError((char*)(err_str.c_str()), __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(1);
    }
    if (m_start_flag == 0) {
      B2INFO("Done. ");
      m_start_flag = 1;
    }
  }


  //
  // Print current status
  //
  if (n_basf2evt % 1000 == 0) {
    //     double cur_time = getTimeSec();
    //     double total_time = cur_time - m_start_time;
    //     double interval = cur_time - m_prev_time;
    //     if (n_basf2evt != 0) {
    //       double multieve = (1. / interval);
    //       if (multieve > 2.) multieve = 2.;
    //     }
    //     time_t timer;
    //     struct tm* t_st;
    //     time(&timer);
    //     t_st = localtime(&timer);
    //     printf( "[DEBUG] Event %d TotSent  %.1lf [MB] ElapsedTime %.1lf [s] RcvdRate %.2lf [MB/s] %s",
    //            n_basf2evt, m_totbytes / 1.e6, total_time, (m_totbytes - m_prev_totbytes) / interval / 1.e6, asctime(t_st));
    //     fflush(stderr);
    //     m_prev_time = cur_time;
    //     m_prev_totbytes = m_totbytes;
    //     m_prev_nevt = n_basf2evt;
  }
  n_basf2evt++;
  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.isAvailable()) {
    status.setOutputNBytes(m_totbytes);
    status.addOutputCount(raw_dblkarray.getEntries());
  }

}
