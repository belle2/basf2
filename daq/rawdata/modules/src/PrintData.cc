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

}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::initialize()
{
  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

}


void PrintDataModule::beginRun()
{
  B2INFO("beginRun called.");
}




void PrintDataModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void PrintDataModule::terminate()
{
  B2INFO("terminate called");
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


void PrintDataModule::event()
{
  StoreArray<RawCOPPER> rawcprarray;

  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    RawHeader rawhdr;
    int* buf;
    int size_byte = 0;
    buf = rawcprarray[ j ]->GetBuffer();
    rawhdr.SetBuffer(rawcprarray[ j ]->GetRawHdrBufPtr());
    size_byte = rawcprarray[ j ]->Size() * sizeof(int);

    //
    // Extract FEE buffer
    //
    int* fee_buf_1st;
    int* fee_buf_2nd;
    int* fee_buf_3rd;
    int* fee_buf_4th;


    printf("=== event====\n exp %d run %d eve %d copperNode %d type %d size %d byte\n",
           rawhdr.GetExpNo(),
           rawhdr.GetRunNo(),
           rawhdr.GetEveNo(),
           rawhdr.GetSubsysId(),
           rawhdr.GetDataType(),
           size_byte);

    fee_buf_1st = rawcprarray[ j ]->Get1stFEEBuffer();
    fee_buf_2nd = rawcprarray[ j ]->Get2ndFEEBuffer();
    fee_buf_3rd = rawcprarray[ j ]->Get3rdFEEBuffer();
    fee_buf_4th = rawcprarray[ j ]->Get4thFEEBuffer();
    printf("FEEbuf %p %p %p %p\n", fee_buf_1st, fee_buf_2nd, fee_buf_3rd, fee_buf_4th);

  }


  n_basf2evt++;

}
