
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <bklm/modules/bklmUnpacker/BKLMUnpackerModule.h>

#include <framework/datastore/DataStore.h>
//#include <framework/datastore/DataObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <boost/crc.hpp>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <bklm/dataobjects/BKLMStatus.h>
#include <framework/database/DBArray.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>

#include <sstream>
#include <iomanip>
#include <cstring>

//#define HIWORD(l) ((WORD) (((DWORD) (l) >> 16) & 0xFFFF))


using namespace std;
using namespace Belle2;


REG_MODULE(BKLMUnpacker)


BKLMUnpackerModule::BKLMUnpackerModule() : Module()
{
  setDescription("Produce BKLMDigits from RawBKLM");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useDefaultModuleId", m_useDefaultModuleId, "use default module id if not found in mapping", true);
  addParam("keepEvenPackages", m_keepEvenPackages, "keep packages that have even length normally indicating that data was corrupted ",
           false);
  addParam("outputDigitsName", m_outputDigitsName, "name of BKLMDigit store array", string("BKLMDigits"));
  addParam("SciThreshold", m_scintThreshold, "scintillator strip hits with NPE lower this value will be marked as bad",
           double(7.0));
  addParam("loadMapFromDB", m_loadMapFromDB, "whether load electronic map from DataBase", true);
  addParam("rawdata", m_rawdata, "is this real rawdata (true) or MC data (false)", false);
}


BKLMUnpackerModule::~BKLMUnpackerModule()
{

}

void BKLMUnpackerModule::initialize()
{
  //StoreArray<BKLMDigit>::registerPersistent();
  StoreArray<BKLMDigit>bklmDigits(m_outputDigitsName);
  bklmDigits.registerInDataStore();
  if (m_loadMapFromDB) loadMapFromDB();
  else loadMap();
}

void BKLMUnpackerModule::loadMapFromDB()
{
  DBArray<BKLMElectronicMapping> elements("BKLMElectronicMapping");
  elements.getEntries();
  for (const auto& element : elements) {
    B2INFO("Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
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
    int elecId = electCooToInt(copperId - BKLM_ID, slotId, laneId, axisId);
    int moduleId = 0;
    B2DEBUG(1, "reading Data Base...");
    moduleId = (isForward ? BKLM_END_MASK : 0)
               | ((sector - 1) << BKLM_SECTOR_BIT)
               | ((layer - 1) << BKLM_LAYER_BIT)
               | ((plane) << BKLM_PLANE_BIT);
    m_electIdToModuleId[elecId] = moduleId;
    B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
  }

}

void BKLMUnpackerModule::loadMap()
{
  GearDir dir("/Detector/ElectronicsMapping/BKLM");

  for (GearDir& copper : dir.getNodes("COPPER")) {
    // UNUSED int id=copper.getInt("ID");
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
          int elecId = electCooToInt(copperId - BKLM_ID, slotId, laneId, axisId);
          int moduleId = 0;


          B2DEBUG(1, "reading xml file...");
          B2DEBUG(1, " copperId: " << copperId << " slotId: " << slotId << " laneId: " << laneId << " axisId: " << axisId);
          B2DEBUG(1, " sector: " << sector << " isforward: " << isForward << " layer: " << layer << " plane: " << plane);
          moduleId = (isForward ? BKLM_END_MASK : 0)
                     | ((sector - 1) << BKLM_SECTOR_BIT)
                     | ((layer - 1) << BKLM_LAYER_BIT)
                     | ((plane) << BKLM_PLANE_BIT);
          m_electIdToModuleId[elecId] = moduleId;
          B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
        }
      }
    }
  }

}


void BKLMUnpackerModule::beginRun()
{
  if (m_loadMapFromDB) loadMapFromDB();
  else loadMap();

}


void BKLMUnpackerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  unsigned long expNumber = eventMetaData->getExperiment();

  StoreArray<RawKLM> rawKLM;
  StoreArray<BKLMDigit> bklmDigits(m_outputDigitsName);
  bklmDigits.clear();

  B2DEBUG(1, "Unpacker has have " << rawKLM.getEntries() << " entries");
  for (int i = 0; i < rawKLM.getEntries(); i++) {

    if (rawKLM[i]->GetNumEvents() != 1) {
      B2DEBUG(1, "rawKLM index " << i << " has more than one entry: " << rawKLM[i]->GetNumEvents());
      continue;
    }
    B2DEBUG(1, "num events in buffer: " << rawKLM[i]->GetNumEvents() << " number of nodes (copper boards) " <<
            rawKLM[i]->GetNumNodes());
    //getNumEntries is defined in RawDataBlock.h and gives the numberOfNodes*numberOfEvents. Number of nodes is num copper boards
    for (int j = 0; j < rawKLM[i]->GetNumEntries(); j++) {
      //since the buffer has multiple events this gets each event/node... but how to disentangle events? Maybe only one event there?
      //is Finesse and detector the same??
      ///   int nWords = rawKLM[i]->Get1stFINESSENwords(j);

      //is this the same as get1stDetectorBuffer??
      ///   int* data=rawKLM[i]->Get1stFINESSEBuffer(j);
      unsigned int copperId = rawKLM[i]->GetNodeID(j);
      //old 117440512 - 117440515 , new Data: 117440513 -- 117440516
      if (copperId < BKLM_ID  || copperId > BKLM_ID + 4) continue;
      //short sCopperId = rawKLM[i]->GetCOPPERNodeId(j);


      rawKLM[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        //addendum: There is always an additional word (count) in the end
        int numDetNwords = rawKLM[i]->GetDetectorNwords(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        int* buf_slot = rawKLM[i]->GetDetectorBuffer(j, finesse_num);
        //// cout << "data in finesse num: " << finesse_num << "( " << rawKLM[i]->GetDetectorNwords(j,             finesse_num) << " words, " << numHits << " hits)" << endl;
        //if (numDetNwords > 0)
        //  cout << "word counter is: " << ((buf_slot[numDetNwords - 1] >> 16) & 0xFFFF) << endl;
        //// cout << "trigger tag is " << rawKLM[i]->GetTRGType(j) << endl;
        //// cout << "ctime is : " << rawKLM[i]->GetTTCtime(j) << endl << endl;
        //we should get two words of 32 bits...

        for (int k = 0; k < numDetNwords; k++) {
          int item = buf_slot[k];
          char buffer[200] = "";
          sprintf(buffer, "%.8x\n", item);
          B2DEBUG(1, buffer);

          //Brandon uses 4 16 bit words
          // int firstBrandonWord;
          // int secondBrandonWord;
          char buffer1[100] = "";
          char buffer2[100] = "";

          sprintf(buffer1, "%.4x\n", item & 0xffff);
          sprintf(buffer2, "%.4x\n", (item >> 16) & 0xffff);

          ////cout << buffer2 << buffer1;

          //  printf("%.8x\n", buf_slot[k]);

          //in Brandon's documenation a word is 16 bit, however the basf2 word seems to be 32 bit
          //first word
          //lowest 7 bits: channel, 1bit axis, 5 bit lane, 2 bits TBD
          //second word:
          // ctime (16 bits)
          //third word:
          //TDC (lowest 11 bits)
          //fourth word:
          //charge (lowest 12bits)
          //if (!((k - 1) % (hitLength)))
          //  cout << endl;
        }


        //either no data (finesse not connected) or with the count word
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          if (!m_keepEvenPackages) {
            B2DEBUG(1, "word count incorrect: " << numDetNwords);
            continue;
          }
        }
        B2DEBUG(1, "this finesse has " << numHits << " hits");

        if (numDetNwords > 0)
          B2DEBUG(1, "counter is: " << (buf_slot[numDetNwords - 1] & 0xFFFF));
        //careful, changed start to 1 to get rid of the first rpc hit which is meaningless (at least as long no rpc data is taken)
        //for (int iHit = 1; iHit < numHits; iHit++) {
        //changed start to 0 to test BKLMRawPacker. Nov.13 2015)
        for (int iHit = 0; iHit < numHits; iHit++) {
          B2DEBUG(1, "unpacking first word: " << buf_slot[iHit * hitLength + 0] << ", second: " << buf_slot[iHit * hitLength + 1]);
          //--->first word is the leftmost, not rightmost
          unsigned short bword2 = buf_slot[iHit * hitLength + 0] & 0xFFFF;
          unsigned short bword1 = (buf_slot[iHit * hitLength + 0] >> 16) & 0xFFFF;
          unsigned short bword4 = buf_slot[iHit * hitLength + 1] & 0xFFFF;
          unsigned short bword3 = (buf_slot[iHit * hitLength + 1] >> 16) & 0xFFFF;

          B2DEBUG(1, "unpacking " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4);

          unsigned short channel = bword1 & 0x7F;
          unsigned short axis = (bword1 >> 7) & 1;
          //lane is the slot in the crate
          unsigned short lane = (bword1 >> 8) & 0x1F;
          unsigned short flag = (bword1 >> 14);
          unsigned short ctime = bword2 & 0xFFFF; //full bword
          unsigned short tdc = bword3 & 0x7FF;
          unsigned short charge = bword4 & 0xFFF;
          int layer = lane;
          if (flag == 1) layer = lane - 5; //layer 1-based
          if (layer < 3) { // z phi plane of sci. is flipped, may be tentative
            if (axis == 0) axis = 1;
            else if (axis == 1) axis = 0;
            else B2WARNING("BKLMUnpackerModule:: axis bit of scintillator is abnormal " << axis);
          }
          //channel = getChannel(layer, axis, channel);//for data


          //if ((1 == layer || 2 == layer)  && fabs(charge - m_scintADCOffset) < m_scintThreshold) continue;

          B2DEBUG(1, "copper: " << copperId << " finesse: " << finesse_num);
          //  B2DEBUG(1, "Unpacker channel: " << channel << ", axi: " << axis << " lane: " << lane << " ctime: " << ctime << " tdc: " << tdc << " charge: " << charge)
          //  cout << "Unpacker channel: " << channel << ", axi: " << axis << " lane: " << lane << " ctime: " << ctime << " tdc: " << tdc <<
          //  " charge: " << charge << endl;

          int electId = electCooToInt(copperId - BKLM_ID, finesse_num + 1, layer, axis);
          int moduleId = 0;
          int sector = -1;
          int isForward = -1;
          int plane = -1;
          if (m_electIdToModuleId.find(electId) == m_electIdToModuleId.end()) {
            if (!m_useDefaultModuleId) {
              B2WARNING("could not find copperid " << copperId << ", finesse " << finesse_num + 1 << ", lane " << lane << ", axis " << axis <<
                        " in mapping");
              continue;
            } else {
              moduleId = getDefaultModuleId(copperId, finesse_num, layer, axis);
            }
          } else { //found moduleId in the mapping
            moduleId = m_electIdToModuleId[electId];
            B2DEBUG(1, " electid: " << electId << " module: " << moduleId);

            //only channel and inrpc flag is not set yet
          }
          //moduleId counts are zero based
          layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
          sector = (moduleId & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT;
          isForward = (moduleId & BKLM_END_MASK) >> BKLM_END_BIT;
          plane = (moduleId & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;

          if (layer > 14) { B2WARNING("BKLMUnpackerModule:: strange that the layer number is larger than 14 " << layer); continue;}

          //handle the flipped channels and out-of-range channels. This way is not good at all, but do this for a while before data format is fixed
          channel = getChannel(sector + 1, layer + 1, plane, channel);
          bool outRange = false;
          channel = flipChannel(isForward, sector + 1, layer + 1, plane, channel, outRange);
          if (outRange) { B2WARNING("BKLMUnpackerModule:: channel number is out of range " << channel); continue; }

          //still have to add the channel and axis
          if (layer > 1) moduleId |= BKLM_INRPC_MASK;
          moduleId |= (((channel - 1) & BKLM_STRIP_MASK) << BKLM_STRIP_BIT) | (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);

          BKLMDigit digit(moduleId, ctime, tdc, charge);
          if (layer < 2 && ((m_scintADCOffset - charge) > m_scintThreshold))  digit.isAboveThreshold(true);

          B2DEBUG(1, "BKLMUnpackerModule:: digi after Unpacker: sector: " << digit.getSector() << " isforward: " << digit.isForward() <<
                  " layer: " << digit.getLayer() << " isPhi: " << digit.isPhiReadout());
          B2DEBUG(1, "BKLMUnpackerModule:: charge " << digit.getNPixel() << " tdc" << digit.getTime() << " ctime " << ctime <<
                  " isAboveThresh " << digit.isAboveThreshold() << " isRPC " << digit.inRPC() << " moduleId " << digit.getModuleID());

          bklmDigits.appendNew(digit);

          B2DEBUG(1, "from digit:sector " << digit.getSector() << " layer: " << digit.getLayer() << " strip: " << digit.getStrip() << ", " <<
                  " isphi? " << digit.isPhiReadout() << " fwd? " << digit.isForward());
        }
      } //finesse boards

    } //copper boards
  }  // events... should be only 1...


}


int BKLMUnpackerModule::electCooToInt(int copper, int finesse, int lane, int axis)
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
///
///void BKLMUnpackerModule::intToElectCoo(int id, int& copper, int& finesse, int& lane)
///{
///  copper = 0;
///  finesse = 0;
///  lane = 0;
///  copper = (id & 0xF);
///  finesse = (id >> 4) & 3;
///  lane = 0;
///  lane = (id >> 6) & 0xF;
///
///}
///

void BKLMUnpackerModule::endRun()
{

}


void BKLMUnpackerModule::terminate()
{


}


int BKLMUnpackerModule::getDefaultModuleId(int copperId, int finesse, int lane, int axis)
{

  int sector = 0;
  int isForward = 0;
  if (copperId == 117440513 || copperId == 117440514) isForward = 1;
  if (copperId == 117440515 || copperId == 117440516) isForward = 0;
  if (copperId == 117440513 || copperId == 117440515) sector = finesse + 3;
  if (copperId == 117440514 || copperId == 117440516) sector = (finesse + 7 > 8) ? finesse - 1 : finesse + 7;

  //attention: moduleId counts are zero based
  int moduleId = (isForward ? BKLM_END_MASK : 0)
                 | ((sector - 1) << BKLM_SECTOR_BIT)
                 | ((lane - 1) << BKLM_LAYER_BIT)
                 | ((axis) << BKLM_PLANE_BIT);

  return moduleId;

}

unsigned short BKLMUnpackerModule::getChannel(int sector, int layer, int axis, unsigned short channel)
{

  if (layer == 1) {
    if (axis == 0) { //phi strips
      if (channel > 0 && channel < 5) channel = 0;
      //else channel = channel - 4;
      if (channel > 4 && channel < 42) channel = channel - 4;
      //if(channel>41) channel=0;
      if (channel > 41) channel = channel - 4;
    }

    if (axis == 1) { //z strips
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 9 && channel < 16) channel = 0;
      if (channel > 15 && channel < 61) channel = channel - 6;
      if (channel > 60) channel = channel - 6;
    }
  }
  if (layer == 2) {
    if (axis == 0) { //phi
      if (channel > 0 && channel < 3) channel = 0;
      if (channel > 2 && channel < 45) channel = channel - 2;
      if (channel > 44) channel = channel - 2;;
    }
    if (axis == 1) {
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 9 && channel < 16) channel = 0;
      if (channel > 15 && channel < 61) channel = channel - 6;
      if (channel > 60) channel = channel - 6;;
    }

  }


  //if (layer > 2) channel = channel + 1;
  //if (sector == 3 && layer > 2 && layer < 16) channel = channel + 1;
  //if (sector == 7 && layer > 2 && layer < 16) channel = channel;

  return channel;
}

