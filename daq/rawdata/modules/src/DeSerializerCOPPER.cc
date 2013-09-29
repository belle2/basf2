//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerCOPPER.h>

#define CHECKEVT 1000


//#define CHECK_SUM
//#define DUMMY
//#define MAXEVTSIZE 400000000
//#define TIME_MONITOR





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
  addParam("FinnesseBitFlag", finnesse_bit_flag, "finnese (A,B,C,D) -> bit (0,1,2,3)", 15);

  //Parameter definition
  B2INFO("DeSerializerCOPPER: Constructor done.");

}


DeSerializerCOPPERModule::~DeSerializerCOPPERModule()
{
}

void DeSerializerCOPPERModule::initialize()
{

#ifndef DUMMY
  use_slot = 0; /* bit mask */
  int slot_shift;

  if ((finnesse_bit_flag & 0x1) == 1) {
    slot_shift = 0; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finnesse_bit_flag >> 1) & 0x1) == 1) {
    slot_shift = 1; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finnesse_bit_flag >> 2) & 0x1) == 1) {
    slot_shift = 2; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }

  if (((finnesse_bit_flag >> 3) & 0x1) == 1) {
    slot_shift = 3; // a:0, b:1, c:2, d:3
    use_slot |= 1 << slot_shift;  //
  }
  //
  // Present slots to use
  //
  if (! use_slot) {
    perror("Slot is not specified. Exiting...");
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

  rawcprarray.registerPersistent();


  if (dump_fname.size() > 0) {
    OpenOutputFile();
  }

  if (m_shmflag != 0) {
    ShmOpen("/cpr_config", "/cpr_status");
    m_cfg_buf = ShmGet(m_shmfd_cfg, 4);
    m_cfg_sta = ShmGet(m_shmfd_sta, 4);
  }


  B2INFO("Rx initialized.");
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));
  memset(time_array3, 0, sizeof(time_array3));
  memset(time_array4, 0, sizeof(time_array4));
  memset(time_array5, 0, sizeof(time_array5));

}




