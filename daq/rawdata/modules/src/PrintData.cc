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

  //Parameter definition
  B2INFO("Tx: Constructor done.");
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


#ifdef DUMMY
  m_buffer = new int[ BUF_SIZE_WORD ];

#endif


  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Tx initialized.");
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));



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



void PrintDataModule::FillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawCOPPER* rawcpr)
{

  int total_send_nwords =
    hdr->GetHdrNwords() +
    rawcpr->m_header.GetHdrNwords() +
    rawcpr->GetBodyNwords() +
    rawcpr->m_trailer.GetTrlNwords() +
    trl->GetTrlNwords();

  hdr->SetNwords(total_send_nwords);

  return;
}




double PrintDataModule::GetTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}


void PrintDataModule::RecordTime(int event, double* array)
{
  if (event >= 50000 && event < 50500) {
    array[ event - 50000 ] = GetTimeSec() - m_start_time;
  }
  return;
}


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
  if (n_basf2evt <= 0) {
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }

#ifdef TIME_MONITOR
  double cur_time;
  RecordTime(n_basf2evt, time_array0);
#endif

  StoreArray<RawCOPPER> rawcprarray;
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    int* buf;
    int m_size_byte = 0;

    buf = rawcprarray[ j ]->GetBuffer();
    m_size_byte = rawcprarray[ j ]->Size() * sizeof(int);

    printf("evn %d j %d size %d\n", n_basf2evt,  j, m_size_byte);
  }

//   StoreObjPtr<RawCOPPER> rawcopper;
//   int* buf;
//   buf = rawcopper->GetBuffer();


//   m_eventMetaDataPtr.create();
//   m_eventMetaDataPtr->setExperiment(1);
//   m_eventMetaDataPtr->setRun(1);
//   m_eventMetaDataPtr->setEvent(n_basf2evt);
//   if( n_basf2evt == 100){
//   m_eventMetaDataPtr->setEndOfData();
//   }


#ifdef TIME_MONITOR
  RecordTime(n_basf2evt, time_array2);
#endif

  n_basf2evt++;

}
