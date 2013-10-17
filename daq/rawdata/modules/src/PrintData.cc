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

#define NOT_USE_SOCKETLIB
//#define NOT_SEND
//#define DUMMY_DATA
#define TIME_MONITOR

//#define MULTIPLE_SEND
//#define MEMCPY_TO_ONE_BUFFER
#define SEND_BY_WRITEV

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


  m_nftsw = 0;

  m_ncdc = 0;

  m_fina_nwords = 0;

  m_finb_nwords = 0;

  m_finc_nwords = 0;

  m_find_nwords = 0;

  m_upper16_ftsw_utime = 0;

  m_upper16_cdc_utime = 0;

  m_prev_ftsw_time16 = 0;

  m_prev_cdc_time16 = 0;

  m_start_ftsw_time = 0.;

  m_start_cdc_time = 0.;

  m_last_utime_ftsw = 0.;

  m_last_utime_cdc = 0.;

  m_prev_ftsw_eve16 = -1;

  m_prev_cdc_eve16 = -1;

  m_cnt_ftsw_evejump = 0;

  m_cnt_cdc_evejump = 0;


}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::initialize()
{
  B2INFO("PrintData: initialize() started.");

  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);
  B2INFO("PrintData: initialize() done.");

}



//
// User defined functions
//

void PrintDataModule::VerifyCheckSum(int* buf)     // Should be modified
{

  int check_sum = 0;
  for (int i = 0 ; i < buf[0]; i++) {
    if (i != 2)   check_sum += buf[i];
  }

  if (buf[2] != check_sum) {
    cout << "Invalid checksum : " << check_sum << " " << buf[1] << endl;
    exit(1);
  }
}


void PrintDataModule::PrintEvent(RawDataBlock* raw_datablock, int i)
{

  // int tot_size_byte = raw_datablock->TotalBufNwords() * sizeof(int);


  int size_byte = 0;

  size_byte = raw_datablock->GetBlockNwords(i) * sizeof(int);
  if (!raw_datablock->CheckFTSWID(i)) {
    RawHeader rawhdr;
    rawhdr.SetBuffer(raw_datablock->GetBuffer(i));
//     printf("== (size %d) : %d (size %d) : This is a non-FTSW(COPPER)block\nexp %d run %d eve %d copperNode %d type %d\n",
//            tot_size_byte, i, size_byte,
//            rawhdr.GetExpNo(),
//            rawhdr.GetRunNo(),
//            rawhdr.GetEveNo(),
//            rawhdr.GetSubsysId(),
//            rawhdr.GetDataType());

  } else {

//     printf("== (size %d) : %d (size %d) : This is a FTSW block\n",
//            tot_size_byte, i, size_byte);

//#ifdef DEBUG
    if (true) {
//    if( m_cnt_cdc_evejump > 0 || m_cnt_ftsw_evejump > 0 ){

      printf("*******BODY**********\n");
      printf("\n 0 : ");
      for (int j = 0; j < (int)(size_byte / sizeof(int)); j++) {
        printf("0x%.8x ", (raw_datablock->GetBuffer(i))[ j ]);
        if ((j + 1) % 10 == 0) {
          printf("\n%.8d : ", j + 1);
        }
      }
      printf("\n");
      printf("\n");
    }

//     printf("i %d eve %d 16bit %d \n",i,
//     (raw_datablock->GetBuffer( i ))[ 7 ] & 0xFFFF,
//     ( ( raw_datablock->GetBuffer( i ) )[ 7 ] >> 16 ) & 0xFFFF );

//#endif
    int cur_eve16 = (raw_datablock->GetBuffer(i))[ 7 ] & 0xFFFF;
    if (m_prev_ftsw_eve16 != -1) {
      if ((cur_eve16 != m_prev_ftsw_eve16 + 1)
          && !(cur_eve16 == 0 && m_prev_ftsw_eve16 == 0xffff)) {


        printf("diff ftsw 0x%x 0x%x\n", cur_eve16, m_prev_ftsw_eve16);
        m_cnt_ftsw_evejump++;
      }
    }
    m_prev_ftsw_eve16 = cur_eve16;

    double cur_time16 = (((raw_datablock->GetBuffer(i))[ 7 ] >> 16) & 0xFFFF)
                        + (((raw_datablock->GetBuffer(i))[ 6 ] >> 4) & 0x7FFFFFF) / 1.27e8;
    if (m_nftsw == 0) {
      m_start_ftsw_time = (double)cur_time16;
    } else {
      if (cur_time16 < m_prev_ftsw_time16) {
        m_upper16_ftsw_utime++;
      }
    }
    m_prev_ftsw_time16 = cur_time16;



    m_last_utime_ftsw = (double)(cur_time16 + (m_upper16_ftsw_utime << 16));

    m_nftsw++;

  }

}



