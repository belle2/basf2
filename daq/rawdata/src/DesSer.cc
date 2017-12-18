//+
// File : DeSerializerPC.cc
// Description : Module to receive data from eb0 and send it to eb1tx
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/DesSer.h>

//#define DEBUG
using namespace std;
using namespace Belle2;

//----------------------------------------------------------------
//                 Implementation
//----------------------------------------------------------------

DesSer::DesSer()
{

  m_num_connections = 1;

  m_exprunsubrun_no = 0; // will obtain info from data

  m_prev_exprunsubrun_no = 0xFFFFFFFF;

#ifdef NONSTOP
  m_run_pause = 0;

  m_run_error = 0;
#endif

  //  B2INFO("DeSerializerPrePC: Constructor done.");
  printf("[DEBUG] DesSer: Constructor done.\n"); fflush(stdout);
}



DesSer::~DesSer()
{

}


int* DesSer::getPreAllocBuf()
{
  int* tempbuf = 0;
  if (m_num_usedbuf < NUM_PREALLOC_BUF) {
    tempbuf = m_bufary[ m_num_usedbuf  ];
    m_num_usedbuf++;
  } else {
    char err_buf[500];
    sprintf(err_buf,
            "No pre-allocated buffers are left. %d > %d. Not enough buffers are allocated or "
            "memory leak or forget to call ClearNumUsedBuf every event loop. Exting...",
            m_num_usedbuf, NUM_PREALLOC_BUF);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }
  return tempbuf;
}


int* DesSer::getNewBuffer(int nwords, int* delete_flag)
{

  int* temp_buf = NULL;
  // Prepare buffer
  if (nwords >  BUF_SIZE_WORD) {
    *delete_flag = 1;
    temp_buf = new int[ nwords ];
  } else {
    if ((temp_buf = getPreAllocBuf()) == 0x0) {
      char err_buf[500];
      sprintf(err_buf, "Null pointer from GetPreALlocBuf(). Exting...\n");
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    } else {
      *delete_flag = 0;
    }
  }

  return temp_buf;

}

void DesSer::initialize(bool close_listen)
{
  printf("[DEBUG] DesSer: initialize() started.\n"); fflush(stdout);
  signal(SIGPIPE , SIG_IGN);

  //
  // initialize Rx part from DeSerializer**.cc
  //

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

  m_prev_copper_ctr = 0xFFFFFFFF;
  m_prev_evenum = 0xFFFFFFFF;


  //
  // initialize Rx part from DeSerializer**.cc
  //
  m_start_flag = 0;
  n_basf2evt = -1;
  m_compressionLevel = 0;

#ifdef DUMMY
  m_buffer = new int[ BUF_SIZE_WORD ];
#endif
  Accept(close_listen);
#ifdef NONSTOP
  openRunPauseNshm();
#endif


  // For monitor
  if (m_status.isAvailable()) {
    m_status.setOutputNBytes(0);
    m_status.setOutputCount(0);
  }
  //  B2INFO("DesSer: initialize() was done.");
  printf("[DEBUG] DesSer: initialize() was done.\n"); fflush(stdout);

}


/////////////////////////////////////////////////////
//   From Serializer.cc
/////////////////////////////////////////////////////

