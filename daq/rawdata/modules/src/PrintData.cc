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

  int tot_size_byte = raw_datablock->TotalBufNwords() * sizeof(int);


  int* buf;
  int size_byte = 0;
  buf = raw_datablock->GetBuffer(i);
  size_byte = raw_datablock->GetBlockNwords(i) * sizeof(int);
  if (!raw_datablock->CheckFTSWID(i)) {
    RawHeader rawhdr;
    rawhdr.SetBuffer(raw_datablock->GetBuffer(i));
    printf("== (size %d) : %d (size %d) : This is a non-FTSW(COPPER)block\nexp %d run %d eve %d copperNode %d type %d\n",
           tot_size_byte, i, size_byte,
           rawhdr.GetExpNo(),
           rawhdr.GetRunNo(),
           rawhdr.GetEveNo(),
           rawhdr.GetSubsysId(),
           rawhdr.GetDataType());

  } else {

    printf("== (size %d) : %d (size %d) : This is a FTSW block\n",
           tot_size_byte, i, size_byte);

    printf("*******BODY**********\n");
    printf("\n%.8d : ", 0);
    for (int j = 0; j < size_byte / sizeof(int); j++) {
      printf("0x%.8x ", (raw_datablock->GetBuffer(i))[ j ]);
      if ((j + 1) % 10 == 0) {
        printf("\n%.8d : ", j + 1);
      }
    }
    printf("\n");
    printf("\n");

  }

}



void PrintDataModule::PrintCOPPEREvent(RawCOPPER* raw_copper)
{


  int tot_size_byte = raw_copper->TotalBufNwords() * sizeof(int);
  for (int i = 0; i < raw_copper->GetNumEntries(); i++) {

    RawHeader rawhdr;
    int* buf;
    int size_byte = 0;
    buf = raw_copper->GetBuffer(i);

    rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(i));

    size_byte = raw_copper->GetBlockNwords(i) * sizeof(int);

    //
    // Extract FEE buffer
    //
    int* finnesse_buf_1st;
    int* finnesse_buf_2nd;
    int* finnesse_buf_3rd;
    int* finnesse_buf_4th;

    int* detector_buf_1st;
    int* detector_buf_2nd;
    int* detector_buf_3rd;
    int* detector_buf_4th;


    printf("(size %d) : %d (size %d)\nexp %d run %d eve %d copperNode %d type %d\n",
           tot_size_byte, i, size_byte,
           rawhdr.GetExpNo(),
           rawhdr.GetRunNo(),
           rawhdr.GetEveNo(),
           rawhdr.GetSubsysId(),
           rawhdr.GetDataType());


    finnesse_buf_1st = raw_copper->Get1stFINNESSEBuffer(i);
    finnesse_buf_2nd = raw_copper->Get2ndFINNESSEBuffer(i);
    finnesse_buf_3rd = raw_copper->Get3rdFINNESSEBuffer(i);
    finnesse_buf_4th = raw_copper->Get4thFINNESSEBuffer(i);
    printf("FINNNESSE buf %p %p %p %p\n", finnesse_buf_1st, finnesse_buf_2nd, finnesse_buf_3rd, finnesse_buf_4th);

    detector_buf_1st = raw_copper->Get1stDetectorBuffer(i);
    detector_buf_2nd = raw_copper->Get2ndDetectorBuffer(i);
    detector_buf_3rd = raw_copper->Get3rdDetectorBuffer(i);
    detector_buf_4th = raw_copper->Get4thDetectorBuffer(i);
    printf("Detector  buf %p %p %p %p\n", detector_buf_1st, detector_buf_2nd, detector_buf_3rd, detector_buf_4th);


    //#ifdef DEBUG
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
    //#endif


  }
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
    exit(1);
    printf("=== RawFTSW event====\nBlock # %d\n", j);
    for (int i = 0; i < raw_ftswarray[ j ]->GetNumEntries(); i++) {
      //  PrintEvent( &rawcprarray );
      PrintEvent(raw_ftswarray[ j ], i);
    }
  }


  for (int j = 0; j < rawcprarray.getEntries(); j++) {
    printf("=== RawCOPPER event====\nBlock %d ", j);
    //  PrintEvent( &rawcprarray );
    PrintCOPPEREvent(rawcprarray[ j ]);
  }

  for (int j = 0; j < raw_cdcarray.getEntries(); j++) {
    printf("=== RawCDC    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_cdcarray[ j ]);
  }

  for (int j = 0; j < raw_svdarray.getEntries(); j++) {
    printf("=== RawSVD    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_svdarray[ j ]);
  }
  for (int j = 0; j < raw_bpidarray.getEntries(); j++) {
    printf("=== RawBPID    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_bpidarray[ j ]);
  }
  for (int j = 0; j < raw_epidarray.getEntries(); j++) {
    printf("=== RawEPID    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_epidarray[ j ]);
  }
  for (int j = 0; j < raw_eclarray.getEntries(); j++) {
    printf("=== RawECL    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_eclarray[ j ]);
  }
  for (int j = 0; j < raw_klmarray.getEntries(); j++) {
    printf("=== RawKLM    event====\nBlock %d ", j);
    //  PrintCOPPEREvent( &rawcprarray );
    PrintCOPPEREvent(raw_klmarray[ j ]);
  }


  n_basf2evt++;

}
