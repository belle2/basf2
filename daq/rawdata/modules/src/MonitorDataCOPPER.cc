//+
// File : MonitorDataCOPPER.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/MonitorDataCOPPER.h>


using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MonitorDataCOPPER)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MonitorDataCOPPERModule::MonitorDataCOPPERModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Raw Data");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_nevt = -1;

}



MonitorDataCOPPERModule::~MonitorDataCOPPERModule()
{
}

void MonitorDataCOPPERModule::defineHisto()
{
  h_ncpr = new TH1F("h_ncpr", "Number of COPPERs", 10, 0.0, 10.0);
  h_nevt = new TH1F("h_nevt", "Number of Events; Number of events; # of entries", 10, 0.0, 10.0);
  h_size = new TH1F("h_size", "Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size2d = new TH2F("h_size2d", "Data Size vs. Copper;Copper ID;Data size [byte]",
                      100, 0.0, 2000.0, 5, 0.0, 5.0);
}


void MonitorDataCOPPERModule::initialize()
{
  REG_HISTOGRAM
}


void MonitorDataCOPPERModule::beginRun()
{

}




void MonitorDataCOPPERModule::endRun()
{
  //fill Run data


}


void MonitorDataCOPPERModule::terminate()
{

}



//
// User defined functions
//


void MonitorDataCOPPERModule::event()
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
  if (m_nevt % 1000 == 0) {
    printf("m_nevt = %d\n", m_nevt);

  }
}