unsigned short BKLMUnpackerModule::flipChannel(int isForward, int sector, int layer, int plane, unsigned short channel,
                                               bool& isOutRange)
{

  isOutRange = false;
  int MaxiChannel = 0;

  if (!isForward && sector == 3 && plane == 0) {
    if (plane == 0 && layer < 3) MaxiChannel = 38;
    if (plane == 0 && layer > 2) MaxiChannel = 34;
  } else {
    if (layer == 1 && plane == 1) MaxiChannel = 37;
    if (layer == 2 && plane == 1) MaxiChannel = 42;
    if (layer > 2 && layer < 7 && plane == 1) MaxiChannel = 36;
    if (layer > 6 && plane == 1) MaxiChannel = 48;
//z plane
    if (layer == 1 && plane == 0) MaxiChannel = 54;
    if (layer == 2 && plane == 0) MaxiChannel = 54;
    if (layer > 2 && plane == 0) MaxiChannel = 48;
  }

  bool dontFlip = false;
  if (isForward && (sector == 7 ||  sector == 8 ||  sector == 1 ||  sector == 2)) dontFlip = true;
  if (!isForward && (sector == 4 ||  sector == 5 ||  sector == 6 ||  sector == 7)) dontFlip = true;
  if (!(dontFlip && layer > 2 && plane == 1)) channel = MaxiChannel - channel + 1;

  if (channel < 1 || channel > MaxiChannel) isOutRange = true;

  return channel;
}
