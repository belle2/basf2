//+
// File : DeSerializerCOPPER.cc
// Description : Module to receive data from COPPER FIFO
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 1 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerCOPPER.h>

#define CHECKEVT 50000


//#define CHECK_SUM
//#define DUMMY
//#define MAXEVTSIZE 400000000
//#define TIME_MONITOR

#define CLONE_ARRAY  // w/o this, this program does not work.

//#define DISCARD_DATA

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
  slot_shift = 0; // a:0, b:1, c:2, d:3
  use_slot |= 1 << slot_shift;  //

  slot_shift = 1; // a:0, b:1, c:2, d:3
  use_slot |= 1 << slot_shift;  //

  slot_shift = 2; // a:0, b:1, c:2, d:3
  use_slot |= 1 << slot_shift;  //

  slot_shift = 3; // a:0, b:1, c:2, d:3
  use_slot |= 1 << slot_shift;  //
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
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];

  //
  // Initialize basf2 related
  //
  for (int i = 0 ; i < NUM_EVT_PER_BASF2LOOP; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  // Initialize Array of RawCOPPER
#ifdef CLONE_ARRAY
  rawcprarray.registerPersistent();
#else
  m_rawcopper.registerPersistent();
#endif

  if (dump_fname.size() > 0) {
    OpenOutputFile();
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

  RawHeader* raw_header = &(raw_copper->m_header);
  RawTrailer* raw_trailer = &(raw_copper->m_trailer);


  //
  // initialize header and trailer
  //
  raw_header->initialize();
  raw_trailer->initialize();

  //
  // Add node-info
  //
  int eve_no = raw_copper->get_coppereve_no() ; //tentative
  raw_header->add_nodeinfo_and_update_hdrsize(raw_copper->get_copper_node_id(), eve_no);

  //
  // Obtain info from Rawdata
  //
  raw_header->set_eve_no(raw_copper->get_coppereve_no());
  raw_header->set_subsys_id(raw_copper->get_subsys_id());
  raw_header->set_num_b2l_block(raw_copper->get_num_b2l_block());
  raw_header->set_offset_1st_b2l(raw_copper->offset_1st_b2l_wo_rawhdr() + raw_header->get_hdr_nwords());
  raw_header->set_offset_2nd_b2l(raw_copper->offset_2nd_b2l_wo_rawhdr() + raw_header->get_hdr_nwords());
  raw_header->set_offset_3rd_b2l(raw_copper->offset_3rd_b2l_wo_rawhdr() + raw_header->get_hdr_nwords());
  raw_header->set_offset_4th_b2l(raw_copper->offset_4th_b2l_wo_rawhdr() + raw_header->get_hdr_nwords());

  //
  // Obtain info from SlowController
  //
  int dummy_exp_no = 0, dummy_run_no = 0;
  raw_header->set_exp_no(dummy_exp_no);
  raw_header->set_run_no(dummy_run_no);

  //
  // Needs definition or not used
  //
  int dummy_data_type = 0;
  raw_header->set_data_type(dummy_data_type);
  //  raw_header->set_trunc_mask( int trunc_mask );


  //
  // Set total words info
  //
  int nwords =
    raw_header->get_hdr_nwords()
    + raw_trailer->get_trl_nwords()
    + raw_copper->get_body_nwords();
  raw_header->set_nwords(nwords);


  // Check magic words are set at proper positions



}


void DeSerializerCOPPERModule::event()
{

#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array0);
#endif

  if (n_basf2evt < 0) {
    OpenCOPPER();
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

#ifdef CLONE_ARRAY
  rawcprarray.create();
#else
  m_rawcopper.create();
#endif


#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array1);
#endif


  RawCOPPER* temp_rawcopper;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    int m_size_word = 0;
    int malloc_flag = 0;
    int* temp_buf = ReadCOPPERFIFO(j, &malloc_flag, &m_size_word);

    //
    // Fill RawCOPPER
    //
#ifndef DISCARD_DATA

#ifdef CLONE_ARRAY
    temp_rawcopper =  rawcprarray.appendNew();
    // Store data buffer
    temp_rawcopper->buffer(temp_buf, m_size_word, malloc_flag);
    // Fill header and trailer
    FillNewRawCOPPERHeader(temp_rawcopper);
#else
    m_rawcopper->buffer(temp_buf, m_size_word, malloc_flag);
    //FillNewRawCOPPERHeader( m_rawcopper );
#endif

    if (dump_fname.size() > 0) {
      DumpData((char*)temp_buf, m_size_word * sizeof(int));
    }


#endif // DISCARD_DATA

#ifdef TIME_MONITOR
    RecordTime(n_basf2evt, time_array2);
#endif
    m_totbytes += m_size_word * sizeof(int);
  }


  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1234);
  m_eventMetaDataPtr->setRun(105);
  m_eventMetaDataPtr->setEvent(n_basf2evt);


#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array3);

  if (n_basf2evt == 51000) {
    double diff0 = 0., diff1 = 0., diff2 = 0., diff3 = 0., diff4 = 0., diff5 = 0.;
    for (int i = 0; i < 500; i++) {
      printf("Read %d %lf %lf %lf %lf %lf\n", i + 50000, time_array0[ i ], time_array1[ i ], time_array2[ i ], time_array3[ i ],     time_array3[i] - time_array2[i]);
      if (i > 1) {
        diff0 += time_array0[ i ] - time_array4[ i - 1 ];
      }
      if (i > 0) {
        diff1 += time_array1[ i ] - time_array0[ i ];
        diff2 += time_array2[ i ] - time_array1[ i ];
        diff3 += time_array3[ i ] - time_array2[ i ];
        diff4 += time_array4[ i ] - time_array3[ i ];
        diff5 += time_array5[ i ] - time_array4[ i ];
      }
    }
    printf("DeSerializerCOPPER : %.2e %.2e %.2e %.2e %.2e %.2e\n",
           diff0 / 498., diff1 / 499., diff2 / 499., diff3 / 499., diff4 / 499., diff5 / 499.);
  }
#endif


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

    printf("Event %d TotRecvd %.1lf [MB] ElapsedTime %.2lf [s] EvtRate %.4lf [kHz] RcvdRate %.4lf [MB/s] %s",
           n_basf2evt, m_totbytes / 1.e6, total_time, (n_basf2evt - m_prev_nevt) / interval / 1.e3 * NUM_EVT_PER_BASF2LOOP, (m_totbytes - m_prev_totbytes) / interval / 1.e6, asctime(t_st));
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_totbytes = m_totbytes;
    m_prev_nevt = n_basf2evt;
  }

  n_basf2evt++;
  return;
}



