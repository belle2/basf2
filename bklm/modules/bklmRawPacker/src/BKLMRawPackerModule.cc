//+
// File : BKLMRawPackerModule.cc
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's BKLMRawPackerModule
//-

#include <bklm/modules/bklmRawPacker/BKLMRawPackerModule.h>
#include <bklm/dataobjects/BKLMDigit.h>
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
  addParam("loadMapFromDB", m_loadMapFromDB, "load electronic map from DataBase or not", false);

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

  rawklmarray.registerPersistent();

  B2DEBUG(1, "BKLMRawPackerModule: initialize() done.");
  if (m_loadMapFromDB) loadMapFromDB();
  else loadMap();
}




void BKLMRawPackerModule::event()
{
  B2DEBUG(1, "pack the event..");
  StoreArray<BKLMDigit> digits;
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
    BKLMDigit* digit = digits[d];

    int iChannelNr = digit->getStrip();
    int iAx = digit->isPhiReadout();
    int iLayer = digit->getLayer();
    int iSector = digit->getSector();
    int isForward = digit->isForward();
    int iTdc = digit->getTime();
    int icharge = digit->getNPixel();
    int iCTime = digit->getCTime();
    int moduleId = (isForward ? BKLM_END_MASK : 0)
                   | ((iSector - 1) << BKLM_SECTOR_BIT)
                   | ((iLayer - 1) << BKLM_LAYER_BIT)
                   | ((iAx) << BKLM_PLANE_BIT);

    int electId = 0;
    if (m_ModuleIdToelectId.find(moduleId) == m_ModuleIdToelectId.end()) {
      if (m_useDefaultElectId) {
        B2DEBUG(1, "BKLMRawPacker::can not find in mapping. Use the default ElectId");
        electId = 1;
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
    int plane;
    intToElectCoo(electId, copperId, finesse, lane, plane);

    B2DEBUG(1, "BKLMRawPacker::copperId " << copperId << " " << isForward << " " << iSector << " " << lane << " " << plane << " " <<
            iChannelNr << " " << iTdc << " " << icharge << " " << iCTime);

    unsigned short bword1 = 0;
    unsigned short bword2 = 0;
    unsigned short bword3 = 0;
    unsigned short bword4 = 0;
    formatData(iChannelNr, plane, lane, iTdc, icharge, iCTime, bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    data_words[copperId][finesse].push_back(buf[0]);
    data_words[copperId][finesse].push_back(buf[1]);

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
    rawcprpacker_info.node_id = BKLM_ID + i;
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;
    rawcprpacker_info.b2l_ctime = 0x7654321;

    //one call per copper
    RawKLM* raw_klm = rawklmarray.appendNew();

    int* buf1, *buf2, *buf3, *buf4;
    int nwords_1st = data_words[i][0].size();
    int nwords_2nd = data_words[i][1].size();
    int nwords_3rd = data_words[i][2].size();
    int nwords_4th = data_words[i][3].size();

    buf1 = NULL;
    buf2 = NULL;
    buf3 = NULL;
    buf4 = NULL;
    buf1 = new int[ nwords_1st + 1];
    buf2 = new int[ nwords_2nd + 1];
    buf3 = new int[ nwords_3rd + 1];
    buf4 = new int[ nwords_4th + 1];

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



void BKLMRawPackerModule::formatData(int channel, int axis, int lane, int tdc, int charge, int ctime, unsigned short& bword1,
                                     unsigned short& bword2, unsigned short& bword3, unsigned short& bword4)
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

void BKLMRawPackerModule::loadMapFromDB()
{
  DBArray<BKLMElectronicMapping> elements("BKLMElectronicMapping");
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
    int sector = element.getSector();
    int isForward = element.getIsForward();
    int layer = element.getLayer();
    int plane =  element.getPlane();
    int elecId = electCooToInt(copperId, slotId - 1, laneId, axisId);
    int moduleId = 0;
    B2DEBUG(1, "BKLMRawPackerModule::reading Data Base for BKLMElectronicMapping...");
    moduleId = (isForward ? BKLM_END_MASK : 0)
               | ((sector - 1) << BKLM_SECTOR_BIT)
               | ((layer - 1) << BKLM_LAYER_BIT)
               | ((plane) << BKLM_PLANE_BIT);
    m_ModuleIdToelectId[moduleId] = elecId;
    B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
  }

}

void BKLMRawPackerModule::loadMap()
{
  GearDir dir("/Detector/ElectronicsMapping/BKLM");

  for (GearDir& copper : dir.getNodes("COPPER")) {
    //GearDir& ID : copper.getNodes("ID")
    //int id=copper.getInt("ID");
    int copperId = copper.getInt("@id");
    //      cout <<"copper id: "<< id <<endl;
    for (GearDir& slot : copper.getNodes("Slot")) {
      int slotId = slot.getInt("@id");
      //      cout << "slotid : " << slotId << endl;
      B2DEBUG(1, "slotid: " << slotId);
      for (GearDir& lane : slot.getNodes("Lane")) {
        int laneId = lane.getInt("@id");
        for (GearDir& axis : lane.getNodes("Axis")) {
          int axisId = axis.getInt("@id");
          int sector = axis.getInt("Sector");
          int isForward = axis.getInt("IsForward");
          int layer = axis.getInt("Layer");
          int plane = axis.getInt("Plane");
          //int elecId = electCooToInt(copperId, slotId, laneId, axisId);
          int elecId = electCooToInt(copperId, slotId - 1, laneId, axisId);
          int moduleId = 0;


          B2DEBUG(1, "reading xml file...");
          B2DEBUG(1, " copperId: " << copperId << " slotId: " << slotId << " laneId: " << laneId << " axisId: " << axisId);
          B2DEBUG(1, " sector: " << sector << " isforward: " << isForward << " layer: " << layer << " plane: " << plane);
          moduleId = (isForward ? BKLM_END_MASK : 0)
                     | ((sector - 1) << BKLM_SECTOR_BIT)
                     | ((layer - 1) << BKLM_LAYER_BIT)
                     | ((plane) << BKLM_PLANE_BIT);
          //m_ModuleIdToelectId[elecId] = moduleId;
          m_ModuleIdToelectId[moduleId] = elecId;
          B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
        }
      }
    }
  }

}

void BKLMRawPackerModule::intToElectCoo(int id, int& copper, int& finesse, int& lane, int& plane)
{
  copper = 0;
  finesse = 0;
  lane = 0;
  copper = (id & 0xF);
  finesse = (id >> 4) & 3;
  lane = 0;
  lane = (id >> 6) & 0xF;
  plane = 0;
  plane = (id >> 10) & 0x1;

}

int BKLMRawPackerModule::electCooToInt(int copper, int finesse, int lane, int axis)
{
  //  there are at most 16 copper -->4 bit
  // 4 finesse --> 2 bit
  // < 16 lanes -->4 bit
  // axis --> 1 bit
  unsigned int ret = 0;
  copper = copper & 0xF;
  ret |= copper;
  finesse = finesse & 3;
  ret |= (finesse << 4);
  lane = lane & 0xF;
  ret |= (lane << 6);
  axis = axis & 0x1;
  ret |= (axis << 10);

  return ret;

}

//    void getTrack(int channel, short& bword1, short& bword2, short& bword3, short& bword4)
//    {
//
//    }
