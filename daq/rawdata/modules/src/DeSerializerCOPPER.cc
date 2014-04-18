//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerCOPPER.h>
#include <rawdata/CRCCalculator.h>

#define CHECKEVT 10000
//#define CHECK_SUM
//#define DUMMY
//#define MAXEVTSIZE 400000000
//#define TIME_MONITOR
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK
#define YAMADA_DUMMY



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
      m_status.open(m_nodename, m_nodeid);
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
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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

#ifndef REDUCED_RAWCOPPER
void DeSerializerCOPPERModule::fillNewRawCOPPERHeader(RawCOPPER* raw_copper)
{

  const int cprblock = 0; // On COPPER, 1 COPPER block will be stored in a RawCOPPER.

  // initialize header(header nwords, magic word) and trailer(magic word)
  RawHeader rawhdr;
  rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(cprblock));
  rawhdr.Initialize(); // Fill 2nd( hdr size) and 20th header word( magic word )

  // 1, Set total words info
  int nwords = raw_copper->GetBlockNwords(cprblock);
  rawhdr.SetNwords(nwords);



  // 2, Set run and exp #
  rawhdr.SetExpRunNumber(raw_copper->GetExpRunBuf(cprblock));       // Fill 3rd header word

  // 3, Make 32bit event number from B2link FEE header
  unsigned int cur_ftsw_eve32 =  raw_copper->GetB2LFEE32bitEventNumber(cprblock);


#ifdef DUMMY_EVENT_NUM
  cur_ftsw_eve32 = m_prev_ftsweve32 + 1;
#endif
  rawhdr.SetEveNo(cur_ftsw_eve32);       // Temporarily use COPPER counter   //raw_copper->GetCOPPERCounter()

  // Set FTSW word
  rawhdr.SetFTSW2Words(raw_copper->GetTTCtimeTRGType(cprblock), raw_copper->GetTTUtime(cprblock));

