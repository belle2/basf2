/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/modules/DummyDataPacker.h>

using namespace std;
using namespace Belle2;

#define USE_PCIE40

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyDataPacker);

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
  //  addParam("NodeID", m_nodeid, "Node ID", 0);

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
  m_eventMetaDataPtr.registerInDataStore();

  raw_cprarray.registerInDataStore();
  raw_svdarray.registerInDataStore();
  raw_cdcarray.registerInDataStore();
  raw_bpidarray.registerInDataStore();
  raw_epidarray.registerInDataStore();
  raw_eclarray.registerInDataStore();
  raw_klmarray.registerInDataStore();
  B2INFO("DummyDataPacker: initialize() done.");
}




void DummyDataPackerModule::event()
{
  //
  //  To Make RawCOPPER array
  //

//  raw_cprarray.create();

  //
  // To make RawDetector array use the following objects
  //

//   raw_cdcarray.create();
//   raw_bpidarray.create();
//   raw_epidarray.create();
//   raw_klmarray.create();
//   raw_eclarray.create();


  RawCOPPERPackerInfo rawcprpacker_info;

  int num_coppers = 48; // In SVD case
  for (int i = 0 ; i < num_coppers; i++) {

    //
    // Example of event info (These values will be stored in RawHeader )
    //
    rawcprpacker_info.exp_num = 1;
    rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
    rawcprpacker_info.eve_num = n_basf2evt;
    //    rawcprpacker_info.node_id = m_nodeid;
    rawcprpacker_info.node_id = SVD_ID + 1000 + (i + 1);   //cpr1001...cpr1048
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;
    rawcprpacker_info.b2l_ctime = 0x7654321;

    //
    // Prepare buffer to fill dummy data
    //
//    RawCOPPER* raw_copper = raw_cprarray.appendNew();
    RawSVD* raw_svd = raw_svdarray.appendNew();
//     RawCDC* raw_cdc = raw_cdcarray.appendNew();
//     RawTOP* raw_bpid = raw_bpidarray.appendNew();
//     RawARICH* raw_epid = raw_epidarray.appendNew();
//     RawECL* raw_ecl = raw_eclarray.appendNew();
//     RawKLM* raw_klm = raw_klmarray.appendNew();
//     RawTRG* raw_trg = raw_trgarray.appendNew();

#ifdef USE_PCIE40
    int* buf_pcie40[MAX_PCIE40_CH];
    int nwords_ch[MAX_PCIE40_CH];

    for (int j = 0; j < MAX_PCIE40_CH; j++) {
      nwords_ch[j] = n_basf2evt % 10 + 1;
      buf_pcie40[j] = new int[ nwords_ch[j]];
      for (int k = 0; k < nwords_ch[j]; k++) {
        buf_pcie40[j][k] = i + j + k;
      }
    }
    raw_svd->PackDetectorBuf(buf_pcie40, nwords_ch, rawcprpacker_info);

    for (int j = 0; j < MAX_PCIE40_CH; j++) {
      delete [] buf_pcie40[j];
    }
#else
    int* buf_hslb1, *buf_hslb2, *buf_hslb3, *buf_hslb4;
    int nwords_1st_hslb = 0, nwords_2nd_hslb = 0, nwords_3rd_hslb = 0, nwords_4th_hslb = 0;

    nwords_1st_hslb = n_basf2evt % 10 + 1;
    buf_hslb1 = new int[ nwords_1st_hslb];
    for (int j = 0; j < nwords_1st_hslb; j++) {
      buf_hslb1[ j ] = i + j;
    }

    nwords_2nd_hslb = (n_basf2evt + 1) % 10 + 1;
    buf_hslb2 = new int[ nwords_2nd_hslb];
    for (int j = 0; j < nwords_2nd_hslb; j++) {
      buf_hslb2[ j ] = i + j + 1;
    }

    nwords_3rd_hslb = 3 * (n_basf2evt + 2) % 10 + 1;
    buf_hslb3 = new int[ nwords_3rd_hslb];
    for (int j = 0; j < nwords_3rd_hslb; j++) {
      buf_hslb3[ j ] = i + j + 2;
    }

    nwords_4th_hslb = 4 * (n_basf2evt + 3)  % 10 + 1;
    buf_hslb4 = new int[ nwords_4th_hslb];
    for (int j = 0; j < nwords_4th_hslb; j++) {
      buf_hslb4[ j ] = i + j + 3;
    }

    raw_svd->PackDetectorBuf(buf_hslb1, nwords_1st_hslb,
                             buf_hslb2, nwords_2nd_hslb,
                             buf_hslb3, nwords_3rd_hslb,
                             buf_hslb4, nwords_4th_hslb,
                             rawcprpacker_info);


    delete [] buf_hslb1;
    delete [] buf_hslb2;
    delete [] buf_hslb3;
    delete [] buf_hslb4;
#endif

  }


  //
  // Update EventMetaData :
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(rawcprpacker_info.exp_num);
  m_eventMetaDataPtr->setRun(rawcprpacker_info.run_subrun_num);
  m_eventMetaDataPtr->setEvent(n_basf2evt);

  B2DEBUG(0, "Event # " <<  n_basf2evt << endl);
  fflush(stdout);

  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      B2DEBUG(0, "[DEBUG] RunStop was detected. ( Setting:  Max event #" << max_nevt << ") Processed Event" << n_basf2evt << endl);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  n_basf2evt++;
  return;
}