int* DeSerializerCOPPERModule::ReadCOPPERFIFO(const int entry, int* malloc_flag, int* m_size_word)
{

  *m_size_word = 0;
  int* temp_buf = m_bufary[ entry ];
  temp_buf[0] =  BUF_SIZE_WORD ;
  *malloc_flag = 0;

#ifndef DUMMY
  //
  // Read data from HSLB
  //

  int n = 0;
  int recvd_byte = 0;
  while (1) {
    int read_size = 0;
    if ((read_size = read(cpr_fd, (char*)m_bufary[entry] + recvd_byte, sizeof(int) *  BUF_SIZE_WORD  - recvd_byte)) < 0) {
      perror("Failed to read header");
      exit(-1);
    } else {
      recvd_byte += read_size;
      if (recvd_byte > (int)(sizeof(int) * (RawCOPPER::POS_DATA_LENGTH + 1)))break;
    }
  }

  *m_size_word = m_bufary[ entry ][ RawCOPPER::POS_DATA_LENGTH ] + 9; // 9 words are COPPER haeder and trailer size.
  if ((int)((*m_size_word)*sizeof(int)) < recvd_byte) {
    // Check buffer size
    if (*m_size_word >  BUF_SIZE_WORD) {
      *malloc_flag = 1;
      temp_buf = new int[ *m_size_word ];
      memcpy(temp_buf, m_bufary[ entry ], recvd_byte);
      recvd_byte += Read(cpr_fd, (char*)temp_buf + recvd_byte, *m_size_word * sizeof(int) - recvd_byte);
    } else {
      recvd_byte += Read(cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte, *m_size_word * sizeof(int) - recvd_byte);
    }
  } else if ((int)((*m_size_word)*sizeof(int)) > recvd_byte) {
    perror("Read more than data size. Exiting...");
    exit(-1);
  }

#ifdef DEBUG
  for (int k = 0; k < 13; k++) {
    printf("%4d %.8x\n", k, m_bufary[ entry ][k ]);
  }
  printf("\n");
  fflush(stdout);
#endif
  m_totbytes +=  recvd_byte;


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