#ifdef debug
  printf("[DEBUG] 1: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  printData(raw_copper->GetBuffer(0), raw_copper->TotalBufNwords());
#endif

  // Obtain info from SlowController via AddParam or COPPER data
  rawhdr.SetSubsysId(m_nodeid);   // Fill 7th header word
  rawhdr.SetDataType(m_data_type);   // Fill 8th header word
  rawhdr.SetTruncMask(m_trunc_mask);   // Fill 8th header word

  // Offset
  rawhdr.SetOffset1stFINESSE(raw_copper->GetOffset1stFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));          // Fill 9th header word
  rawhdr.SetOffset2ndFINESSE(raw_copper->GetOffset2ndFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 10th header word
  rawhdr.SetOffset3rdFINESSE(raw_copper->GetOffset3rdFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 11th header word
  rawhdr.SetOffset4thFINESSE(raw_copper->GetOffset4thFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 12th header word

  // Set magic word
  rawhdr.SetMagicWordEntireHeader();

  // Add node-info
  rawhdr.AddNodeInfo(m_nodeid);   // Fill 13th header word

#ifdef debug
  printf("[DEBUG] 2: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  printData(raw_copper->GetBuffer(0), raw_copper->TotalBufNwords());
#endif

  //
  // Fill info in Trailer
  //
  RawTrailer rawtrl;
  rawtrl.SetBuffer(raw_copper->GetRawTrlBufPtr(cprblock));
  rawtrl.Initialize(); // Fill 2nd word : magic word
  rawtrl.SetChksum(calcXORChecksum(raw_copper->GetBuffer(cprblock),
                                   raw_copper->GetBlockNwords(cprblock) - rawtrl.GetTrlNwords()));

  //magic word check
#ifndef NO_DATA_CHECK
// 3, magic word check
  if (!(raw_copper->CheckCOPPERMagic(cprblock))) {
    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Invalid Magic word 0x7FFFF0008=%x 0xFFFFFAFA=%x 0xFFFFF5F5=%x 0x7FFF0009=%x\n",
            raw_copper->GetMagicDriverHeader(cprblock),
            raw_copper->GetMagicFPGAHeader(cprblock),
            raw_copper->GetMagicFPGATrailer(cprblock),
            raw_copper->GetMagicDriverTrailer(cprblock));
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }

  // 3, event # increment check
#ifdef WO_FIRST_EVENUM_CHECK
  if ((m_prev_ftsweve32 + 1 != cur_ftsw_eve32) && (m_prev_ftsweve32 != 0xFFFFFFFF)) {
#else
  if (m_prev_ftsweve32 + 1 != cur_ftsw_eve32) {
#endif // WO_FIRST_EVENUM_CHECK

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Invalid event_number. Exiting...: cur 32bit eve %x preveve %x\n",  cur_ftsw_eve32, m_prev_ftsweve32);
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    printf("[DEBUG] i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
    printData(raw_copper->GetBuffer(0), raw_copper->TotalBufNwords());
    exit(-1);

  }
#endif // NO_DATA_CHECK

  m_prev_ftsweve32 = cur_ftsw_eve32;
  // Check magic words are set at proper positions

#ifdef debug
  printf("[DEBUG] 3: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  printData(raw_copper->GetBuffer(0), raw_copper->TotalBufNwords());
#endif

  return;
}

#endif // REDUCED_RAWCOPPER


int* DeSerializerCOPPERModule::readOneEventFromCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word)
{

  // prepare buffer
  *m_size_word = 0;
  int* temp_buf = m_bufary[ entry ];
  temp_buf[0] =  BUF_SIZE_WORD ;
  *malloc_flag = 0;

#ifndef DUMMY
  //
  // Read data from HSLB
  //
#ifndef REDUCED_RAWCOPPER
  int recvd_byte = RawHeader::RAWHEADER_NWORDS * sizeof(int);
#else
  int recvd_byte = ReducedRawHeader::RAWHEADER_NWORDS * sizeof(int);
#endif

  while (1) {
    int read_size = 0;
    if ((read_size = read(m_cpr_fd, (char*)m_bufary[entry] + recvd_byte, sizeof(int) *  BUF_SIZE_WORD  - recvd_byte)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {

        if (
#ifndef REDUCED_RAWCOPPER
          recvd_byte > RawHeader::RAWHEADER_NWORDS * sizeof(int)
#else
          recvd_byte > ReducedRawHeader::RAWHEADER_NWORDS * sizeof(int)
#endif
        ) {
          char err_buf[500];
          sprintf("EAGAIN return in the middle of an event( COPPER driver should't do this.). Exting...", strerror(errno));
          print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
        print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      recvd_byte += read_size;

#ifndef REDUCED_RAWCOPPER
      if (recvd_byte - RawHeader::RAWHEADER_NWORDS * sizeof(int) > (int)(sizeof(int) * (RawCOPPER::POS_DATA_LENGTH + 1)))break;
#else
      if (recvd_byte - ReducedRawHeader::RAWHEADER_NWORDS * sizeof(int) > (int)(sizeof(int) * (PreRawCOPPER::POS_DATA_LENGTH + 1)))break;
#endif

    }
  }

  //
  // Calcurate data size
  //
#ifndef REDUCED_RAWCOPPER
  *m_size_word = m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH + RawHeader::RAWHEADER_NWORDS ]
                 + RawCOPPER::SIZE_COPPER_DRIVER_HEADER + RawCOPPER::SIZE_COPPER_DRIVER_TRAILER
                 + RawHeader::RAWHEADER_NWORDS + RawTrailer::RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.
#else
  *m_size_word = m_bufary[ entry ][ PreRawCOPPER::POS_DATA_LENGTH + ReducedRawHeader::RAWHEADER_NWORDS ]
                 + PreRawCOPPER::SIZE_COPPER_DRIVER_HEADER + PreRawCOPPER::SIZE_COPPER_DRIVER_TRAILER
                 + ReducedRawHeader::RAWHEADER_NWORDS + ReducedRawTrailer::RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.
#endif

  //
  // Allocate buffer if needed
  //
  if (
#ifndef REDUCED_RAWCOPPER
    (int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) > recvd_byte
#else
    (int)((*m_size_word - ReducedRawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) > recvd_byte
#endif
  ) {
    // Check buffer size
    if (*m_size_word >  BUF_SIZE_WORD) {
      *malloc_flag = 1;
      temp_buf = new int[ *m_size_word ];


      memcpy(temp_buf, m_bufary[ entry ], recvd_byte);
#ifndef REDUCED_RAWCOPPER
      recvd_byte += readFD(m_cpr_fd, (char*)temp_buf + recvd_byte,
                           (*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
#else
      recvd_byte += readFD(m_cpr_fd, (char*)temp_buf + recvd_byte,
                           (*m_size_word - ReducedRawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
#endif
    } else {
#ifndef REDUCED_RAWCOPPER
      recvd_byte += readFD(m_cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte,
                           (*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
#else
      recvd_byte += readFD(m_cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte,
                           (*m_size_word - ReducedRawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
#endif
    }

    if (
#ifndef REDUCED_RAWCOPPER
      (int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) != recvd_byte
#else
      (int)((*m_size_word - ReducedRawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) != recvd_byte
#endif
    ) {
      char    err_buf[500];

#ifndef REDUCED_RAWCOPPER
      sprintf(err_buf, "CORRUPTED DATA: Read less bytes(%d) than expected(%d:%d). Exiting...\n",
              recvd_byte,
              *m_size_word * sizeof(int) - RawTrailer::RAWTRAILER_NWORDS * sizeof(int),
              m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ]);
#else
      sprintf(err_buf, "CORRUPTED DATA: Read less bytes(%d) than expected(%d:%d). Exiting...\n",
              recvd_byte,
              *m_size_word * sizeof(int) - ReducedRawTrailer::RAWTRAILER_NWORDS * sizeof(int),
              m_bufary[ entry ][ PreRawCOPPER::POS_DATA_LENGTH ]);
#endif
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
  } else if (
#ifndef REDUCED_RAWCOPPER
    (int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte
#else
    (int)((*m_size_word - ReducedRawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte
#endif
  ) {
    char    err_buf[500];
#ifndef REDUCED_RAWCOPPER
    sprintf(err_buf, "CORRUPTED DATA: Read more than data size. Exiting...: %d %d %d %d %d\n",
            recvd_byte, *m_size_word * sizeof(int) , RawTrailer::RAWTRAILER_NWORDS * sizeof(int),
            m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ],  RawCOPPER::POS_DATA_LENGTH);
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#else
    sprintf(err_buf, "CORRUPTED DATA: Read more than data size. Exiting...: %d %d %d %d %d\n",
            recvd_byte, *m_size_word * sizeof(int) , ReducedRawTrailer::RAWTRAILER_NWORDS * sizeof(int),
            m_bufary[ entry ][ PreRawCOPPER::POS_DATA_LENGTH ],  PreRawCOPPER::POS_DATA_LENGTH);
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif

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
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
          print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
        print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    if (m_shmflag > 0) {

      B2INFO("DeSerializerCOPPER: Waiting for Start...\n");
      m_status.reportRunning();
    }
    //
    // for DESY test
    //
    m_nodeid = SVD_ID | m_nodeid  ;
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }

  //  rawcprarray.create();
  raw_dblkarray.create();
  RawDataBlock* temp_rawdblk;
  //  RawCOPPER* temp_rawcopper;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_COPPER; j++) {
    int m_size_word = 0;
    int malloc_flag = 0;
    if (m_start_flag == 0) {
      B2INFO("DeSerializerCOPPER: Reading the 1st event from COPPER FIFO...");
    }


    int* temp_buf;
    try {
      temp_buf = readOneEventFromCOPPERFIFO(j, &malloc_flag, &m_size_word);
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
      print_err.PrintError(m_shmflag, &m_status, err_str);
      exit(1);
    }

    if (m_start_flag == 0) {
      B2INFO("DeSerializerCOPPER: Done. the size of the 1st event is " << m_size_word << "words");
      m_start_flag = 1;
    }

    const int num_nodes = 1;
    const int num_events = 1;
    temp_rawdblk =  raw_dblkarray.appendNew();
    temp_rawdblk->SetBuffer(temp_buf, m_size_word, malloc_flag, num_events, num_nodes);
    // Fill Header and Trailer

#ifndef REDUCED_RAWCOPPER
    RawCOPPER temp_rawcopper;
#else
    PreRawCOPPER temp_rawcopper;
#endif
    temp_rawcopper.SetBuffer(temp_buf, m_size_word, 0, num_events, num_nodes);



    // Fill header and trailer
    try {
      m_prev_ftsweve32 = temp_rawcopper.FillTopBlockRawHeader(m_nodeid, m_data_type, m_trunc_mask,
                                                              m_prev_ftsweve32, m_prev_runsubrun_no, &m_runsubrun_no);
      m_prev_runsubrun_no = m_runsubrun_no;
      //    fillNewRawCOPPERHeader( &temp_rawcopper );
    } catch (string err_str) {
      print_err.PrintError(m_shmflag, &m_status, err_str);
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
  // Print current status
  //
  //  if (n_basf2evt % 100 == 0 || n_basf2evt < 10) {
  if (n_basf2evt % 100 == 0) {
    RateMonitor(m_prev_ftsweve32);
  }

  n_basf2evt++;
  return;
}

