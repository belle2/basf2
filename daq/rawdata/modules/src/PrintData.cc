//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/PrintData.h>
#include <framework/core/InputController.h>

using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintDataModule::PrintDataModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  n_basf2evt = -1;
  m_compressionLevel = 0;
  B2INFO("PrintData: Constructor done.");
  m_ncpr = 0;
  m_nftsw = 0;

  m_time_cdc_1 = -1.;
  m_time_cdc_2 = -1.;
  m_time_ut3 = -1.;

  m_prev_time_cdc_1 = -1.;
  m_prev_time_cdc_2 = -1.;
  m_prev_time_ut3 = -1.;

  m_eve_cnt = 0;
  m_prev_eve_from_data = 0xFFFFFFFF;


}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::defineHisto()
{

  //   double t_min = 0.;
  //   double t_max = 600.;
  //   int t_nbin = 600;

  h_size = new TH1F("h_size", "Data size / COPPER; Data size [Byte]; entries", 50, 0, 10000);
  h_nhit = new TH1F("h_rate", "Data rate / COPPER; COPPER ID; Data rate [Bytes/s]", 48, 0, 48);
  h_chfadc = new TH2F("h_chfadc", "FADC value v.s. ch; CDC ch; FADC value", 48, 0., 48., 256, 0., 256.);
  h_chtdc = new TH2F("h_chfadc", "FADC value v.s. ch; CDC ch; FADC value", 48, 0., 48., 256, 0., 256.);




  /*
    h_hslb_size[0] = new TH1F("h_hslb_size_0", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
    h_hslb_size[1] = new TH1F("h_hslb_size_1", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
    h_hslb_size[2] = new TH1F("h_hslb_size_2", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
    h_hslb_size[3] = new TH1F("h_hslb_size_3", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
    h_hslb_nevt = new TH1F("h_hslb_nevt", "Number of Events / HSLB; HSLB slot; # of Events", 4, 0, 4);
    h_hslb_rate = new TH1F("h_hslb_rate", "Data rate / HSLB; HSLB slot; Data rate [Bytes/s]", 4, 0, 4);
  */
}


void PrintDataModule::initialize()
{

  B2INFO("PrintData: initialize() started.");

  RbTupleManager::Instance().register_module(this);
  m_eventMetaDataPtr.registerAsPersistent();
  defineHisto();
  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);
  REG_HISTOGRAM
  B2INFO("PrintData: initialize() done.");

}



//
// User defined functions
//


void PrintDataModule::printBuffer(int* buf, int nwords)
{
  //  printf("\n%.8d : ", 0);
  //  printf("%.8d : ", 0);
  for (int j = 0; j < nwords; j++) {
    printf("%.8x ", buf[ j ]);
    if ((j + 1) % 12 == 0) {
      //      printf("\n%.8d : ", j + 1);
      //      printf("\n %.8d :", j + 1);
      printf("\n");
      //      break;
    }
  }
  printf("\n");
  //  printf("\n");
  return;
}


void PrintDataModule::printFTSWEvent(RawDataBlock* raw_datablock, int i)
{
  int* buf  = raw_datablock->GetBuffer(i);
  int nwords =  raw_datablock->GetBlockNwords(i);
  m_eve_from_ftsw = buf[7] & 0xFFFF;

  if ((5 - 5 < m_eve_cnt && m_eve_cnt < 5 + 5)
      //  if( ( 173848 -5 < m_eve_cnt && m_eve_cnt < 173848 + 5)
     ) { // for run10038
    //   if( ( 579 -5 < m_eve_cnt && m_eve_cnt < 579 + 5)
    //       ){
    //     if( ( 25348 -5 < m_eve_cnt && m_eve_cnt < 25348 + 5) ||
    //  ( 1292201 -5 < m_eve_cnt && m_eve_cnt < 1292201 + 5) ||
    //  ( 384553 -5 < m_eve_cnt && m_eve_cnt < 384553 + 5) ||
    //  ( 387625 -5 < m_eve_cnt && m_eve_cnt < 387625 + 5) ||
    //  ( 1026080  -5 < m_eve_cnt && m_eve_cnt < 1026080 + 5)
    //  ){  // for run10048
    //      printf("FTSWEVE %d %x\n", m_eve_cnt, m_eve_cnt);
    printf("FTSW **** %x %x %x ******** **** 4\n",
           buf[7] & 0xFFFF, (buf[6] >> 4)  & 0x7FFFFFF,
           (buf[7] >> 16) & 0xFFFF
          );
    printBuffer(buf, nwords);

  }

  //  RawFTSW rawftsw;
  //  int malloc_flag = 0; // No need to free the buffer
  //  int num_event = 1;
  //  int num_nodes = 1;
  //  rawftsw.SetBuffer( buf, nwords, malloc_flag, num_event, num_nodes );

  //  int n = 0;
  //   printf("%d %d %.8x %.8x %lf\n",
  //   rawftsw.GetNwords(n),
  //   rawftsw.GetNwordsHeader(n),
  //   rawftsw.GetFTSWNodeID(n),
  //   rawftsw.GetTrailerMagic(n),
  //   rawftsw.GetEventUnixTime(n)
  //   );


  m_nftsw++;
  return;
}

