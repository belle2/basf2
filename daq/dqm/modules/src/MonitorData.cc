//+
// File : MonitorData.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/dqm/modules/MonitorData.h>


using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MonitorData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MonitorDataModule::MonitorDataModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Raw Data");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_nevt = -1;

}



MonitorDataModule::~MonitorDataModule()
{
}

void MonitorDataModule::defineHisto()
{
  h_ncpr = new TH1F("NumCOPPER", "Number of COPPERs", 10, 0.0, 10.0);
  h_nevt = new TH1F("Nevent", "Number of Events", 10, 0.0, 10.0);
  h_size = new TH1F("Data Size", "Data Size", 100, 0.0, 2000.0);
  h_size2d = new TH2F("Data Size vs. Cpr", "Data Size vs. Copper", 100, 0.0, 2000.0, 5, 0.0, 5.0);
}


void MonitorDataModule::initialize()
{
  REG_HISTOGRAM
}


void MonitorDataModule::beginRun()
{
  B2INFO("beginRun called.");
}




void MonitorDataModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void MonitorDataModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//


void MonitorDataModule::event()
{
  h_nevt->Fill((float)m_nevt);
  StoreArray<RawCOPPER> rawcprarray;

  int ncpr = rawcprarray.getEntries();
  h_ncpr->Fill((float)ncpr);

  for (int j = 0; j < ncpr; j++) {
    for (int i = 0; i < rawcprarray[ j ]->GetNumEntries(); i++) {
      RawHeader rawhdr;
      int* buf;
      int size_byte = 0;
      buf = rawcprarray[ j ]->GetBuffer(i);
      rawhdr.SetBuffer(rawcprarray[ j ]->GetRawHdrBufPtr(i));
      size_byte = rawcprarray[ j ]->GetBlockNwords(i) * sizeof(int);
      h_size->Fill((float)size_byte);
      h_size2d->Fill((float)size_byte, (float)j);

      /*
      printf("=== event====\n exp %d run %d eve %d copperNode %d type %d size %d byte\n",
       rawhdr.GetExpNo(),
       rawhdr.GetRunNo(),
       rawhdr.GetEveNo(),
       rawhdr.GetSubsysId(),
       rawhdr.GetDataType(),
       size_byte);
      */

      int* finnesse_buf_1st = rawcprarray[ j ]->Get1stFINNESSEBuffer(i);
      int* finnesse_buf_2nd = rawcprarray[ j ]->Get2ndFINNESSEBuffer(i);
      int* finnesse_buf_3rd = rawcprarray[ j ]->Get3rdFINNESSEBuffer(i);
      int* finnesse_buf_4th = rawcprarray[ j ]->Get4thFINNESSEBuffer(i);
      //    printf("FEEbuf %p %p %p %p\n", fee_buf_1st, fee_buf_2nd, fee_buf_3rd, fee_buf_4th);
    }
  }
  m_nevt++;

}
