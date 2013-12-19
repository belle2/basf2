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

#define NO_DATA_CHECK
#define WO_FIRST_EVENUM_CHECK

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
  addParam("inputRepetitionTimes", m_repetition_max,
           "Input repetition times to read the input file", 0);

  //Parameter definition
  B2INFO("DeSerializerFILE: Constructor done.");

}


DeSerializerFILEModule::~DeSerializerFILEModule()
{
}

void DeSerializerFILEModule::FileOpen()
{
  m_fp_in = fopen(m_fname_in.c_str(), "r");
  if (!m_fp_in) {
    char    err_buf[500];
    sprintf(err_buf, "Cannot open an input file: %s : Exiting...\n",
            m_fname_in.c_str());
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
  return;
}

void DeSerializerFILEModule::initialize()
{
  B2INFO("DeSerializerFILE: initialize() started.");
  FileOpen();
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

  m_start_flag = 0;

  m_repetition_cnt = 0;
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
  while (true) {
    if ((read_size = fread((char*)(&temp_size_word), 1, bytes_to_read, m_fp_in)) != bytes_to_read) {
      if (feof(m_fp_in)) {
        if (m_repetition_max > m_repetition_cnt) {
          m_repetition_cnt++;
          fclose(m_fp_in);
          FileOpen();
          continue;
        } else {
          return 0x0;
        }
      }
      char err_buf[100] = "Failed to read header"; print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
    break;
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

    *size_word = length_buf[ pos_data_length - 1 ] +
                 RawCOPPER::SIZE_COPPER_DRIVER_HEADER + RawCOPPER::SIZE_COPPER_DRIVER_TRAILER
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



int* DeSerializerFILEModule::Modify131213SVDdata(int* buf_in, int buf_in_nwords, int* malloc_flag, unsigned int evenum)
{



  // prepare buffer
  int* buf_out = new int[ buf_in_nwords + 1 ];
  *malloc_flag = 1;
  memset(buf_out, 0, sizeof(int) * (buf_in_nwords + 1));


  // Copy other part of data
  int b2lhslb_header_nwords_131213 = 1;
  int b2lfee_header_nwords_131213 = 4;
  int offset_in = RawHeader::RAWHEADER_NWORDS + RawCOPPER::SIZE_COPPER_HEADER +
                  b2lhslb_header_nwords_131213 + b2lfee_header_nwords_131213;
  int offset_out = RawHeader::RAWHEADER_NWORDS + RawCOPPER::SIZE_COPPER_HEADER +
                   RawCOPPER::SIZE_B2LHSLB_HEADER + RawCOPPER::SIZE_B2LFEE_HEADER;


  memcpy(buf_out, buf_in, (offset_in - b2lfee_header_nwords_131213)* sizeof(int));
  memcpy(buf_out + offset_out , buf_in + offset_in,
         (buf_in_nwords - offset_in) * sizeof(int));
  int* b2lfee_buf_in = &(buf_in[ offset_in - b2lfee_header_nwords_131213 ]);
  int* b2lfee_buf_out = &(buf_out[ offset_out - RawCOPPER::SIZE_B2LFEE_HEADER ]);


  // Fill B2LFEE header part
  if (evenum == 0) {
    b2lfee_buf_out[ 0 ] = (b2lfee_buf_in[ 3 ] & 0x7ffffff0) | 0x0000000f;   // ctime & trgtype
  } else {
    b2lfee_buf_out[ 0 ] = b2lfee_buf_in[ 3 ] & 0x7ffffff0; // ctime & trgtype
  }
  b2lfee_buf_out[ 1 ] = evenum; // b2lfee_buf_in[ 0 ]; // event #
  b2lfee_buf_out[ 2 ] = b2lfee_buf_in[ 1 ]; // TT-utime
  b2lfee_buf_out[ 3 ] = b2lfee_buf_in[ 2 ]; // TT-exprun
  b2lfee_buf_out[ 4 ] = b2lfee_buf_in[ 3 ] & 0x7ffffff0; // B2Lctime & debugflag


  // Event # Modification in SVD data
  RawCOPPER temp_rawcopper;
  int num_nodes = 1;
  int num_events = 1;
  temp_rawcopper.SetBuffer(buf_out, buf_in_nwords + 1, 0, num_events, num_nodes);

  for (int i = 0 ; i < 4; i++) {
    if (temp_rawcopper.GetFINESSENwords(0, i) > 0) {
      int* temp_buf = temp_rawcopper.GetDetectorBuffer(0, i);
      if (temp_buf[ 0 ] != 0xffaa0000) {
        char    err_buf[500];
        sprintf(err_buf, "Invalid SVN header magic word %x : Exiting...\n",
                temp_buf[0]);
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
      temp_buf[ 1 ] = (((evenum) << 8) & 0xFFFFFF00) | (0x000000FF & temp_buf[ 1 ]);
    }
  }
  return buf_out;
}



void DeSerializerFILEModule::event()
{

  if (m_start_flag == 0) {
    m_start_flag = 1;
    m_dummy_evenum = 0;
  }

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

      //
      // To make a RawSVD dummy file from data sent by Nakamura-san on Dec. 13, 2013
      //
      {
        int* temp_temp_buf = Modify131213SVDdata(temp_buf, size_word, &malloc_flag, m_dummy_evenum);
        delete temp_buf;
        temp_buf = temp_temp_buf;
        m_dummy_evenum++;
      }

      if (data_type == COPPER_DATABLOCK) {
        RawCOPPER temp_rawcopper;
        int num_nodes = 1;
        int num_events = 1;
        temp_rawcopper.SetBuffer(temp_buf, size_word, 0, num_events, num_nodes);

//  for (int j = 0; j < temp_rawcopper.TotalBufNwords(); j++) {
//    printf("0x%.8x ", (temp_rawcopper.GetBuffer(0))[ j ]);
//    if ((j % 10) == 9)printf("\n");
//    fflush(stdout);
//  }
//        temp_rawcopper.GetB2LFEE32bitEventNumber(0);
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
      StoreArray<RawSVD> ary;
      (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
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
//       StoreArray<RawCOPPER> ary;
//       (ary.appendNew())->SetBuffer(temp_buf, size_word, 1, 1, 1);
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