void PrintDataModule::printArray(int* array, int array_size, double min, double max, const char* title)
{
  printf("\n# %d %lf %lf %s", array_size, min, max, title);
  for (int i = 0; i < array_size ; i++) {
    printf("\n");
    printf("%d", array[ i ]);
  }
}

void PrintDataModule::fillHisto1D(double value, int* array, int array_size, double min, double max)
{

  if (value > max) {
    array[ array_size - 1 ]++;
  } else if (value < min) {
    array[ 0 ]++;
  } else {
    int entry = (int)((value - min) / (max - min) * array_size);
    array[ entry ]++;
  }
  return;
}


void PrintDataModule::fillHisto2D(double value_x, double value_y, int* array,
                                  int array_size_x, double min_x, double max_x,
                                  int array_size_y, double min_y, double max_y
                                 )
{
//   int entry_x = 1;
//   if (value_x > max_x) {
//     entry_x = array_size_x - 1;
//   } else if (value_x < min_x) {
//     entry_x = 0;
//   } else {
//     entry_x = (int)((value_x - min_x) / (max_x - min_x) * array_size_x);
//   }

  int entry_y = 1;
  if (value_y > max_y) {
    entry_y = array_size_y - 1;
  } else if (value_y < min_y) {
    entry_y = 0;
  } else {
    entry_y = (int)((value_y - min_y) / (max_y - min_y) * array_size_y);
  }

  array[ entry_y * array_size_x + array_size_x ]++;

  return;
}


void PrintDataModule::endRun()
{
  //fill Run data

  printArray(m_size_cdc_1, SIZE_CDC_1_NUM, SIZE_CDC_1_MIN, SIZE_CDC_1_MAX,
             "Data_size/event_CDC_1;Data_size/event/FEE[byte];#_of_events"
            );
  printArray(m_size_cdc_2, SIZE_CDC_2_NUM, SIZE_CDC_2_MIN, SIZE_CDC_2_MAX,
             "Data_size/event_CDC_2;Data_size/event/FEE[byte];#_of_events"
            );
  printArray(m_size_ut3, SIZE_UT3_NUM, SIZE_UT3_MIN, SIZE_UT3_MAX,
             "Data_size/event_UT3;Data_size/event/FEE[byte];#_of_events"
            );

  printArray(m_tdiff_cdc_1, TDIFF_EVE_NUM, TDIFF_EVE_MIN, TDIFF_EVE_MAX,
             "Time_difference_from_the_previous_event;Time_difference[s];#_of_events"
            );
  printArray(m_tdiff_cdc_2, TDIFF_EVE_NUM, TDIFF_EVE_MIN, TDIFF_EVE_MAX,
             "Time_difference_from_the_previous_event;Time_difference[s];#_of_events"
            );
  printArray(m_tdiff_ut3, TDIFF_EVE_NUM, TDIFF_EVE_MIN, TDIFF_EVE_MAX ,
             "Time_difference_from_the_previous_event;Time_difference[s];#_of_events"
            );
  printArray(m_eve_diff, EVEDIFF_NUM, EVEDIFF_MIN, EVEDIFF_MAX,
             "Event_#_shift;Event_#_difference_from_the_correct_number;#_of_events"
            );

  printArray(m_eve_diff_prev, EVEDIFF_NUM, EVEDIFF_MIN, EVEDIFF_MAX,
             "Event_#_difference_from_prev_Event_#;Event_#_difference;#_of_events"
            );

  printArray(m_rate, TRGRATE_NUM, TRGRATE_MIN, TRGRATE_MAX,
             "Trigger_rate;Time_from_Runstart[s];#_of_events/s"
            );


}


void PrintDataModule::terminate()
{
  //fill Run data
}


void PrintDataModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{


  RawCDC raw_cdc;

  int* buf  = raw_copper->GetBuffer(i);

  int nwords =  raw_copper->GetBlockNwords(i);
  int malloc_flag = 0;
  int num_nodes = 1;
  int num_events = 1;

  raw_cdc.SetBuffer(buf, nwords, malloc_flag, num_events, num_nodes);

#ifdef DEBUG
  printf(": Event # %d \n", raw_copper->GetEveNo(i));
#endif

  //   printf(": Event # %d : node ID 0x%.8x : block size %d bytes : %d %d %d %d : %d %d %d %d\n",
  //          raw_copper->GetEveNo(i), raw_copper->GetCOPPERNodeId(i),
  //          raw_copper->GetBlockNwords(i) * sizeof(int),
  //          raw_copper->Get1stDetectorNwords(i),
  //          raw_copper->Get2ndDetectorNwords(i),
  //          raw_copper->Get3rdDetectorNwords(i),
  //          raw_copper->Get4thDetectorNwords(i),
  //          raw_cdc.RawDataType1st(i),
  //          raw_cdc.RawDataType2nd(i),
  //          raw_cdc.RawDataType3rd(i),
  //          raw_cdc.RawDataType4th(i)
  //   );

#ifdef DEBUG
  printf("******* Raw COPPER data block(including Detector Buffer)**********\n");
  PrintData(raw_copper->GetBuffer(i), raw_copper->GetBlockNwords(i));
#endif

  m_eve_from_data = raw_copper->GetEveNo(i);
  if (m_eve_cnt >= 0) {
    fillHisto1D((double)(m_eve_from_data - m_prev_eve_from_data), m_eve_diff_prev,
                EVEDIFF_NUM, EVEDIFF_MIN, EVEDIFF_MAX);
    if (m_eve_from_data - m_prev_eve_from_data != 1) {
      printf("daiff eve %d %d %d %d\n", m_eve_from_data , m_prev_eve_from_data , m_eve_from_data - m_prev_eve_from_data,
             m_eve_cnt);
    }

    if ((m_eve_cnt & 0xFFFF) != m_eve_from_ftsw) {
      //    if( ( m_eve_from_data & 0xFFFF ) != m_eve_from_ftsw ){
      printf("Diff eve %d %d %d\n", m_eve_from_data , m_eve_from_ftsw,
             m_eve_cnt);
    }
  }
  m_prev_eve_from_data = m_eve_from_data;
  fillHisto1D((double)(m_eve_from_data - m_eve_cnt), m_eve_diff,
              EVEDIFF_NUM, EVEDIFF_MIN, EVEDIFF_MAX);


  for (int finesse = 0; finesse < 4; finesse++) {
    if (raw_copper->GetDetectorNwords(i, finesse) > 0) {
      if ((5 - 5 < m_eve_cnt && m_eve_cnt < 5 + 5)
          //   if( ( 173848 -5 < m_eve_cnt && m_eve_cnt < 173848 + 5)
         ) { // for run10038
        //   if( ( 579 -5 < m_eve_cnt && m_eve_cnt < 579 + 5)
        //       ){ // for run10036
        //     if( ( 25348 -5 < m_eve_cnt && m_eve_cnt < 25348 + 5) ||
        //  ( 1292201 -5 < m_eve_cnt && m_eve_cnt < 1292201 + 5) ||
        //  ( 384553 -5 < m_eve_cnt && m_eve_cnt < 384553 + 5) ||
        //  ( 1026080  -5 < m_eve_cnt && m_eve_cnt < 1026080 + 5)
        //  ){ //for run10048

        //      int* temp_buf = raw_copper->GetBuffer(i);
        int* temp_buf = raw_copper->GetFINESSEBuffer(i, finesse);
        int nwords = raw_copper->GetFINESSENwords(i, finesse);
        unsigned int hslb_eve = temp_buf[0] & 0xFFFF; //temp_buf[];
        unsigned int tt_eve = temp_buf[2] & 0xFFFF; //temp_buf[];
        unsigned int tt_ctime = (temp_buf[1] >> 4) & 0x7FFFFFF ;
        unsigned int tt_utime1 = (temp_buf[2] >> 16) & 0xFFFF ;
        unsigned int b2l_ctime = (temp_buf[4] >> 4) & 0x7FFFFFF;
        unsigned int tt_utime2 = 0x12345678;
        if (nwords >= 2) {
          tt_utime2 = (temp_buf[ nwords - 2 ] >> 16) & 0xFFFF;
        }
        printf("CDC1  %x %x %x %x %x %x %d\n",
               hslb_eve, tt_eve, tt_ctime, tt_utime1, b2l_ctime, tt_utime2,
               nwords
              );


        //      PrintData(raw_copper->GetBuffer(i), raw_copper->GetBlockNwords(i) );
      }
    }
  }



  m_ncpr++;

}


void PrintDataModule::event()
{
  B2INFO("PrintData: event() started.");

#define TEMP
#ifdef TEMP
  //
  // FTSW data
  //
  StoreArray<RawFTSW> raw_ftswarray;
  StoreArray<RawCOPPER> rawcprarray;
  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_ftswarray.getEntries(); i++) {

    for (int j = 0; j < raw_ftswarray[ i ]->GetNumEntries(); j++) {
#ifdef DEBUG
      printf("\n0==== DataBlock(CDC and TRG) : Block # %d ", i);
#endif
      printFTSWEvent(raw_ftswarray[ i ], j);
      printCOPPEREvent(raw_cdcarray[ i ], j);
#ifdef DEBUG
      printf("\n5==== DataBlock(FTSW): Block # %d\n", i);

#endif


    }
    m_eve_cnt++;
  }


#endif


  n_basf2evt++;

}
