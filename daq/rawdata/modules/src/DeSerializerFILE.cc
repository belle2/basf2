//+
// File : DeSerializerPC.cc
// Description : Module to read COPPER FIFO and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerFILE.h>
using namespace std;
using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerFILE)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerFILEModule::DeSerializerFILEModule() : DeSerializerCOPPERModule()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  //  addParam("FinesseBitFlag", finesse_bit_flag, "finnese (A,B,C,D) -> bit (0,1,2,3)", 15);

  addParam("inputFileName", m_fname_in, "Input binary filename", string(""));

  //Parameter definition
  B2INFO("DeSerializerFILE: Constructor done.");

}


DeSerializerFILEModule::~DeSerializerFILEModule()
{
}

void DeSerializerFILEModule::initialize()
{
  B2INFO("DeSerializerFILE: initialize() started.");
  m_fp_in = fopen(m_fname_in.c_str(), "r");
  if (!m_fp_in) {
    char    err_buf[500];
    sprintf(err_buf, "Cannot open an input file: %s : Exiting...\n",
            m_fname_in.c_str());
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));
  memset(time_array3, 0, sizeof(time_array3));
  memset(time_array4, 0, sizeof(time_array4));
  memset(time_array5, 0, sizeof(time_array5));

  // Initialize Array of RawCOPPER
  StoreArray<RawDataBlock>::registerPersistent();
  StoreArray<RawCOPPER>::registerPersistent();
  StoreArray<RawSVD>::registerPersistent();
  StoreArray<RawCDC>::registerPersistent();
  StoreArray<RawBPID>::registerPersistent();
  StoreArray<RawEPID>::registerPersistent();
  StoreArray<RawECL>::registerPersistent();
  StoreArray<RawKLM>::registerPersistent();


  B2INFO("DeSerializerFILE: initialize() done.");

}


