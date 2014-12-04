//+
// File : BKLMRawPackerModule.cc
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's BKLMRawPackerModule
//-

#include <bklm/modules/bklmRawPacker/BKLMRawPackerModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMRawPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMRawPackerModule::BKLMRawPackerModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawKLM object");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  B2INFO("BKLMRawPackerModule: Constructor done.");

  // initialize event #
  n_basf2evt = 0;


}



BKLMRawPackerModule::~BKLMRawPackerModule()
{

}


void BKLMRawPackerModule::initialize()
{
  B2INFO("BKLMRawPackerModule: initialize() started.");

  // Open message handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  rawklmarray.registerPersistent();

  B2INFO("BKLMRawPackerModule: initialize() done.");
}




void BKLMRawPackerModule::event()
{
  cout << "pack the event.." << endl;
  //    Make RawKLM array
  rawklmarray.create();

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

  //one call per copper, so here we only pack data for one sector
  RawKLM* raw_klm = rawklmarray.appendNew();
  //one hit per layer in both directions, this is what we would expect per finesse...
  int numHits = 13 * 2;
  int hitCounter = -1;
  int* buf1, *buf2, *buf3, *buf4;
  int nwords_1st = 2 * numHits, nwords_2nd = 2 * numHits, nwords_3rd = 2 * numHits, nwords_4th = 2 * numHits;
  buf1 = new int[ nwords_1st];
  buf2 = new int[ nwords_2nd];
  buf3 = new int[ nwords_3rd];
  buf4 = new int[ nwords_4th];
  for (int iLay = 1; iLay < 14; iLay++)    {
    for (int iAx = 0; iAx < 2; iAx++) {
      hitCounter++;
      //basf2 words are 32 bits, whereas in the dataformat documentation for the KLM assumes 16bits

      unsigned short bword1 = 0;
      unsigned short bword2 = 0;
      unsigned short bword3 = 0;
      unsigned short bword4 = 0;


      buf1[0 + hitCounter * 2] = 0;
      buf1[1 + hitCounter * 2] = 0;
      int iChannelNr = 10;

      //tdc, charge, ctime
      //we need to switch bword1/bword2 and bword3/bword4 since the hi/loword ordering
      //from the concentrator board has teh hiword on the rightmost side
      formatData(iChannelNr, iAx, iLay, 20, 30, 70, bword1, bword2, bword3, bword4);
      cout << "buf1: " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4 << endl;
      buf1[0 + hitCounter * 2] |= bword2;
      buf1[0 + hitCounter * 2] |= ((bword1 << 16));
      buf1[1 + hitCounter * 2] |= bword4;
      buf1[1 + hitCounter * 2] |= ((bword3 << 16));

      cout << "word1: " << buf1[0 + hitCounter * 2] << " word2: " << buf1[1 + hitCounter * 2] << endl;
      buf2[0 + hitCounter * 2] = 0;
      buf2[1 + hitCounter * 2] = 0;

      iChannelNr = 25;

      formatData(iChannelNr, iAx, iLay, 20, 30, 70, bword1, bword2, bword3, bword4);
      buf2[0 + hitCounter * 2] |= bword2;
      buf2[0 + hitCounter * 2] |= ((bword1 << 16) & 0xFFFF0000);
      buf2[1 + hitCounter * 2] |= bword4;
      buf2[1 + hitCounter * 2] |= ((bword3 << 16) & 0xFFFF0000);
      cout << "buf2: " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4 << endl;
      cout << "word1: " << buf2[0 + hitCounter * 2] << " word2: " << buf2[1 + hitCounter * 2] << endl;

      buf3[0 + hitCounter * 2] = 0;
      buf3[1 + hitCounter * 2] = 0;

      formatData(iChannelNr, iAx, iLay, 20, 30, 70, bword1, bword2, bword3, bword4);
      iChannelNr = 35;

      formatData(iChannelNr, iAx, iLay, 20, 30, 70, bword1, bword2, bword3, bword4);
      cout << "buf3: " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4 << endl;
      cout << "word1: " << buf3[0 + hitCounter * 2] << " word2: " << buf3[1 + hitCounter * 2] << endl;
      buf3[0 + hitCounter * 2] |= bword2;
      buf3[0 + hitCounter * 2] |= ((bword1 << 16) & 0xFFFF0000);
      buf3[1 + hitCounter * 2] |= bword4;
      buf3[1 + hitCounter * 2] |= ((bword3 << 16) & 0xFFFF0000);



      buf4[0 + hitCounter * 2] = 0;
      buf4[1 + hitCounter * 2] = 0;

      iChannelNr = 17;

      formatData(iChannelNr, iAx, iLay, 20, 30, 70, bword1, bword2, bword3, bword4);
      cout << "buf4: " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4 << endl;
      cout << "word1: " << buf4[0 + hitCounter * 2] << " word2: " << buf4[1 + hitCounter * 2] << endl;
      buf4[0 + hitCounter * 2] |= bword2;
      buf4[0 + hitCounter * 2] |= ((bword1 << 16) & 0xFFFF0000);
      buf4[1 + hitCounter * 2] |= bword4;
      buf4[1 + hitCounter * 2] |= ((bword3 << 16) & 0xFFFF0000);






    }
  }
  raw_klm->PackDetectorBuf(buf1, nwords_1st,
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



void BKLMRawPackerModule::formatData(int channel, int axis, int lane, int tdc, int charge, int ctime, unsigned short& bword1, unsigned short& bword2, unsigned short& bword3, unsigned short& bword4)
{

  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (channel & 0x7F);
  bword1 |= ((axis & 1) << 7);
  bword1 |= ((lane & 0x1F) << 8);
  bword2 |= (ctime & 0xFFFF);
  bword3 |= (tdc & 0x07FF);
  bword4 |= (charge & 0xFFF);

}
//    void getTrack(int channel, short& bword1, short& bword2, short& bword3, short& bword4)
//    {
//
//    }