void DeSerializerCOPPERModule::FillNewRawCOPPERHeader(RawCOPPER* raw_copper)
{
  const int num_cprblock = 0; // On COPPER, 1 COPPER block will be stored in a RawCOPPER.
  RawHeader rawhdr;
  rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(num_cprblock));

  //
  // initialize header(header nwords, magic word) and trailer(magic word)
  //
  rawhdr.Initialize(); // Fill 2nd( hdr size) and 20th header word( magic word )

  // Set total words info
  int nwords = raw_copper->GetCprBlockNwords(num_cprblock);
  rawhdr.SetNwords(nwords);

  //
  // Obtain info from SlowController via AddParam
  //
  rawhdr.SetExpNo(m_exp_no);   // Fill 3rd header word
  rawhdr.SetRunNo(m_run_no);   // Fill 3rd header word

  // Obtain eve.# from COPPER header
  rawhdr.SetEveNo(raw_copper->GetCOPPEREveNo(num_cprblock));     // Fill 4th header word
  //  rawhdr.SetB2LFEEHdrPart(raw_copper->GetB2LFEEHdr1(), raw_copper->GetB2LFEEHdr2());   // Fill 5th and 6th words

  // Obtain info from SlowController via AddParam or COPPER data
  rawhdr.SetSubsysId(m_nodeid);   // Fill 7th header word
  rawhdr.SetDataType(m_data_type);   // Fill 8th header word
  rawhdr.SetTruncMask(m_trunc_mask);   // Fill 8th header word

  // Offset
  rawhdr.SetOffset1stFINNESSE(raw_copper->GetOffset1stFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));          // Fill 9th header word
  rawhdr.SetOffset2ndFINNESSE(raw_copper->GetOffset2ndFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 10th header word
  rawhdr.SetOffset3rdFINNESSE(raw_copper->GetOffset3rdFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 11th header word
  rawhdr.SetOffset4thFINNESSE(raw_copper->GetOffset4thFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 12th header word

  // Add node-info
  rawhdr.SetMagicWordEntireHeader();

  // Add node-info
  rawhdr.AddNodeInfo(m_nodeid);   // Fill 13th header word

  //
  // Fill info in Trailer
  //
  RawTrailer rawtrl;
  rawtrl.SetBuffer(raw_copper->GetRawTrlBufPtr(num_cprblock));
  rawtrl.Initialize(); // Fill 2nd word : magic word
  rawtrl.SetChksum(CalcSimpleChecksum(raw_copper->GetBuffer(num_cprblock),
                                      raw_copper->GetCprBlockNwords(num_cprblock) - rawtrl.GetTrlNwords()));

  // Check magic words are set at proper positions
}



int* DeSerializerCOPPERModule::ReadCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word)
{
  // Get header size
  const int COPPER_HEADER_TRAILER_NWORDS = 9;

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
      perror("Failed to read header");
      exit(-1);
    } else {
      recvd_byte += read_size;
      if (recvd_byte - RawHeader::RAWHEADER_NWORDS * sizeof(int) > (int)(sizeof(int) * (RawCOPPER::POS_DATA_LENGTH + 1)))break;
    }
  }

  *m_size_word = m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH + RawHeader::RAWHEADER_NWORDS ]
                 + COPPER_HEADER_TRAILER_NWORDS + RawHeader::RAWHEADER_NWORDS + RawTrailer::RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.

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
  } else if ((int)((*m_size_word - RawTrailer::RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte) {
    printf("Read more than data size. Exiting...: %d %d %d %d %d\n", recvd_byte, *m_size_word * sizeof(int) , RawTrailer::RAWTRAILER_NWORDS * sizeof(int), m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ],  RawCOPPER::POS_DATA_LENGTH);
    exit(-1);
  }

#ifdef DEBUG
  for (int k = 0; k < 13; k++) {
    printf("%4d %.8x\n", k, m_bufary[ entry ][k ]);
  }
  printf("\n");
  fflush(stdout);
#endif
  m_totbytes +=  recvd_byte - RawHeader::RAWHEADER_NWORDS * sizeof(int);


#else
  //
  // Make dummy data
  //
  *m_size_word = 256 + entry;
  m_bufary[entry][0] = *m_size_word;
#endif


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


void* DeSerializerCOPPERModule::OpenCOPPER()
{
  //
  // Open a finnesse device
  //
  if ((cpr_fd = open("/dev/copper/copper", O_RDONLY)) == -1) {
    perror("Failed to open Finnese. Exiting... ");
    exit(1);
  }

  int set_regval = 100; // How many events to be stored in COPPER FIFO before request for DMA
  //    int set_regval=1;
  ioctl(cpr_fd, CPRIOSET_LEF_WA_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WB_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WC_FF, &set_regval);
  ioctl(cpr_fd, CPRIOSET_LEF_WD_FF, &set_regval);
  int ret = ioctl(cpr_fd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));
  printf("Reset slot %d ret %d\n", use_slot, ret);
  //    m_start_time = GetTimeSec();
}



int DeSerializerCOPPERModule::Read(int fd, char* buf, int data_size_byte)
{

  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = read(fd, (char*)buf + n, data_size_byte - n)) < 0) {
      perror("Failed to read header");
      exit(-1);
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

    // Use shared memory to start(for HSLB dummy data)
    if (m_shmflag != 0) {
      //      int* cfg_buf = ShmGet(m_shmfd_cfg, 4);
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

  rawcprarray.create();
  RawCOPPER* temp_rawcopper;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    int m_size_word = 0;
    int malloc_flag = 0;
    int* temp_buf = ReadCOPPERFIFO(j, &malloc_flag, &m_size_word);

    //
    // Fill RawCOPPER
    //
    temp_rawcopper =  rawcprarray.appendNew();
    // Store data buffer
    temp_rawcopper->SetBuffer(temp_buf, m_size_word, malloc_flag, 1, 1);
    // Fill header and trailer
    FillNewRawCOPPERHeader(temp_rawcopper);

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
  if (n_basf2evt % (CHECKEVT / NUM_EVT_PER_BASF2LOOP) == 0) {
    double cur_time = GetTimeSec();
    double total_time = cur_time - m_start_time;
    double interval = cur_time - m_prev_time;

    time_t timer;
    struct tm* t_st;
    time(&timer);
    t_st = localtime(&timer);
    printf("Event %d TotRecvd %.1lf [MB] ElapsedTime %.1lf [s] EvtRate %.2lf [kHz] RcvdRate %.2lf [MB/s] %s",
           n_basf2evt, m_totbytes / 1.e6, total_time, (n_basf2evt - m_prev_nevt) / interval / 1.e3 * NUM_EVT_PER_BASF2LOOP, (m_totbytes - m_prev_totbytes) / interval / 1.e6, asctime(t_st));
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_totbytes = m_totbytes;
    m_prev_nevt = n_basf2evt;
  }

  n_basf2evt++;
  return;
}

