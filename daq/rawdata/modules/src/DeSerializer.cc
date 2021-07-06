/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DeSerializer.h>

#include <sys/mman.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RunInfoBuffer DeSerializerModule::g_status;

DeSerializerModule::DeSerializerModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  addParam("DumpFileName", m_dump_fname, "filename to record data", string(""));

  addParam("EventDataBufferWords", BUF_SIZE_WORD, "DataBuffer words per event", 4800);

  addParam("MaxEventNum", max_nevt, "Maximum event number in one run", -1);

  addParam("MaxTime", max_seconds, "Time(s) to stop, DAQ", -1.);

  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);

  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));

  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  m_nodeid = m_nodeid << 12; // input value is used as slog ID in subsystemID record

  n_basf2evt = -1;

  m_totbytes = 0;

  m_compressionLevel = 0;

  m_prev_time = 0.;

  monitor_numeve = 10;

  m_exp_no = 0; // will obtain info from parameter

  m_data_type = 0; // will obtain info from parameter

  m_trunc_mask = 0; // will obtain info from parameter

  m_prev_nevt = -1;

  prev_event = -1;

  m_run_no = 0; // will obtain info from data

  m_prev_run_no = -1;

  m_exprunsubrun_no = 0; // will obtain info from data

  m_prev_exprunsubrun_no = 0xFFFFFFFF;

  m_start_flag  = 0;


}


DeSerializerModule::~DeSerializerModule()
{
}


void DeSerializerModule::initialize()
{

  // allocate buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

}

void DeSerializerModule::terminate()
{
  delete[] m_buffer;
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP_COPPER; i++) {
    delete[] m_bufary[i];
  }

  if (m_dump_fname.size() > 0) {
    fclose(m_fp_dump);
  }

}


void DeSerializerModule::shmOpen(char*, char*)
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

int* DeSerializerModule::shmGet(int fd, int size_words)
{
  int offset = 0;
  return (int*)mmap(NULL, size_words * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
}




unsigned int  DeSerializerModule::calcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {

    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


unsigned int  DeSerializerModule::calcSimpleChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum + (unsigned int)buf[ i ];
    //    printf( "[DEBUG] i %.4d 0x%.8x 0x%.8x\n", i, checksum, buf[i]);
  }
  return checksum;
}



double DeSerializerModule::getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void DeSerializerModule::recordTime(int event, double* array)
{
  if (event >= 10000 && event < 10500) {
    array[ event - 10000 ] = getTimeSec() - m_start_time;
  }
  return;
}


void DeSerializerModule::openOutputFile()
{
  if ((m_fp_dump = fopen(m_dump_fname.c_str(), "wb")) == NULL) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to open file %s. Exiting...\n", m_dump_fname.c_str());
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
}

void DeSerializerModule::dumpData(char* buf, int size)
{
  if (fwrite(buf, size, 1, m_fp_dump) <= 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to write buffer to a file. Exiting...\n");
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
}

void DeSerializerModule::printData(int* buf, int nwords)
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

void DeSerializerModule::printASCIIData(int* buf, int nwords)
{
  char ascii_code[500];
  sprintf(ascii_code,
          "                                 !  #$ &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[ ]^_'abcdefghijklmnopqrstuvwxyz{|}~ ");
  printf("[DEBUG]");
  for (int i = 0; i < nwords; i++) {
    for (int j = 0 ; j < 4; j++) {
      printf("%c", ascii_code[(buf[ i ] >> j * 8) & 0x7f ]);
    }
    if (i % 10 == 9) printf("\n[DEBUG]");
  }
  printf("\n[DEBUG]");
  printf("\n");
  return;
}

int* DeSerializerModule::getNewBuffer(int nwords, int* delete_flag)
{

  int* temp_buf = NULL;
  // Prepare buffer
  //  printf( "[DEBUG] ############ %d %d %d %d\n", nwords, BUF_SIZE_WORD, m_num_usedbuf, NUM_PREALLOC_BUF );
  if (nwords >  BUF_SIZE_WORD) {
    *delete_flag = 1;
    temp_buf = new int[ nwords ];
  } else {
    if ((temp_buf = getPreAllocBuf()) == 0x0) {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] Null pointer from GetPreALlocBuf(). Exting...\n");
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(1234567);
      exit(1);
    } else {
      *delete_flag = 0;
    }
  }

  return temp_buf;

}

