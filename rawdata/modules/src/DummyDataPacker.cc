//+
// File : DummyDataPacker.cc
// Description : Module to store dummy data in RawCOPPER object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-

#include <rawdata/modules/DummyDataPacker.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyDataPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DummyDataPackerModule::DummyDataPackerModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawCOPPER object");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  B2INFO("DummyDataPacker: Constructor done.");

  // initialize event #
  n_basf2evt = 0;


}



DummyDataPackerModule::~DummyDataPackerModule()
{

}


void DummyDataPackerModule::initialize()
{
  B2INFO("DummyDataPacker: initialize() started.");

  // Open message handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  rawcprarray.registerPersistent();

  B2INFO("DummyDataPacker: initialize() done.");
}




void DummyDataPackerModule::event()
{

  //    Make RawCOPPER array
  rawcprarray.create();


  //
  // Fill event info (These values will be stored in RawHeader )
  //
  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 1;
  rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = n_basf2evt;
  rawcprpacker_info.node_id = m_nodeid;
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;


  //
  // Prepare buffer to fill dummy data
  //
  RawCOPPER* raw_copper = rawcprarray.appendNew();

  int* buf1, *buf2, *buf3, *buf4;
  int nwords_1st = 0, nwords_2nd = 0, nwords_3rd = 0, nwords_4th = 0;

  nwords_1st = n_basf2evt % 10;
  buf1 = new int[ nwords_1st];
  for (int i = 0; i < nwords_1st; i++) {
    buf1[ i ] = i;
  }

  nwords_2nd = (n_basf2evt + 1) % 10;
  buf2 = new int[ nwords_2nd];
  for (int i = 0; i < nwords_2nd; i++) {
    buf2[ i ] = i + 1;
  }

  nwords_3rd = 3 * (n_basf2evt + 2) % 10;
  buf3 = new int[ nwords_3rd];
  for (int i = 0; i < nwords_3rd; i++) {
    buf3[ i ] = i + 2;
  }

  nwords_4th = 4 * (n_basf2evt + 3)  % 10;
  buf4 = new int[ nwords_4th];
  for (int i = 0; i < nwords_4th; i++) {
    buf4[ i ] = i + 3;
  }

  raw_copper->PackDetectorBuf(buf1, nwords_1st,
                              buf2, nwords_2nd,
                              buf3, nwords_3rd,
                              buf4, nwords_4th,
                              rawcprpacker_info);



  delete [] buf1;
  delete [] buf2;
  delete [] buf3;
  delete [] buf4;

  //
  // Update EventMetaData : Not affect on the output
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(rawcprpacker_info.exp_num);
  m_eventMetaDataPtr->setRun(rawcprpacker_info.run_subrun_num);
  m_eventMetaDataPtr->setEvent(n_basf2evt);

  printf("Event # %.8d\n", n_basf2evt);
  fflush(stdout);

  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n",
             max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  n_basf2evt++;
  return;
}
