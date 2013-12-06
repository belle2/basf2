//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerCOPPER.h>

#define CHECKEVT 10000
//#define CHECK_SUM
//#define DUMMY
//#define MAXEVTSIZE 400000000
//#define TIME_MONITOR
//#define NO_DATA_CHECK
#define WO_FIRST_EVENUM_CHECK


using namespace std;
using namespace Belle2;



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
  addParam("FinesseBitFlag", finesse_bit_flag, "finnese (A,B,C,D) -> bit (0,1,2,3)", 15);

  //Parameter definition
  B2INFO("DeSerializerCOPPER: Constructor done.");
  m_prev_ftsweve32 = 0xFFFFFFFF;

}


DeSerializerCOPPERModule::~DeSerializerCOPPERModule()
{
}

void DeSerializerCOPPERModule::initialize()
{
  B2INFO("DeSerializerCOPPER: initialize() started.");
#ifndef DUMMY
  use_slot = 0; /* bit mask */
  int slot_shift;

  if ((finesse_bit_flag & 0x1) == 1) {
    slot_shift = 0; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finesse_bit_flag >> 1) & 0x1) == 1) {
    slot_shift = 1; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finesse_bit_flag >> 2) & 0x1) == 1) {
    slot_shift = 2; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finesse_bit_flag >> 3) & 0x1) == 1) {
    slot_shift = 3; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }
  //
  // Present slots to use
  //
  if (! use_slot) {
    char err_buf[100] = "Slot is not specified. Exiting...";
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  } else {
    int slot;
    for (slot = 0; slot < 4; slot++) {
      if (use_slot & (1 << slot)) printf(" %c", 'A' + slot);
    }
    printf("\n");
  }


#endif

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

  if (dump_fname.size() > 0) {
    OpenOutputFile();
  }

  if (m_shmflag != 0) {
    ShmOpen("/cpr_config", "/cpr_status");
    // Status format : status_flag
    m_cfg_buf = ShmGet(m_shmfd_cfg, 4);
    m_cfg_sta = ShmGet(m_shmfd_sta, 4);
    m_cfg_sta[ 0 ] = 0; // Status bit is 0 : not ready
  }

  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));
  memset(time_array3, 0, sizeof(time_array3));
  memset(time_array4, 0, sizeof(time_array4));
  memset(time_array5, 0, sizeof(time_array5));


  B2INFO("DeSerializerCOPPER: initialize() done.");

}




void DeSerializerCOPPERModule::FillNewRawCOPPERHeader(RawCOPPER* raw_copper)
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
  rawhdr.SetFTSW2Words(raw_copper->GetB2LFEETtCtime(cprblock), raw_copper->GetB2LFEETtUtime(cprblock));

#ifdef debug
  printf("1: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  for (int j = 0; j < raw_copper->TotalBufNwords(); j++) {
    printf("0x%.8x ", (raw_copper->GetBuffer(0))[ j ]);
    if ((j % 10) == 9)printf("\n");
    fflush(stdout);
  }
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
  printf("2: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  for (int j = 0; j < raw_copper->TotalBufNwords(); j++) {
    printf("0x%.8x ", (raw_copper->GetBuffer(0))[ j ]);
    if ((j % 10) == 9)printf("\n");
    fflush(stdout);
  }
#endif

  //
  // Fill info in Trailer
  //
  RawTrailer rawtrl;
  rawtrl.SetBuffer(raw_copper->GetRawTrlBufPtr(cprblock));
  rawtrl.Initialize(); // Fill 2nd word : magic word
  rawtrl.SetChksum(CalcSimpleChecksum(raw_copper->GetBuffer(cprblock),
                                      raw_copper->GetBlockNwords(cprblock) - rawtrl.GetTrlNwords()));

  //magic word check
#ifndef NO_DATA_CHECK
// 3, magic word check
  if (!(raw_copper->CheckCOPPERMagic(cprblock))) {
    char err_buf[500];
    sprintf(err_buf, "Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u\n",
            raw_copper->GetMagicDriverHeader(cprblock),
            raw_copper->GetMagicFPGAHeader(cprblock),
            raw_copper->GetMagicFPGATrailer(cprblock),
            raw_copper->GetMagicDriverTrailer(cprblock));
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }

  // 3, event # increment check