int* DeSerializerModule::getPreAllocBuf()
{
  int* tempbuf = 0;
  if (m_num_usedbuf < NUM_PREALLOC_BUF) {
    tempbuf = m_bufary[ m_num_usedbuf  ];
    m_num_usedbuf++;
  } else {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] No pre-allocated buffers are left. %d > %d. Not enough buffers are allocated or memory leak or forget to call ClearNumUsedBuf every event loop. Exting...",
            m_num_usedbuf, NUM_PREALLOC_BUF);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(1234567);
    exit(1);
  }
  return tempbuf;
}


void DeSerializerModule::RateMonitor(unsigned int nevt, int subrun, int run)
{
  double cur_time = getTimeSec();
  double total_time = cur_time - m_start_time;
  double interval = cur_time - m_prev_time;
  time_t timer;
  struct tm* t_st;
  time(&timer);
  t_st = localtime(&timer);
  printf("[INFO] run %d sub %d Event %12d Rate %6.2lf[kHz] Recvd Flow %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s] %s",
         run, subrun,
         nevt, (nevt  - m_prev_nevt) / interval / 1.e3,
         (m_totbytes - m_prev_totbytes) / interval / 1.e6,
         total_time,
         interval,
         asctime(t_st));

  fflush(stdout);
  m_prev_time = cur_time;
  m_prev_totbytes = m_totbytes;
  m_prev_nevt = nevt;

}

#ifdef NONSTOP
void DeSerializerModule::openRunPauseNshm()
{
  char path_shm[100] = "/cpr_pause_resume";
  int fd = shm_open(path_shm, O_RDONLY, 0666);
  if (fd < 0) {
    printf("[DEBUG] %s\n", path_shm);
    perror("[FATAL] failed to open shm");
    exit(1);
  }
  m_ptr = (int*)mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
  return;
}

int DeSerializerModule::checkRunPause()
{

  if (m_ptr == NULL) {
    B2INFO("Shared memory is not assigned.");
    return 0;
  }
#ifdef NONSTOP_SLC
  if (g_status.getState() == g_status.PAUSING) {
#else
  if (*m_ptr) {
#endif
    return 1;
  } else {
    return 0;
  }
}

int DeSerializerModule::checkRunRecovery()
{
  if (m_ptr == NULL) {
    B2INFO("Shared memory is not assigned.");
    return 0;
  }
#ifdef NONSTOP_SLC
  if (g_status.getState() == g_status.RESUMING) {
#else
  if (*m_ptr) {
#endif
    return 0;
  } else {
    return 1;
  }
}


void DeSerializerModule::resumeRun()
{

#ifdef NONSTOP_DEBUG
  printf("###########(Des) Resume from PAUSE  ############### %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
  fflush(stdout);
#endif
  g_run_error = 0;
  g_run_resuming = 1;
  m_start_flag = 0;

  return;
}


void DeSerializerModule::pauseRun()
{
  g_run_pause = 1;
#ifdef NONSTOP_DEBUG
  printf("###########(Des) Pause the run ###############\n");
  fflush(stdout);
#endif
  return;
}


void DeSerializerModule::waitResume()
{
  while (true) {
    if (checkRunRecovery()) {
      g_run_pause = 0;
      break;
    }
#ifdef NONSTOP_DEBUG
    printf("###########(Des) Waiting for RESUME  ###############\n");
    fflush(stdout);
#endif
    sleep(1);
  }
  return;
}



void DeSerializerModule::callCheckRunPause(string& err_str)
{
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(Des) TIMEOUT during recv()  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  if (checkRunPause()) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Des) Pause is detected during recv(). ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    g_run_pause = 1;
    err_str = "RUN_PAUSE";

    throw (err_str);
  }
  return;
}

int DeSerializerModule::CheckConnection(int socket)
{
  // Modify Yamagata-san's eb/iseof.cc


  int ret;
  char buffer[100000];
  int eagain_cnt = 0;
  int tot_ret = 0;
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

void DeSerializerModule::event()
{
}