void PrintDataModule::PrintCOPPEREvent(RawCOPPER* raw_copper, int i)
{


  //  int tot_size_byte = raw_copper->TotalBufNwords() * sizeof(int);
  //  for (int i = 0; i < raw_copper->GetNumEntries(); i++) {

  RawHeader rawhdr;
  //  int* buf;
  //  int size_byte = 0;
  //  buf = raw_copper->GetBuffer(i);
  rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(i));
  //  size_byte = raw_copper->GetBlockNwords(i) * sizeof(int);

  //
  // Check incrementation of 16bit events
  //
  int cur_eve16 = raw_copper->GetFTSW16bitEventNumber(i);
  if (m_prev_cdc_eve16 != -1) {
    if ((cur_eve16 != m_prev_cdc_eve16 + 1)
        && !(cur_eve16 == 0 && m_prev_cdc_eve16 == 0xffff)) {
      if (cur_eve16 == m_prev_cdc_eve16) {
        printf("diff same cdc 0x%x 0x%x\n", cur_eve16, m_prev_cdc_eve16);
      } else {
        printf("diff cdc 0x%x 0x%x\n", cur_eve16, m_prev_cdc_eve16);
      }
      m_cnt_cdc_evejump++;
    }
  }
  m_prev_cdc_eve16 = cur_eve16;


  //
  // Obtain time
  //
  int cur_time16 = ((int)(raw_copper->GetEventUnixTime(i)) & 0xFFFF);
  if (m_ncdc == 0) {
    m_start_cdc_time = (double)cur_time16;
  } else {
    if (cur_time16 < m_prev_cdc_time16) {
      m_upper16_cdc_utime++;
    }
  }
  m_prev_cdc_time16 = cur_time16;
  m_last_utime_cdc = (double)(cur_time16 + (m_upper16_cdc_utime << 16));


  //
  // Obtain data size for each FINNESSE
  //
  m_fina_nwords += raw_copper->Get1stFINNESSENwords(i);
  m_finb_nwords += raw_copper->Get2ndFINNESSENwords(i);
  m_finc_nwords += raw_copper->Get3rdFINNESSENwords(i);
  m_find_nwords += raw_copper->Get4thFINNESSENwords(i);
  m_ncdc++;

  //    printf("cpreve %d\n", raw_copper->GetFTSW16bitEventNumber( i ) );
  //#ifdef DEBUG
  if (true) {
    //    if( m_cnt_cdc_evejump > 0 || m_cnt_ftsw_evejump > 0 ){

    printf("*******BODY**********\n");
    printf("\n%.8d : ", 0);
    for (int j = 0; j < raw_copper->GetBlockNwords(i); j++) {
      printf("0x%.8x ", (raw_copper->GetBuffer(i))[ j ]);
      if ((j + 1) % 10 == 0) {
        printf("\n%.8d : ", j + 1);
      }
    }
    printf("\n");
    printf("\n");
  }
  //#endif
  //  }
}


void PrintDataModule::event()
{
  B2INFO("PrintData: event() started.");
  StoreArray<RawDataBlock> raw_datablkarray;
  StoreArray<RawCOPPER> rawcprarray;
  StoreArray<RawCDC> raw_cdcarray;
  StoreArray<RawFTSW> raw_ftswarray;
  StoreArray<RawSVD> raw_svdarray;
  StoreArray<RawBPID> raw_bpidarray;
  StoreArray<RawEPID> raw_epidarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawECL> raw_eclarray;


  for (int j = 0; j < raw_ftswarray.getEntries(); j++) {
    //    printf("=== RawFTSW event====\nBlock # %d\n", j);
    for (int i = 0; i < raw_ftswarray[ j ]->GetNumEntries(); i++) {
      //  PrintEvent( &rawcprarray );
      PrintEvent(raw_ftswarray[ j ], i);
      if (j < rawcprarray.getEntries() && i < rawcprarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(rawcprarray[ j ], i);
      }
      if (j < raw_cdcarray.getEntries() && i < raw_cdcarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(raw_cdcarray[ j ], i);
      }
      if (j < raw_svdarray.getEntries() && i < raw_svdarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(raw_svdarray[ j ], i);
      }
      if (j < raw_bpidarray.getEntries() && i < raw_bpidarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(raw_bpidarray[ j ], i);
      }
      if (j < raw_epidarray.getEntries() && i < raw_epidarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(raw_epidarray[ j ], i);
      }
      if (j < raw_klmarray.getEntries() && i < raw_klmarray[ j ]->GetNumEntries()) {
        PrintCOPPEREvent(raw_klmarray[ j ], i);
      }
    }
  }


//   for (int j = 0; j < rawcprarray.getEntries(); j++) {
//     //    printf("=== RawCOPPER event====\nBlock %d ", j);
//     //  PrintEvent( &rawcprarray );
//     PrintCOPPEREvent(rawcprarray[ j ]);
//   }

//   for (int j = 0; j < raw_cdcarray.getEntries(); j++) {
//     //    printf("=== RawCDC    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_cdcarray[ j ]);
//   }

//   for (int j = 0; j < raw_svdarray.getEntries(); j++) {
//     printf("=== RawSVD    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_svdarray[ j ]);
//   }
//   for (int j = 0; j < raw_bpidarray.getEntries(); j++) {
//     printf("=== RawBPID    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_bpidarray[ j ]);
//   }
//   for (int j = 0; j < raw_epidarray.getEntries(); j++) {
//     printf("=== RawEPID    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_epidarray[ j ]);
//   }
//   for (int j = 0; j < raw_eclarray.getEntries(); j++) {
//     printf("=== RawECL    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_eclarray[ j ]);
//   }
//   for (int j = 0; j < raw_klmarray.getEntries(); j++) {
//     printf("=== RawKLM    event====\nBlock %d ", j);
//     //  PrintCOPPEREvent( &rawcprarray );
//     PrintCOPPEREvent(raw_klmarray[ j ]);
//   }


  printf("ftsw %d cdc %d a %d b %d c %d d %d : diffstart %lf ftswtime %lf cdctime %lf : %d %d\n", m_nftsw, m_ncdc, m_fina_nwords, m_finb_nwords, m_finc_nwords, m_find_nwords,
         m_start_ftsw_time - m_start_cdc_time, m_last_utime_ftsw - m_start_ftsw_time,
         m_last_utime_cdc - m_start_cdc_time, m_cnt_ftsw_evejump, m_cnt_cdc_evejump
        );

  n_basf2evt++;

}