int* DeSerializerFILEModule::ReadOneDataBlock(int* malloc_flag, int* size_word, int* data_type)
{
  *malloc_flag = 1;
  //
  // Read 1st word( data size ) of RawDataBlock
  //
  int n = 0;
  //  int recvd_byte = RawHeader::RAWHEADER_NWORDS * sizeof(int);
  int temp_size_word = 0;
  int recvd_byte = 0;
  int bytes_to_read = sizeof(int);

  int read_size = 0;
  //  if( read_size = fread( (char*)(&temp_size_word), bytes_to_read, 1, m_fp_in ) < 0 ) {
  if ((read_size = fread((char*)(&temp_size_word), 1, bytes_to_read, m_fp_in)) != bytes_to_read) {
    if (feof(m_fp_in)) {
      return 0x0;
    }
    char err_buf[100] = "Failed to read header"; print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
  recvd_byte += read_size;

  int start_word = 0, stop_word = 0;
  int* temp_buf = 0x0;
  if (temp_size_word == 0x7fff0008) {
    // 0x7fff0008 -> 2.1Gword. So this is too large for the length of one RawDataBlock.
    *data_type = COPPER_DATABLOCK;
    int pos_data_length = RawCOPPER::POS_DATA_LENGTH;
    start_word = 0;
    stop_word = pos_data_length;
    int* length_buf = ReadfromFILE(m_fp_in, pos_data_length, start_word, stop_word);

    *size_word = length_buf[ pos_data_length - 1 ] + RawCOPPER::COPPER_HEADER_TRAILER_NWORDS
                 + RawHeader::RAWHEADER_NWORDS + RawTrailer::RAWTRAILER_NWORDS;
    start_word = 1 + pos_data_length + RawHeader::RAWHEADER_NWORDS;
    stop_word = *size_word - RawTrailer::RAWTRAILER_NWORDS;
    temp_buf = ReadfromFILE(m_fp_in, *size_word, start_word, stop_word);

    temp_buf[ RawHeader::RAWHEADER_NWORDS ] = 0x7fff0008;
    memcpy((temp_buf + RawHeader::RAWHEADER_NWORDS + 1),
           length_buf, sizeof(int) * pos_data_length);
    delete length_buf;
  } else {
    *data_type = RAW_DATABLOCK;
    *size_word = temp_size_word;
    start_word = 1;
    stop_word = *size_word;
    temp_buf = ReadfromFILE(m_fp_in, *size_word, start_word, stop_word);
  }
  return temp_buf;
}



int* DeSerializerFILEModule::ReadfromFILE(FILE* fp_in, const int size_word, const int start_word, const int stop_word)
{
  //
  // Allocate buffer if needed
  //
  int* temp_buf;
  temp_buf = new int[ size_word ];

  memset(temp_buf, 0, size_word * sizeof(int));
  temp_buf[ 0 ] = size_word;
  int recvd_byte = start_word * sizeof(int);
  int bytes_to_read = stop_word * sizeof(int);
  int read_size;

  if ((read_size = fread((char*)temp_buf + recvd_byte, 1, bytes_to_read - recvd_byte, fp_in)) < 0) {
    if (feof(fp_in)) {
      delete temp_buf;
      return 0x0;
    }
    char err_buf[100] = "Failed to read header"; print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
  recvd_byte += read_size;

  if (bytes_to_read != recvd_byte) {
    char    err_buf[500];
    sprintf(err_buf, "Read less bytes(%d) than expected(%d). Exiting...\n",
            recvd_byte, bytes_to_read);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
  return temp_buf;
}





void DeSerializerFILEModule::event()
{


  printf("n %d\n", n_basf2evt);
  if (n_basf2evt < 0) {
    B2INFO("DeSerializerFILE: event() started.");
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

  //  rawcprarray.create();
  //  raw_dblkarray.create();

  //  RawCOPPER* temp_rawcopper;

  int first_flag = 0;
  unsigned int prev_eve_num = 0;
  int eof_flag = 0;

  //
  // Update EventMetaData
  //

  while (true) {
    int size_word = 0;
    int malloc_flag = 0;
    //    int* temp_buf = ReadOneEventFromCOPPERFIFO(j, &malloc_flag, &size_word);
    int* temp_buf;

    if (m_prev_buf_flag == 1) {
      temp_buf = m_prev_buf;
      size_word = temp_buf[ 0 ];
      m_prev_buf_flag = 0;
    } else {
      int data_type;
      temp_buf = ReadOneDataBlock(&malloc_flag, &size_word, &data_type);
      if (temp_buf == 0x0) { // End of File
        printf("End of file\n");
        eof_flag = 1;
        break;
      }
      if (data_type == COPPER_DATABLOCK) {
        RawCOPPER temp_rawcopper;
        int num_nodes = 1;
        int num_events = 1;
        temp_rawcopper.SetBuffer(temp_buf, size_word, 0, num_events, num_nodes);
        temp_rawcopper.GetB2LFEE32bitEventNumber(0);
        FillNewRawCOPPERHeader(&temp_rawcopper);
      }
    }

    //
    // Check Event #
    //
    RawDataBlock temp_rawdblk;
    int num_nodes = 1;
    int num_events = 1;

    temp_rawdblk.SetBuffer(temp_buf, size_word, 0, num_nodes, num_events);
    int block_num = 0;

    int eve_num;
    int subsysid = 0;

    if (temp_rawdblk.CheckFTSWID(block_num)) {
      RawFTSW temp_raw_ftsw;
      temp_raw_ftsw.SetBuffer(temp_buf, size_word, 0, num_nodes, num_events);
      eve_num = temp_raw_ftsw.GetEveNo(block_num);
    } else {
      RawCOPPER temp_raw_copper;
      temp_raw_copper.SetBuffer(temp_buf, size_word, 0, num_nodes, num_events);
      eve_num = temp_raw_copper.GetEveNo(block_num);
      subsysid = temp_raw_copper.GetSubsysId(block_num);
    }

    if (eve_num != prev_eve_num && first_flag != 0) {
      m_prev_buf_flag = 1;
      m_prev_buf = temp_buf;
      break;
    }
    prev_eve_num = eve_num;

    if (temp_rawdblk.CheckFTSWID(block_num)) {
      StoreArray<RawFTSW> ary;
      (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, num_nodes, num_events);
    } else {

      //
      // Switch to each detector and register RawXXX
      //
      //       if (subsysid == CDC_ID) {
      //  StoreArray<RawCDC> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else if (subsysid == SVD_ID) {
      //  StoreArray<RawSVD> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else if (subsysid == ECL_ID) {
      //  StoreArray<RawECL> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else if (subsysid == BPID_ID) {
      //  StoreArray<RawBPID> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else if (subsysid == EPID_ID) {
      //  StoreArray<RawEPID> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else if (subsysid == KLM_ID) {
      //  StoreArray<RawKLM> ary;
      //  (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //       } else {
      StoreArray<RawCOPPER> ary;
      (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
      //      }


    }

    m_totbytes += size_word * sizeof(int);

    first_flag = 1;
  }


  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(0);
  m_eventMetaDataPtr->setRun(0);
  m_eventMetaDataPtr->setEvent(prev_eve_num);
  if (eof_flag == 1) {
    m_eventMetaDataPtr->setEndOfData();
  }

  if (n_basf2evt % 100 == 0) {
    printf("Processing Evt # %d...\n", prev_eve_num);
  }

  n_basf2evt++;
  return;
}

