//+
// File : BKLMRawPackerModule.cc
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's BKLMRawPackerModule
//-

#include <bklm/modules/bklmRawPacker/BKLMRawPackerModule.h>
#include <framework/database/DBArray.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
#define NUM_BKLM_COPPER 4

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
  addParam("useDefaultModuleId", m_useDefaultElectId, "use default elect id if not found in mapping", true);
  addParam("loadMapFromDB", m_loadMapFromDB, "load electronic map from DataBase or not", true);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  B2DEBUG(1, "BKLMRawPackerModule: Constructor done.");

  // initialize event #
  n_basf2evt = 0;


}



BKLMRawPackerModule::~BKLMRawPackerModule()
{

}


void BKLMRawPackerModule::initialize()
{
  B2DEBUG(1, "BKLMRawPackerModule: initialize() started.");

  // Open message handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  rawklmarray.registerInDataStore();

  B2DEBUG(1, "BKLMRawPackerModule: initialize() done.");
  if (m_loadMapFromDB) loadMapFromDB();
}


void BKLMRawPackerModule::beginRun()
{
  //load ADCoffset from database
  m_scintADCOffset = m_ADCParams->getADCOffset();
}


void BKLMRawPackerModule::event()
{
  B2DEBUG(1, "pack the event..");
  vector<uint32_t> data_words[4][4];//4 copper, 16 finesse
  //data_words[4][4].clear();
  for (unsigned int ii = 0; ii < 4; ++ii) {
    for (unsigned int jj = 0; jj < 4; ++jj) {
      data_words[ii][jj].clear();
    }
  }
  //int tot_num_hits=digits.getEntries();

  B2DEBUG(1, "BKLMRawPackerModule:: entries of bklmdigits " << digits.getEntries());
  ///fill data_words
  for (int d = 0; d < digits.getEntries(); d++) {
    int* buf = new int[2];//for one hit, hit length is 2;
    buf[0] = 0;
    buf[1] = 0;
    BKLMDigit* bklmDigit = digits[d];

    int iChannelNr = bklmDigit->getStrip();
    int iAx = bklmDigit->isPhiReadout();
    int iLayer = bklmDigit->getLayer();
    int iSector = bklmDigit->getSector();
    int isForward = bklmDigit->isForward();
    float iTdc = bklmDigit->getTime();
    float icharge = bklmDigit->getCharge();
    short iCTime = bklmDigit->getCTime();
    bool isRPC = bklmDigit->inRPC();
    bool isAboveThresh = bklmDigit->isAboveThreshold();
    int moduleId = (isForward ? BKLM_END_MASK : 0)
                   | ((iSector - 1) << BKLM_SECTOR_BIT)
                   | ((iLayer - 1) << BKLM_LAYER_BIT)
                   | ((iAx) << BKLM_PLANE_BIT)
                   | ((iChannelNr - 1) << BKLM_STRIP_BIT);
    B2DEBUG(1, "BKLMRawPackerModule:: digi before packer: sector: " << iSector << " isforward: " << isForward << " layer: " << iLayer <<
            " plane: " << iAx << " icharge " << icharge << " tdc " << iTdc << " ctime " << iCTime << " isAboveThresh " << isAboveThresh <<
            " isRPC " << isRPC << " " << moduleId << bklmDigit->getModuleID());

    int electId = 0;
    if (m_ModuleIdToelectId.find(moduleId) == m_ModuleIdToelectId.end()) {
      if (m_useDefaultElectId) {
        B2DEBUG(1, "BKLMRawPacker::can not find in mapping. Use the default ElectId");
        electId = getDefaultElectId(isForward, iSector, iLayer, iAx, iChannelNr);
      } else {
        B2DEBUG(1, "BKLMRawPacker::can not find in mapping for moduleId " << moduleId << " isForward? " << isForward << " , sector " <<
                iSector);
        continue;
      }
    } else {
      electId = m_ModuleIdToelectId[moduleId];
    }

    int copperId;
    int finesse;
    int lane;
    int axis;
    int channelId;
    intToElectCoo(electId, copperId, finesse, lane, axis, channelId);

    B2DEBUG(1, "BKLMRawPacker::copperId " << copperId << " " << isForward << " " << iSector << " " << lane << " " << axis << " " <<
            channelId << " " << iTdc << " " << icharge << " " << iCTime);

    unsigned short bword1 = 0;
    unsigned short bword2 = 0;
    unsigned short bword3 = 0;
    unsigned short bword4 = 0;
    int flag;
    if (isRPC) flag = 2; //010
    else flag = 4; //100
    formatData(flag, channelId, axis, lane, iTdc, icharge, iCTime, bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    if (copperId < 1 || copperId > 4) { B2WARNING("BKLMRawPacker:: abnormal copper index: " << copperId); continue; }
    data_words[copperId - 1][finesse].push_back(buf[0]);
    data_words[copperId - 1][finesse].push_back(buf[1]);

    delete [] buf;
  }

  //    Make RawKLM array

  RawCOPPERPackerInfo rawcprpacker_info;
  for (int i = 0 ; i < NUM_BKLM_COPPER; i++) {
    //
    // Fill event info (These values will be stored in RawHeader )
    //
    rawcprpacker_info.exp_num = 1;
    rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
    rawcprpacker_info.eve_num = n_basf2evt;
    rawcprpacker_info.node_id = BKLM_ID + i + 1;
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;
    rawcprpacker_info.b2l_ctime = 0x7654321;

    //one call per copper
    RawKLM* raw_klm = rawklmarray.appendNew();

    int nwords_1st = data_words[i][0].size();
    int nwords_2nd = data_words[i][1].size();
    int nwords_3rd = data_words[i][2].size();
    int nwords_4th = data_words[i][3].size();

    int* buf1 = new int[ nwords_1st + 1];
    int* buf2 = new int[ nwords_2nd + 1];
    int* buf3 = new int[ nwords_3rd + 1];
    int* buf4 = new int[ nwords_4th + 1];

    for (int j = 0; j < nwords_1st; j++) {
      buf1[j] = data_words[i][0][j];
    }
    buf1[nwords_1st] = 0; // DIVOT

    for (int j = 0; j < nwords_2nd; j++) {
      buf2[j] = data_words[i][1][j];
    }
    buf2[nwords_2nd] = 0; // DIVOT

    for (int j = 0; j < nwords_3rd; j++) {
      buf3[j] = data_words[i][2][j];
    }
    buf3[nwords_3rd] = 0; // DIVOT

    for (int j = 0; j < nwords_4th; j++) {
      buf4[j] = data_words[i][3][j];
    }
    buf4[nwords_4th] = 0; // DIVOT


    raw_klm->PackDetectorBuf(buf1, nwords_1st + 1,
                             buf2, nwords_2nd + 1,
                             buf3, nwords_3rd + 1,
                             buf4, nwords_4th + 1,
                             rawcprpacker_info);

    delete [] buf1;
    delete [] buf2;
    delete [] buf3;
    delete [] buf4;

  }


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

void BKLMRawPackerModule::endRun()
{

}


void BKLMRawPackerModule::formatData(int flag, int channel, int axis, int lane, int tdc, int charge, int ctime,
                                     unsigned short& bword1,
                                     unsigned short& bword2, unsigned short& bword3, unsigned short& bword4)
{

  charge = m_scintADCOffset - charge;
  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (channel & 0x7F);
  bword1 |= ((axis & 1) << 7);
  bword1 |= ((lane & 0x1F) << 8);
  bword1 |= ((flag & 0x7) << 13);
  bword2 |= (ctime & 0xFFFF);
  bword3 |= (tdc & 0x7FF);
  bword4 |= (charge & 0xFFF);

}

void BKLMRawPackerModule::loadMapFromDB()
{
  DBArray<BKLMElectronicMapping> elements;
  elements.getEntries();
  for (const auto& element : elements) {
    B2DEBUG(1, "Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
            ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
            ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
            " plane(z/phi) = " << element.getPlane());

    int copperId = element.getCopperId();
    int slotId = element.getSlotId();
    int laneId = element.getLaneId();
    int axisId = element.getAxisId();
    int channelId = element.getChannelId();
    int sector = element.getSector();
    int isForward = element.getIsForward();
    int layer = element.getLayer();
    int plane =  element.getPlane();
    int stripId = element.getStripId();
    int elecId = electCooToInt(copperId - BKLM_ID, slotId - 1, laneId, axisId, channelId);
    int moduleId = 0;
    B2DEBUG(1, "BKLMRawPackerModule::reading Data Base for BKLMElectronicMapping...");
    moduleId = (isForward ? BKLM_END_MASK : 0)
               | ((sector - 1) << BKLM_SECTOR_BIT)
               | ((layer - 1) << BKLM_LAYER_BIT)
               | ((plane) << BKLM_PLANE_BIT)
               | ((stripId - 1) << BKLM_STRIP_BIT);
    m_ModuleIdToelectId[moduleId] = elecId;
    B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
  }

}

void BKLMRawPackerModule::intToElectCoo(int id, int& copper, int& finesse, int& lane, int& axis, int& channelId)
{
  copper = 0;
  finesse = 0;
  lane = 0;
  copper = (id & 0xF);
  finesse = (id >> 4) & 3;
  lane = 0;
  lane = (id >> 6) & 0x1F;
  axis = 0;
  axis = (id >> 11) & 0x1;
  channelId = 0;
  channelId = (id >> 12) & 0x3F;

}

int BKLMRawPackerModule::electCooToInt(int copper, int finesse, int lane, int axis, int channel)
{
  //  there are at most 16 copper -->4 bit
  // 4 finesse --> 2 bit
  // < 20 lanes -->5 bit
  // axis --> 1 bit
  // channel --> 6 bit
  int ret = 0;
  copper = copper & 0xF;
  ret |= copper;
  finesse = finesse & 3;
  ret |= (finesse << 4);
  lane = lane & 0x1F;
  ret |= (lane << 6);
  axis = axis & 0x1;
  ret |= (axis << 11);
  channel = channel & 0x3F;
  ret |= (channel << 12);

  return ret;

}

int BKLMRawPackerModule::getDefaultElectId(int isForward, int sector, int layer, int plane, int stripId)
{

  int copperId = 0;
  int finesse = 0;
  int lane = 0;
  int axisId = 0;
  if (isForward && (sector == 3 || sector == 4 || sector == 5 || sector == 6)) copperId = 1;
  if (isForward && (sector == 1 || sector == 2 || sector == 7 || sector == 8)) copperId = 2;
  if (!isForward && (sector == 3 || sector == 4 || sector == 5 || sector == 6)) copperId = 3;
  if (!isForward && (sector == 1 || sector == 2 || sector == 7 || sector == 8)) copperId = 4;
  if (sector == 3 || sector == 4 || sector == 5 || sector == 6) finesse = sector - 3;
  if (sector == 1 || sector == 2) finesse = sector + 1;
  if (sector == 7 || sector == 8) finesse = sector - 7;
  if (layer > 2) lane = layer + 5;
  else lane = layer;
  if (layer > 2) axisId = plane;
  else { if (plane == 0) axisId = 1; else axisId = 0; }

  int channelId = getChannel(isForward, sector, layer, plane, stripId);
  return electCooToInt(copperId, finesse , lane, axisId, channelId);

}

//better to put into database
int BKLMRawPackerModule::getChannel(int isForward, int sector, int layer, int plane, int channel)
{

  //we flip channel to match raw data
  int MaxiChannel = 0;
  if (!isForward && sector == 3 && plane == 0) {
    if (plane == 0 && layer < 3) MaxiChannel = 38;
    if (plane == 0 && layer > 2) MaxiChannel = 34;
  } else {
    if (layer == 1 && plane == 1) MaxiChannel = 37;
    if (layer == 2 && plane == 1) MaxiChannel = 42;
    if (layer > 2 && layer < 7 && plane == 1) MaxiChannel = 36;
    if (layer > 6 && plane == 1) MaxiChannel = 48;

    if (layer == 1 && plane == 0) MaxiChannel = 54;
    if (layer == 2 && plane == 0) MaxiChannel = 54;
    if (layer > 2 && plane == 0) MaxiChannel = 48;
  }

  bool dontFlip = false;
  if (isForward && (sector == 7 ||  sector == 8 ||  sector == 1 ||  sector == 2)) dontFlip = true;
  if (!isForward && (sector == 4 ||  sector == 5 ||  sector == 6 ||  sector == 7)) dontFlip = true;
  if (!(dontFlip && layer > 2 && plane == 1)) channel = MaxiChannel - channel + 1;

  if (plane == 1) { //phi strips
    if (layer == 1)  channel = channel + 4;
    if (layer == 2)  channel = channel + 2;
  } else if (plane == 0) { //z strips
    if (layer < 3) { //scintillator
      if (isForward == 0 && sector == 3) {
        if (layer == 1) {
          if (channel > 0 && channel < 9) channel = 9 - channel;
          else if (channel > 8 && channel < 24) channel = 54 - channel;
          else if (channel > 23 && channel < 39) channel = 54 - channel;
        } else {
          if (channel > 0 && channel < 10) channel = 10 - channel;
          else if (channel > 9 && channel < 24) channel = 40 - channel;
          else if (channel > 23 && channel < 39) channel = 69 - channel;
        }
      } else { //all sectors except backward #3
        if (channel > 0 && channel < 10) channel = 10 - channel;
        else if (channel > 9 && channel < 25) channel = 40 - channel;
        else if (channel > 24 && channel < 40) channel = 70 - channel;
        else if (channel > 39 && channel < 55) channel = 100 - channel;
      }
    }
  }

  return channel;
}

//    void getTrack(int channel, short& bword1, short& bword2, short& bword3, short& bword4)
//    {
//
//    }
