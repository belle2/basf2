//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerCOPPER.h>
#include <rawdata/CRCCalculator.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>



#define CHECKEVT 10000
//#define CHECK_SUM
//#define DUMMY
//#define MAXEVTSIZE 400000000
//#define TIME_MONITOR
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK
//#define YAMADA_DUMMY



using namespace std;
using namespace Belle2;

#ifdef NONSTOP
int g_run_recovery = 0;
int g_run_restarting = 0;
int g_run_stop = 0;
#endif

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerCOPPER)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerCOPPERModule::DeSerializerCOPPERModule() : DeSerializerModule()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  addParam("FinesseBitFlag", m_finesse_bit_flag, "finesse (A,B,C,D) -> bit (0,1,2,3)", 15);

  //Parameter definition
  B2INFO("DeSerializerCOPPER: Constructor done.");
  m_prev_ftsweve32 = 0xFFFFFFFF;

  m_cpr_fd = -1;
}


DeSerializerCOPPERModule::~DeSerializerCOPPERModule()
{
}

void DeSerializerCOPPERModule::initialize()
{
  B2INFO("DeSerializerCOPPER: initialize() started.");
  // allocate buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

  //
  // Initialize basf2 related
  //
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  // Initialize Array of RawCOPPER
  //  rawcprarray.registerPersistent();
  raw_dblkarray.registerPersistent();

  if (m_dump_fname.size() > 0) {
    openOutputFile();
  }
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));
  memset(time_array3, 0, sizeof(time_array3));
  memset(time_array4, 0, sizeof(time_array4));
  memset(time_array5, 0, sizeof(time_array5));

  // initialize COPPER
  initializeCOPPER();

  //
  // report ready to SLC
  //
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      printf("nodename = %s\n", m_nodename.c_str());
      g_status.open(m_nodename, m_nodeid);
      g_status.reportReady();
    }
  }

#ifdef NONSTOP
  openRunStopNshm();
#ifdef NONSTOP_DEBUG
  printf("###########(DesCpr) prepare shm  ###############\n");
  fflush(stdout);
#endif
#endif

  B2INFO("DeSerializerCOPPER: initialize() done.");
}


void DeSerializerCOPPERModule::initializeCOPPER()
{

#ifndef DUMMY
  m_use_slot = 0; /* bit mask */
  int slot_shift;

  if ((m_finesse_bit_flag & 0x1) == 1) {
    slot_shift = 0; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 1) & 0x1) == 1) {
    slot_shift = 1; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 2) & 0x1) == 1) {
    slot_shift = 2; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 3) & 0x1) == 1) {
    slot_shift = 3; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }
  //
  // Present slots to use
  //
  if (! m_use_slot) {
    char err_buf[100] = "Slot is not specified. Exiting...";
    print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  } else {
    int slot;
    printf("[DEBUG] ");
    for (slot = 0; slot < 4; slot++) {
      if (m_use_slot & (1 << slot)) printf(" %c", 'A' + slot);
    }
    printf("\n");
  }


#endif

#ifndef YAMADA_DUMMY
  B2INFO("Opening COPPER...");      fflush(stderr);
  openCOPPER();
  B2INFO("Done.\n");    fflush(stderr);
#endif



}