#ifdef WO_FIRST_EVENUM_CHECK
  if ((m_prev_ftsweve32 + 1 != cur_ftsw_eve32) && (m_prev_ftsweve32 != 0xFFFFFFFF)) {
#else
  if (m_prev_ftsweve32 + 1 != cur_ftsw_eve32) {
#endif
    char err_buf[500];
    sprintf(err_buf, "Invalid event_number. Exiting...: cur 32bit eve %u preveve %u\n",  cur_ftsw_eve32, m_prev_ftsweve32);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    printf("i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
    for (int j = 0; j < raw_copper->TotalBufNwords(); j++) {
      printf("0x%.8x ", (raw_copper->GetBuffer(0))[ j ]);
      if ((j % 10) == 9)printf("\n");
      fflush(stdout);
    }

    exit(-1);
  }
#endif
  m_prev_ftsweve32 = cur_ftsw_eve32;
  // Check magic words are set at proper positions

#ifdef debug
  printf("3: i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
  for (int j = 0; j < raw_copper->TotalBufNwords(); j++) {
    printf("0x%.8x ", (raw_copper->GetBuffer(0))[ j ]);
    if ((j % 10) == 9)printf("\n");
    fflush(stdout);
  }
#endif

  return;
}



int* DeSerializerCOPPERModule::ReadOneEventFromCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word)
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
  int n = 0;
  int recvd_byte = RawHeader::RAWHEADER_NWORDS * sizeof(int);
  while (1) {
    int read_size = 0;
    if ((read_size = read(cpr_fd, (char*)m_bufary[entry] + recvd_byte, sizeof(int) *  BUF_SIZE_WORD  - recvd_byte)) < 0) {
      if (read_size == EINTR) {
        continue;
      } else {
        char err_buf[100] = "Failed to read header"; print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      recvd_byte += read_size;
      if (recvd_byte - RawHeader::RAWHEADER_NWORDS * sizeof(int) > (int)(sizeof(int) * (RawCOPPER::POS_DATA_LENGTH + 1)))break;
    }
  }

  //
  // Calcurate data size
  //
  *m_size_word = m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH + RawHeader::RAWHEADER_NWORDS ]
                 + COPPER_HEADER_TRAILER_NWORDS + RawHeader::RAWHEADER_NWORDS + RawTrailer::RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.

  //
  // Allocate buffer if needed
  //
  if ((int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) > recvd_byte) {
    // Check buffer size
    if (*m_size_word >  BUF_SIZE_WORD) {
      *malloc_flag = 1;
      temp_buf = new int[ *m_size_word ];
      memcpy(temp_buf, m_bufary[ entry ], recvd_byte);
      recvd_byte += Read(cpr_fd, (char*)temp_buf + recvd_byte,
                         (*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
    } else {
      recvd_byte += Read(cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte,
                         (*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte);
    }

    if ((int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) != recvd_byte) {
      char    err_buf[500];
      sprintf(err_buf, "Read less bytes(%d) than expected(%d:%d). Exiting...\n",
              recvd_byte,
              *m_size_word * sizeof(int) - RawTrailer::RAWTRAILER_NWORDS * sizeof(int),
              m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ],  RawCOPPER::POS_DATA_LENGTH);
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }

  } else if ((int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte) {
    char    err_buf[500];
    sprintf(err_buf, "Read more than data size. Exiting...: %d %d %d %d %d\n", recvd_byte, *m_size_word * sizeof(int) , RawTrailer::RAWTRAILER_NWORDS * sizeof(int), m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ],  RawCOPPER::POS_DATA_LENGTH);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);

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
    cur_time = GetTimeSec();
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





void DeSerializerCOPPERModule::OpenCOPPER()
{
  //
  // Open a finesse device
  //
  if ((cpr_fd = open("/dev/copper/copper", O_RDONLY)) == -1) {
    char err_buf[100] = "Failed to open Finesse. Exiting... ";  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int set_regval = 4; // How many events to be stored in COPPER FIFO before request for DMA
  //    int set_regval=1;
  ioctl(cpr_fd, CPRIOSET_LEF_WA_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WB_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WC_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WD_FF, &set_regval);
  int ret = ioctl(cpr_fd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));

  int v = 511 - 32;

  ioctl(cpr_fd, CPRIOSET_LEF_WA_AF, &v, sizeof(v));
  ioctl(cpr_fd, CPRIOSET_LEF_WB_AF, &v, sizeof(v));
  ioctl(cpr_fd, CPRIOSET_LEF_WC_AF, &v, sizeof(v));
  ioctl(cpr_fd, CPRIOSET_LEF_WD_AF, &v, sizeof(v));


  B2INFO("DeSerializerCOPPER: OpenCOPPER() done.");

}



int DeSerializerCOPPERModule::Read(int fd, char* buf, int data_size_byte)
{

  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = read(fd, (char*)buf + n, data_size_byte - n)) < 0) {
      if (read_size == EINTR) {
        continue;
      } else {
        char err_buf[100] = "Failed to read header";   print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      n += read_size;
      if (n == data_size_byte)break;
    }
  }
  return n;

}



void DeSerializerCOPPERModule::event()
{


  if (n_basf2evt < 0) {

    B2INFO("DeSerializerCOPPER: event() started.");
    // Use shared memory to start(for HSLB dummy data)
    if (m_shmflag != 0) {
      //      int* cfg_buf = ShmGet(m_shmfd_cfg, 4);
      m_cfg_sta[ 0 ] = 1; // Status bit is 0 : not ready
      printf("Waiting for Start...\n");
      fflush(stdout);
      while (1) {
        if (m_cfg_buf[0] == 1)break;
        usleep(10000);
      }
    }

    OpenCOPPER();
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

  //  rawcprarray.create();
  raw_dblkarray.create();
  RawDataBlock* temp_rawdblk;
  //  RawCOPPER* temp_rawcopper;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_COPPER; j++) {
    int m_size_word = 0;
    int malloc_flag = 0;
    int* temp_buf = ReadOneEventFromCOPPERFIFO(j, &malloc_flag, &m_size_word);

    const int num_nodes = 1;
    const int num_events = 1;
    temp_rawdblk =  raw_dblkarray.appendNew();
    temp_rawdblk->SetBuffer(temp_buf, m_size_word, malloc_flag, num_events, num_nodes);
    // Fill Header and Trailer
    RawCOPPER temp_rawcopper;
    temp_rawcopper.SetBuffer(temp_buf, m_size_word, 0, num_events, num_nodes);

    // Fill header and trailer
    FillNewRawCOPPERHeader(&temp_rawcopper);

    if (dump_fname.size() > 0) {
      DumpData((char*)temp_buf, m_size_word * sizeof(int));
    }
    m_totbytes += m_size_word * sizeof(int);
  }


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
  if (n_basf2evt % 100 == 0) {
    double cur_time = GetTimeSec();
    double total_time = cur_time - m_start_time;
    double interval = cur_time - m_prev_time;
    if (n_basf2evt != 0) {
      double multieve = (1. / interval);
      if (multieve > 2.) multieve = 2.;
    }


    time_t timer;
    struct tm* t_st;
    time(&timer);
    t_st = localtime(&timer);
    printf("Event %d(%d) TotRecvd %.1lf [MB] ElapsedTime %.1lf [s] EvtRate %.2lf [kHz] RcvdRate %.2lf [MB/s] %s",
           n_basf2evt, m_prev_ftsweve32,  m_totbytes / 1.e6, total_time, (n_basf2evt - m_prev_nevt) / interval / 1.e3 * NUM_EVT_PER_BASF2LOOP_COPPER, (m_totbytes - m_prev_totbytes) / interval / 1.e6, asctime(t_st));
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_totbytes = m_totbytes;
    m_prev_nevt = n_basf2evt;
  }

  n_basf2evt++;
  return;
}