//void DesSer::fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlock* rawdblk)
void DesSer::fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlockFormat* rawdblk)
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
      sprintf(err_buf, "Length error. total length %d rawdblk length %d. Exting...\n" ,
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


//int DesSer::sendByWriteV(RawDataBlock* rawdblk)
int DesSer::sendByWriteV(RawDataBlockFormat* rawdblk)
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
    if ((n = writev(m_socket_send, iov, NUM_BUFFER)) < 0) {
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
        sprintf(err_buf, "[WARNING] WRITEV error.(%s) : sent %d bytes, header %d bytes body %d trailer %d\n" ,
                strerror(errno), n, iov[0].iov_len, iov[1].iov_len, iov[2].iov_len);
#ifdef NONSTOP
        m_run_error = 1;
        //        B2ERROR(err_buf);
        printf("%s\n", err_buf); fflush(stdout);
        string err_str = "RUN_ERROR";
        throw (err_str);  // To exit this module, go to DeSerializer** and wait for run-resume.
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
    //    B2WARNING("Serializer: Sent byte(" << n << "bytes) is not same as the event size (" << total_send_bytes << "bytes). Retryring...");
    printf("[NOTICE] Serializer: Sent byte( %d bytes) is not same as the event size ( %d bytes). Retryring...\n", n, total_send_bytes);
    fflush(stdout);

    double retry_start = getTimeSec();
    // Send Header
    if (n < (int)(iov[ 0 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 0 ].iov_base + n, iov[ 0 ].iov_len - n);
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 1 ].iov_base + (n - iov[ 0 ].iov_len), iov[ 1 ].iov_len - (n - iov[ 0 ].iov_len));
    }

    if (n < (int)(iov[ 0 ].iov_len + iov[ 1 ].iov_len + iov[ 2 ].iov_len)) {
      n += Send(m_socket_send, (char*)iov[ 2 ].iov_base + (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len),
                iov[ 2 ].iov_len - (n - iov[ 0 ].iov_len - iov[ 1 ].iov_len));
    }
    double retry_end = getTimeSec();
    //    B2WARNING("Resending ends. It takes " << retry_end - retry_start << "(s)");
    printf("[NOTICE] Resending ends. It takes %lf (s)\n", retry_end - retry_start); fflush(stdout);
  }
  //   printf( "[DEBUG] n %d total %d\n", n, total_send_bytes);
  //  delete temp_buf;

  return total_send_bytes;

}


int DesSer::Send(int socket, char* buf, int size_bytes)
{
  int sent_bytes = 0;
  while (true) {
    int ret = 0;
    if ((ret = send(socket,
                    buf + sent_bytes, size_bytes - sent_bytes,  MSG_NOSIGNAL)) < 0) {
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
        sprintf(err_buf, "[WARNING] SEND ERROR.(%s)", strerror(errno));
#ifdef NONSTOP
        m_run_error = 1;
        //        B2ERROR(err_buf);
        printf("%s\n", err_buf); fflush(stdout);
        string err_str = "RUN_ERROR";
        throw (err_str);  // To exit this module, go to DeSerializer** and wait for run-resume.
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

void DesSer::Accept(bool close_listen)
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
    printf("[FATAL] Failed to bind. Maybe other programs have already occupied this port(%d). Exiting...\n",
           m_port_to); fflush(stdout);
    // Check the process occupying the port 30000.
    FILE* fp;
    char buf[256];
    char cmdline[500];
    sprintf(cmdline, "/usr/sbin/ss -ap | grep %d", m_port_to);
    if ((fp = popen(cmdline, "r")) == NULL) {
      printf("[WARNING] Failed to run %s\n", cmdline);
    }
    while (fgets(buf, 256, fp) != NULL) {
      printf("[ERROR] Failed to bind. output of ss(port %d) : %s\n", m_port_to, buf); fflush(stdout);
    }
    // Error message
    fclose(fp);
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
    sprintf(err_buf, "Failed in listen(%s). Exting...", strerror(errno));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  //
  // Accept
  //
  int fd_accept;
  struct sockaddr_in sock_accept;
  printf("[DEBUG] Accepting... : port %d server %s\n", m_port_to, m_hostname_local.c_str());
  fflush(stdout);

  if ((fd_accept = accept(fd_listen, (struct sockaddr*) & (sock_accept), &addrlen)) == 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to accept(%s). Exiting...", strerror(errno));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  } else {
    //    B2INFO("Done.");
    printf("[DEBUG] Done.\n"); fflush(stdout);

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

  if (close_listen) {
    close(fd_listen);
  }

  //   int flag = 1;
  //   ret = setsockopt(fd_accept, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag) );
  m_socket_send = fd_accept;

  if (m_status.isAvailable()) {
    m_status.setOutputPort(ntohs(sock_listen.sin_port));
    m_status.setOutputAddress(sock_listen.sin_addr.s_addr);
    //    B2INFO("Accepted " << (int)ntohs(sock_listen.sin_port) << " " << (int)sock_listen.sin_addr.s_addr);
    printf("Accepted. port %d address %u\n", (int)ntohs(sock_listen.sin_port), (int)sock_listen.sin_addr.s_addr); fflush(stdout);
  }

  return;

}

double DesSer::getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void DesSer::recordTime(int event, double* array)
{
  if (event >= 50000 && event < 50500) {
    array[ event - 50000 ] = getTimeSec() - m_start_time;
  }
  return;
}


unsigned int DesSer::calcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


void DesSer::printData(int* buf, int nwords)
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
void DesSer::openRunPauseNshm()
{
  char path_shm[100] = "/cpr_pause_resume";
  int fd = shm_open(path_shm, O_RDONLY, 0666);
  if (fd < 0) {
    printf("[DEBUG] %s\n", path_shm);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  m_ptr = (int*)mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
  return;
}

int DesSer::checkRunPause()
{
#ifdef NONSTOP_SLC
  if (m_status.getState() == m_status.PAUSING) {
#else
  if (*m_ptr) {
#endif
    return 1;
  } else {
    return 0;
  }
}

int DesSer::checkRunRecovery()
{
#ifdef NONSTOP_SLC
  if (m_status.getState() == m_status.RESUMING) {
#else
  if (*m_ptr) {
#endif
    return 0;
  } else {
    return 1;
  }
}

void DesSer::resumeRun()
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(Ser) the 1st event sicne the resume  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  m_run_error = 0;
  m_run_pause = 0;
  return;
}


void DesSer::pauseRun()
{
  m_run_pause = 1;
#ifdef NONSTOP_DEBUG
  printf("###########(Ser) Pause the run ###############\n");
  fflush(stdout);
#endif
  return;
}




void DesSer::callCheckRunPause(string& err_str)
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(DesSer) TIMEOUT.  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  if (checkRunPause()) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(DesSer) Pause is detected. ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    m_run_pause = 1;
    throw (err_str);
  }
  return;
}


int DesSer::CheckConnection(int socket)
{
  // Modify Yamagata-san's eb/iseof.cc


  int ret;
  char buffer[100000];
  int eagain_cnt = 0;
  int tot_ret = 0;
  printf("CC1\n"); fflush(stdout);
  ret = send(socket, buffer, 0, MSG_DONTWAIT);
  printf("CC2\n"); fflush(stdout);
  switch (ret) {
    case 0:
      break;
    case -1:
      if (errno == EAGAIN) {
        printf("EAGAIN %d cnt %d recvd %d\n", socket, eagain_cnt, tot_ret); fflush(stdout);
        /* not EOF, no data in queue */
        if (eagain_cnt > 100) {
          return 0;
        }
        usleep(10000);
        eagain_cnt++;
      } else {
        printf("ERROR %d errno %d err %s\n", socket , errno, strerror(errno)); fflush(stdout);
        close(socket);
        return -1;
      }
    default:
      printf("Return value %d of send is strange. Exting...\n",  ret);
      fflush(stdout);
      exit(1);
  }

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
          printf("EAGAIN %d cnt %d recvd %d\n", socket, eagain_cnt, tot_ret); fflush(stdout);
          /* not EOF, no data in queue */
          if (eagain_cnt > 100) {
            return 0;
          }
          usleep(10000);
          eagain_cnt++;
        } else {
          printf("ERROR %d errno %d err %s\n", socket , errno, strerror(errno)); fflush(stdout);
          close(socket);
          return -1;
        }
        break;
      default:
        tot_ret += ret;
        printf("Flushing data in socket buffer : sockid = %d %d bytes tot %d bytes\n", socket, ret, tot_ret); fflush(stdout);
    }
  }
}