int* DeSerializerCOPPERModule::readOneEventFromCOPPERFIFO(const int entry, int* delete_flag, int* m_size_word)
{

  // prepare buffer
  *m_size_word = 0;
  int* temp_buf = m_bufary[ entry ];
  temp_buf[0] =  BUF_SIZE_WORD ;
  *delete_flag = 0;

#ifndef DUMMY
  //
  // Read data from HSLB
  //

  int recvd_byte = (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int);


  while (1) {
    int read_size = 0;
    if ((read_size = read(m_cpr_fd, (char*)m_bufary[entry] + recvd_byte, sizeof(int) *  BUF_SIZE_WORD  - recvd_byte)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {

        if (recvd_byte > (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int)) {
          char err_buf[500];
          sprintf("EAGAIN return in the middle of an event( COPPER driver should't do this.). Exting...", strerror(errno));
          print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(-1);
        }
#ifdef NONSTOP
#ifdef NONSTOP_DEBUG
        printf("\033[34m");
        printf("###########(DesCpr) Return from read with EAGAIN  ###############\n");
        fflush(stdout);
        printf("\033[0m");
#endif
        if (checkRunStop()) {
#ifdef NONSTOP_DEBUG
          printf("\033[31m");
          printf("###########(DesCpr) Stop is detected after return with EAGAIN  ###############\n");
          printf("\033[0m");
          fflush(stdout);
#endif
          string err_str = "EAGAIN";
          throw (err_str);
        }
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf("Failed to read data from COPPER(%s). Exiting...", strerror(errno));
        print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      recvd_byte += read_size;
      if (recvd_byte - (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int) > (int)(sizeof(int) * (m_pre_rawcpr.POS_DATA_LENGTH + 1)))break;
    }
  }

  //
  // Calcurate data size
  //
  *m_size_word = m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH + (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) ]
                 + m_pre_rawcpr.SIZE_COPPER_DRIVER_HEADER + m_pre_rawcpr.SIZE_COPPER_DRIVER_TRAILER
                 + m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS + m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.

  //
  // Allocate buffer if needed
  //
  if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) > recvd_byte) {
    // Check buffer size
    if (*m_size_word >  BUF_SIZE_WORD) {
      *delete_flag = 1;
      temp_buf = new int[ *m_size_word ];
      memcpy(temp_buf, m_bufary[ entry ], recvd_byte);
      recvd_byte += readFD(m_cpr_fd, (char*)temp_buf + recvd_byte,
                           (*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
    } else {

      recvd_byte += readFD(m_cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte,
                           (*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
    }

    if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) != recvd_byte) {
      char    err_buf[500];

      sprintf(err_buf, "CORRUPTED DATA: Read less bytes(%d) than expected(%d:%d). Exiting...\n",
              recvd_byte,
              *m_size_word * sizeof(int) - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS * sizeof(int),
              m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH ]);
      print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
  } else if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte) {
    char    err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Read more than data size. Exiting...: %d %d %d %d %d\n",
            recvd_byte, *m_size_word * sizeof(int) , m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS * sizeof(int),
            m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH ],  m_pre_rawcpr.POS_DATA_LENGTH);
    print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
#else
  //
  // Make dummy data
  //
  *m_size_word = 256 + entry;
  m_bufary[entry][0] = *m_size_word;
#endif

  //
  // Fill Data length
  //
  temp_buf[ 0 ] = *m_size_word;


#ifdef TIME_MONITOR
  if (n_basf2evt >= 50000 && n_basf2evt < 50500) {
    cur_time = getTimeSec();
    time_array2[ n_basf2evt - 50000 ] = cur_time - m_start_time;
  }
#endif

#ifdef CHECK_SUM
  unsigned int checksum = 0;
  for (int i = 0; i < m_bufary[entry][0]; i++) {
    if (i != 2) checksum += m_bufary[entry][i];
  }
  m_bufary[entry][2] = checksum;
#endif



  return temp_buf;

}





void DeSerializerCOPPERModule::openCOPPER()
{

  if (m_cpr_fd != -1) {
    close(m_cpr_fd);
    m_cpr_fd = -1;
  }
  //
  // Open a finesse device
  //
  if ((m_cpr_fd = open("/dev/copper/copper", O_RDONLY)) == -1) {
    char err_buf[500];
    sprintf("Failed to open Finesse(%s). Exiting... ", strerror(errno));
    print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int set_regval = 15; // How many events to be stored in COPPER FIFO before request for DMA
  //    int set_regval=1;
  ioctl(m_cpr_fd, CPRIOSET_LEF_WA_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WB_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WC_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WD_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_FINESSE_STA, &m_use_slot, sizeof(m_use_slot));

  int v = 511 - 32;

  ioctl(m_cpr_fd, CPRIOSET_LEF_WA_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WB_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WC_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WD_AF, &v, sizeof(v));


  B2INFO("DeSerializerCOPPER: openCOPPER() done.");

}



int DeSerializerCOPPERModule::readFD(int fd, char* buf, int data_size_byte)
{

  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = read(fd, (char*)buf + n, data_size_byte - n)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        if (n > 0) {
          char err_buf[500];
          sprintf("EAGAIN return in the middle of an event( COPPER driver should't do this.). Exting...", strerror(errno));
          print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(-1);
        }
#ifdef NONSTOP
#ifdef NONSTOP_DEBUG
        printf("###########(DesCpr) Return from readFD with EAGAIN  ###############\n");
        fflush(stdout);
#endif
        if (checkRunStop()) {
#ifdef NONSTOP_DEBUG
          printf("###########(DesCpr) Stop is detected after return from readFD with EAGAIN ##\n");
          fflush(stdout);
#endif
          string err_str = "EAGAIN";
          throw (err_str);
        }
#endif
        continue;
      } else {
        char err_buf[500];
        sprintf("Failed to read data from COPPER(%s). Exting...", strerror(errno));
        print_err.PrintError(m_shmflag, &g_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      n += read_size;
      if (n == data_size_byte)break;
    }
  }
  return n;

}

