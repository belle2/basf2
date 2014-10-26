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
  //setPropertyFlags(c_ParallelProcessingCertified); //not sure ibf true yet...
}


BKLMUnpackerModule::~BKLMUnpackerModule()
{

}

void BKLMUnpackerModule::initialize()
{
  StoreArray<BKLMDigit>::registerPersistent();
  loadMap();
}


void BKLMUnpackerModule::loadMap()
{
  GearDir dir("/Detector/ElectronicsMapping/BKLM");

  for (GearDir & copper : dir.getNodes("COPPER")) {
    // UNUSED int id=copper.getInt("ID");
    int copperId = copper.getInt("@id");
    //      cout <<"copper id: "<< id <<endl;
    for (GearDir & slot : copper.getNodes("Slot")) {
      int slotId = slot.getInt("@id");
      cout << "slotid : " << slotId << endl;
      for (GearDir & lane : slot.getNodes("Lane")) {
        int laneId = lane.getInt("@id");
        for (GearDir & axis : lane.getNodes("Axis")) {
          int axisId = axis.getInt("@id");
          int sector = axis.getInt("Sector");
          int isForward = axis.getInt("IsForward");
          int layer = axis.getInt("Layer");
          int plane = axis.getInt("Plane");
          int elecId = electCooToInt(copperId, slotId, laneId, axisId);
          int moduleId = 0;

          cout << "reading xml file...";
          cout << " copperId: " << copperId << " slotId: " << slotId << " laneId: " << laneId << " axisId: " << axisId;
          cout << " sector: " << sector << " isforward: " << isForward << " layer: " << layer << " plane: " << plane << endl;
          moduleId = (isForward ? BKLM_END_MASK : 0)
                     | ((sector - 1) << BKLM_SECTOR_BIT)
                     | ((layer - 1) << BKLM_LAYER_BIT)
                     | ((plane) << BKLM_PLANE_BIT);
          m_electIdToModuleId[elecId] = moduleId;
          cout << " electId: " << elecId << " modId: " << moduleId << endl;
        }
      }
    }
  }

}

void BKLMUnpackerModule::beginRun()
{
  loadMap();

}


void BKLMUnpackerModule::event()
{
  cout << "unpacker..." << endl;
  StoreArray<RawKLM> rawKLM;
  StoreArray<BKLMDigit> bklmDigits;


  for (int i = 0; i < rawKLM.getEntries(); i++) {

    if (rawKLM[i]->GetNumEvents() != 1) {
      B2ERROR("rawKLM has more than one entry");
      continue;
    }
    cout << "num events in buffer: " << rawKLM[i]->GetNumEvents() << " number of nodes (copper boards) " << rawKLM[i]->GetNumNodes() << endl;
    //getNumEntries is defined in RawDataBlock.h and gives the numberOfNodes*numberOfEvents. Number of nodes is num copper boards
    for (int j = 0; j < rawKLM[i]->GetNumEntries(); j++) {
      //since the buffer has multiple events this gets each event/node... but how to disentangle events? Maybe only one event there?
      //is Finesse and detector the same??
      ///   int nWords = rawKLM[i]->Get1stFINESSENwords(j);

      //is this the same as get1stDetectorBuffer??
      ///   int* data=rawKLM[i]->Get1stFINESSEBuffer(j);

      rawKLM[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        int* buf_slot = rawKLM[i]->GetDetectorBuffer(j, finesse_num);
        //      cout <<"data in finesse num: " << finesse_num<< "(we have " << rawKLM[i]->GetDetectorNwords(j,finesse_num) << " words " <<endl;
        //we should get two words of 32 bits...
        for (int k = 0; k < rawKLM[i]->GetDetectorNwords(j, finesse_num); k++) {
          printf("%.8x\n", buf_slot[k]);

          //in Brandon's documenation a word is 16 bit, however the basf2 word seems to be 32 bit
          //first word
          //lowest 7 bits: channel, 1bit axis, 5 bit lane, 2 bits TBD
          //second word:
          // ctime (16 bits)
          //third word:
          //TDC (lowest 11 bits)
          //fourth word:
          //charge (lowest 12bits)

        }

        if (rawKLM[i]->GetDetectorNwords(j, finesse_num) != 2) {
          char buffer[200];
          sprintf(buffer, "not the correct number of words: %d\n", rawKLM[i]->GetDetectorNwords(j, finesse_num));
          B2ERROR(buffer);
          continue;
        }
        cout << "unpacking first word: " << buf_slot[0] << ", second: " << buf_slot[1] << endl;
        unsigned short bword1 = buf_slot[0] & 0xFFFF;
        unsigned short bword2 = (buf_slot[0] >> 16) & 0xFFFF;
        unsigned short bword3 = buf_slot[1] & 0xFFFF;
        unsigned short bword4 = (buf_slot[1] >> 16) & 0xFFFF;

        cout << "unpacking " << bword1 << ", " << bword2 << ", " << bword3 << ", " << bword4 << endl;

        unsigned short channel = bword1 & 0x7F;
        unsigned short axis = (bword1 >> 7) & 1;
        //lane is the slot in the crate
        unsigned short lane = (bword1 >> 8) & 0x1F;
        unsigned short ctime = bword2 & 0xFFFF; //full bword
        unsigned short tdc = bword3 & 0x7FF;
        unsigned short charge = bword4 & 0xFFF;
        cout << "copper: " << j << " finesse: " << finesse_num << ", ";
        cout << "Unpacker channel: " << channel << ", axi: " << axis << " lane: " << lane << " ctime: " << ctime << " tdc: " << tdc << " charge: " << charge << endl;

        //j should give the copper board. We add 1 because the xml file is one based
        int electId = electCooToInt(j + 1, finesse_num + 1, lane, axis);
        if (m_electIdToModuleId.find(electId) == m_electIdToModuleId.end()) {
          char buffer[200];
          sprintf(buffer, "could not find copperid %d, finesse %d, lane %d, axis %d in mapping\n", j + 1, finesse_num + 1, lane, axis);
          B2INFO(buffer);
          continue;
        }
        int moduleId = m_electIdToModuleId[electId];
        cout << " electid: " << electId << " module: " << moduleId << endl;
        //still have to add the channel and axis
        int layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
        //plane should already be set
        //moduleId counts are zero based

        //only channel and inrpc flag is not set yet

        if (layer > 1)
          moduleId |= BKLM_INRPC_MASK;
        moduleId |= ((channel - 1) << BKLM_STRIP_BIT) | ((channel - 1) << BKLM_MAXSTRIP_BIT);

        //(j,finesse_num,lane,channel,axis);
        BKLMDigit digit(moduleId, ctime, tdc, charge);
        //  Digit.setModuleID();
        bklmDigits.appendNew(digit);

        cout << "from digit:sector " << digit.getSector() << " layer: " << digit.getLayer() << " strip: " << digit.getStrip() << ", " << " isphi? " << digit.isPhiReadout() << " fwd? " << digit.isForward() << endl;

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
///
void BKLMUnpackerModule::endRun()
{

}


void BKLMUnpackerModule::terminate()
{


}