#endif

void DesSer::shmOpen(char*, char*)
//(char* path_cfg, char* path_sta)
{
  errno = 0;
  /*m_shmfd_cfg = shm_open( "/cpr_config2", O_CREAT | O_EXCL | O_RDWR, 0666);
  if (m_shmfd_cfg < 0) {
    if (errno != EEXIST) {
      perror("shm_open1");
      exit(1);
    }
  m_shmfd_cfg = shm_open(path_cfg, O_RDWR, 0666);
  if (m_shmfd_cfg < 0) {
    printf( "[DEBUG] %s\n", path_cfg);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  */
  //}
  /*
  m_shmfd_sta = shm_open( "/cpr_status2", O_CREAT | O_EXCL | O_RDWR, 0666);
  if (m_shmfd_sta < 0) {
    if (errno != EEXIST) {
      perror("shm_open1");
      exit(1);
    }
  m_shmfd_sta = shm_open(path_sta , O_RDWR, 0666);
  if (m_shmfd_sta < 0) {
    printf( "[DEBUG] %s\n", path_sta);
    perror("[ERROR] shm_open2");
    exit(1);
  }
  //}
  int size = 4 * sizeof(int);
  ftruncate(m_shmfd_cfg, size);
  ftruncate(m_shmfd_sta, size);
    */
}