#ifdef NONSTOP
void DeSerializerCOPPERModule::restartRun()
{

#ifdef NONSTOP_DEBUG
  printf("\033[31m");
  printf("###########(DesCpr) Restart from PAUSE  ###############\n");
  printf("\033[0m");
  fflush(stdout);
#endif
  initializeCOPPER();
  g_run_recovery = 0;
  g_run_restarting = 1;
  m_start_flag = 0;
  return;
}

void DeSerializerCOPPERModule::waitRestart()
{
  // close COPPER FIFO
  if (m_cpr_fd != -1) close(m_cpr_fd);
  m_cpr_fd = -1;
  while (true) {
    if (checkRunRecovery()) {
      g_run_stop = 0;
      g_run_recovery = 1;
      break;
    }
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(DesCpr) Waiting for RESTART  ###############\n");
    printf("\033[0m");
    fflush(stdout);
#endif
    sleep(1);
  }
  return;
}

#endif

void DeSerializerCOPPERModule::event()
{



#ifdef NONSTOP
  if (g_run_recovery == 1) {
    restartRun();
  }
#endif

  if (m_start_flag == 0) {
    // Use shared memory to start(for HSLB dummy data)
#ifdef YAMADA_DUMMY
    B2INFO("Opening COPPER...");      fflush(stderr);
    openCOPPER();
    B2INFO("Done.\n");    fflush(stderr);
#endif
    if (g_status.isAvailable()) {
      B2INFO("DeSerializerCOPPER: Waiting for Start...\n");
      g_status.reportRunning();
    }
    //
    // for DESY test
    //
    //    m_nodeid = SVD_ID | m_nodeid  ;
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }

  //  rawcprarray.create();
  raw_dblkarray.create();
  RawDataBlock* temp_rawdblk;
  //  RawCOPPER* temp_rawcopper;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_COPPER; j++) {
    int m_size_word = 0;
    int delete_flag = 0;
    if (m_start_flag == 0) {
      B2INFO("DeSerializerCOPPER: Reading the 1st event from COPPER FIFO...");
    }


    int* temp_buf;
    try {
      temp_buf = readOneEventFromCOPPERFIFO(j, &delete_flag, &m_size_word);
    } catch (string err_str) {
#ifdef NONSTOP
      if (err_str == "EAGAIN") {
#ifdef NONSTOP_DEBUG
        printf("###########(DesCpr) caught Exception ###############\n");
        fflush(stdout);
#endif
        pauseRun();
        break;
      }
#endif
      print_err.PrintError(m_shmflag, &g_status, err_str);
      exit(1);
    }

    if (m_start_flag == 0) {
      B2INFO("DeSerializerCOPPER: Done. the size of the 1st event is " << m_size_word << "words");
      m_start_flag = 1;
    }

    const int num_nodes = 1;
    const int num_events = 1;
    temp_rawdblk =  raw_dblkarray.appendNew();
    temp_rawdblk->SetBuffer(temp_buf, m_size_word, delete_flag, num_events, num_nodes);
    // Fill Header and Trailer

    PreRawCOPPERFormat_latest temp_rawcopper;
    temp_rawcopper.SetBuffer(temp_buf, m_size_word, 0, num_events, num_nodes);

    // Fill header and trailer
    try {
      m_prev_ftsweve32 = temp_rawcopper.FillTopBlockRawHeader(m_nodeid, m_data_type, m_trunc_mask,
                                                              m_prev_ftsweve32, m_prev_runsubrun_no, &m_runsubrun_no);
      m_prev_runsubrun_no = m_runsubrun_no;
      //    fillNewRawCOPPERHeader( &temp_rawcopper );
    } catch (string err_str) {
      print_err.PrintError(m_shmflag, &g_status, err_str);
      exit(1);
    }

    if (m_dump_fname.size() > 0) {
      dumpData((char*)temp_buf, m_size_word * sizeof(int));
    }
    m_totbytes += m_size_word * sizeof(int);
  }

#ifdef NONSTOP
  if (g_run_stop == 1 || checkRunStop()) {
    waitRestart();
  }
#endif




  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1234);
  m_eventMetaDataPtr->setRun(105);
  m_eventMetaDataPtr->setEvent(n_basf2evt);


  //
  // Monitor
  //
  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
        || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n",
             max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  //
  // Print current status
  //
  //  if (n_basf2evt % 100 == 0 || n_basf2evt < 10) {
  if (n_basf2evt % 100 == 0) {
    RateMonitor(m_prev_ftsweve32);
  }
  if (g_status.isAvailable()) {
    g_status.setInputNBytes(m_totbytes);
    g_status.setInputCount(m_prev_ftsweve32);
  }

  n_basf2evt++;
  return;
}

