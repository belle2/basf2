/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                            *
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
//#include <rawdata/dataobjects/RawKLM.h>
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
  addParam("SciThreshold", m_scintThreshold, "scintillator strip hits with charge lower this value will be marked as bad",
           double(140.0));
  addParam("loadThresholdFromDB", m_loadThresholdFromDB, "load threshold from DataBase (true) or not (false)", true);
  addParam("loadMapFromDB", m_loadMapFromDB, "whether load electronic map from DataBase", true);
  addParam("rawdata", m_rawdata, "is this real rawdata (true) or MC data (false)", false);
  addParam("enableDebugFile", m_enableDebugFile, "if true, enable the production of output root file for debbuging studies", false);
  addParam("nameDebugFile", m_nameDebugFile, "if enabled, name of the output root file", string("allhits.root"));
}


BKLMUnpackerModule::~BKLMUnpackerModule()
{

}

void BKLMUnpackerModule::initialize()
{
  rawKLM.isRequired();
  //StoreArray<BKLMDigit>bklmDigits(m_outputDigitsName);
  bklmDigits.registerInDataStore(m_outputDigitsName);
  if (m_loadMapFromDB)
    loadMapFromDB();
  if (m_enableDebugFile)
    B2INFO("BKLMUnpackerModule:: debugging mode enabled: the additional file " << m_nameDebugFile.c_str() << " will be created");
  bookDebugFile();
}

void BKLMUnpackerModule::loadMapFromDB()
{
  DBArray<BKLMElectronicMapping> elements;
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
    int elecId = electCooToInt(copperId - BKLM_ID, slotId - 1 , laneId, axisId, channelId);
    int moduleId = 0;
    B2DEBUG(1, "reading Data Base...");
    moduleId = (isForward ? BKLM_END_MASK : 0)
               | ((sector - 1) << BKLM_SECTOR_BIT)
               | ((layer - 1) << BKLM_LAYER_BIT)
               | ((plane) << BKLM_PLANE_BIT)
               | ((stripId - 1) << BKLM_STRIP_BIT);
    m_electIdToModuleId[elecId] = moduleId;
    B2DEBUG(1, " electId: " << elecId << " modId: " << moduleId);
  }

}


void BKLMUnpackerModule::beginRun()
{
  if (m_loadMapFromDB)
    loadMapFromDB();

  if (m_loadThresholdFromDB) {
    m_scintADCOffset = m_ADCParams->getADCOffset();
    m_scintThreshold = m_ADCParams->getADCThreshold();
  }

  if (m_enableDebugFile)
    m_iEvent = 0;
}


void BKLMUnpackerModule::event()
{
  //
  // Store informations for debugging root file
  //
  if (m_enableDebugFile) {
    StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
    m_event = eventMetaData->getEvent();
    m_run = eventMetaData->getRun();
    m_exp = eventMetaData->getExperiment();
    m_time = eventMetaData->getTime();
    m_iEvent++;
    m_nTotal = 0;

    m_windowStart = 0;
    //int cntTotal = 0;
    m_nSci = 0;
    m_nRPC = 0;
    m_nBF2L2 = 0;
    m_nLane0 = 0;
    //int cntBF2L1_z_pass=0, cntBF2L2_z_pass=0;
    //int cntBF2L1_phi_pass=0,cntBF2L2_phi_pass=0;
    //int cnthit[2][8][15][2] = {0};
    m_firstHitCtime = 0;
    m_sector = -1;
    m_layer = -1;
    m_channel = -1;

    for (int ii = 0; ii < 2; ii++) {
      for (int jj = 0; jj < 8; jj++) {
        m_cntsRPC[ii][jj] = 0;
        m_minctime[ii][jj] = 65536;
        m_maxctime[ii][jj] = -1;
        for (int kk = 0; kk < 1000; kk++) {
          if (kk < 15)
            m_nDigit[ii][jj][kk] = 0;
          // recorded ctime of RPC hits on each sector
          m_ctimeOfRPC[ii][jj][kk] = 0;
        }

      }
    }

    //int tempTriggerCTime = rawKLM[0]->GetTTCtime(0);
    if (m_iEvent == 1)
      m_interval = -1;
    else
      // time in us
      m_interval = (rawKLM[0]->GetTTCtime(0) - m_triggerTime) * 8.0 / 1000;
    m_triggerTime = rawKLM[0]->GetTTCtime(0);
  }

  //StoreArray<BKLMDigit> bklmDigits(m_outputDigitsName);
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

      //
      // Store informations for debugging root file
      //
      if (m_enableDebugFile) {
        m_triggerctime = rawKLM[i]->GetTTCtime(j) & 0xFFFF;
        m_trigUTime = rawKLM[i]->GetTTUtime(j) & 0xFFFF;
        m_windowStart = rawKLM[i]->GetTrailerChksum(j);
      }

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
          //unsigned short flag = (bword1 >> 14);
          unsigned short ctime = bword2 & 0xFFFF; //full bword
          unsigned short tdc = bword3 & 0x7FF;
          unsigned short charge = bword4 & 0xFFF;

          //if ((1 == layer || 2 == layer)  && fabs(charge - m_scintADCOffset) < m_scintThreshold) continue;

          B2DEBUG(1, "copper: " << copperId << " finesse: " << finesse_num);
          //  B2DEBUG(1, "Unpacker channel: " << channel << ", axi: " << axis << " lane: " << lane << " ctime: " << ctime << " tdc: " << tdc << " charge: " << charge)
          //  cout << "Unpacker channel: " << channel << ", axi: " << axis << " lane: " << lane << " ctime: " << ctime << " tdc: " << tdc <<
          //  " charge: " << charge << endl;

          int electId = electCooToInt(copperId - BKLM_ID, finesse_num , lane, axis, channel);
          int moduleId = 0;
          bool outRange = false;
          if (m_electIdToModuleId.find(electId) == m_electIdToModuleId.end()) {
            if (!m_useDefaultModuleId) {
              B2DEBUG(1, "could not find copperid " << copperId << ", finesse " << finesse_num + 1 << ", lane " << lane << ", axis " << axis <<
                      " in mapping");
              continue;
            } else {
              moduleId = getDefaultModuleId(copperId, finesse_num, lane, axis, channel, outRange);
            }
          } else { //found moduleId in the mapping
            moduleId = m_electIdToModuleId[electId];
            B2DEBUG(1, " electid: " << electId << " module: " << moduleId);

            //only channel and inrpc flag is not set yet
          }
          //moduleId counts are zero based
          int layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
          //int sector = (moduleId & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT;
          //int isForward = (moduleId & BKLM_END_MASK) >> BKLM_END_BIT;
          //int plane = (moduleId & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;
          channel = (moduleId & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT;

          if (layer > 14) { B2DEBUG(1, "BKLMUnpackerModule:: strange that the layer number is larger than 14 " << layer); continue;}

          if (outRange) {
            std::string message = "BKLMUnpackerModule:: channel number is out of range ";
            m_rejected[message] += 1;
            m_rejectedCount++;
            if (m_rejectedCount < 10) {
              B2INFO("BKLMUnpackerModule:: channel number is out of range " << channel);
            } else if (m_rejectedCount == 10) {
              B2INFO("BKLMUnpackerModule:: channel number is out of range "
                     << "(message will be suppressed now)");
            }
            continue;
          }

          //still have to add the channel and axis
          if (layer > 1) moduleId |= BKLM_INRPC_MASK;
          //moduleId |= (((channel - 1) & BKLM_STRIP_MASK) << BKLM_STRIP_BIT) | (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);
          moduleId |= (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);

          BKLMDigit bklmDigit(moduleId, ctime, tdc, m_scintADCOffset - charge);
          if (layer < 2 && ((m_scintADCOffset - charge) > m_scintThreshold))  bklmDigit.isAboveThreshold(true);

          B2DEBUG(1, "BKLMUnpackerModule:: digit after Unpacker: sector: " << bklmDigit.getSector() << " isforward: " << bklmDigit.isForward()
                  << " layer: " << bklmDigit.getLayer() << " isPhi: " << bklmDigit.isPhiReadout());
          B2DEBUG(1, "BKLMUnpackerModule:: charge " << bklmDigit.getCharge() << " tdc" << bklmDigit.getTime() << " ctime " << ctime <<
                  " isAboveThreshold " << bklmDigit.isAboveThreshold() << " isRPC " << bklmDigit.inRPC() << " moduleId " << bklmDigit.getModuleID());

          bklmDigits.appendNew(bklmDigit);

          B2DEBUG(1, "from bklmDigit: sector " << bklmDigit.getSector() << " layer: " << bklmDigit.getLayer() << " strip: " <<
                  bklmDigit.getStrip() << ", " <<
                  " isPhi? " << bklmDigit.isPhiReadout() << " isForward? " << bklmDigit.isForward());

          //
          // Store informations for debugging root file
          //
          if (m_enableDebugFile) {
            m_trigger = m_triggerctime;

            m_format = 0;
            if ((bword1 & 0xFF) == 0x7F)
              m_format = 1;

            //unsigned short window = (bword2 >> 7) & 0x1FF;
            //unsigned short asic = (bword2) & 0x7F;
            m_window_2 = (bword2 >> 7) & 0x1FF;
            m_asic_2 = (bword2) & 0x7F;
            if (bword2  == 0xCAFE or bword2 == 0xDAFE) {
              m_window_2 = 520;
              m_asic_2 = 0;
            }

            m_window_3 = (bword3 >> 7) & 0x1FF;
            m_asic_3 = (bword3) & 0x7F;
            if (bword3  == 0xCAFE or bword3 == 0xDAFE) {
              m_window_3 = 520;
              m_asic_3 = 0;
            }

            m_window_4 = (bword4 >> 7) & 0x1FF;
            m_asic_4 = (bword4) & 0x7F;
            if (bword4  == 0xCAFE or bword4 == 0xDAFE) {
              m_window_4 = 520;
              m_asic_4 = 0;
            }

            char ss[200];
            if (m_format == 0)
              m_nTotal++;
            if (m_format == 1)
              sprintf(ss, "%04X, %04X, %04X, %04X, %d\t%d\t%d\t%d\t%d\t%d", bword1, bword2, bword3, bword4, m_window_2,
                      m_asic_2, m_window_3, m_asic_3 , m_window_4, m_asic_4);

            m_unused_word3 = (bword3 >> 11); // & 0x1F;
            m_unused_word4 = (bword4 >> 12); // & 0xF;

            /*if(iHit==0) {
              if(i==0) cout<<"cpr 7001 ";
              if(i==1) cout<<"cpr 7002 ";
              if(i==2) cout<<"cpr 7003 ";
              if(i==3) cout<<"cpr 7004 ";
              if(finesse_num==0) cout<<" -a "<<endl;
              if(finesse_num==1) cout<<" -b "<<endl;
              if(finesse_num==2) cout<<" -c "<<endl;
              if(finesse_num==3) cout<<" -d "<<endl;
            }*/

            int debugLayer = lane;
            m_flag = (bword1 >> 13);
            if (m_flag == 2)
              debugLayer = lane - 5;
            int plane = axis;
            if (debugLayer < 3) {
              if (axis == 0)
                plane = 1;
              else if (axis == 1)
                plane = 0;
              else
                B2WARNING("BKLMUnpackerModule:: axis bit of scintillator is abnormal " << axis);
            }
            if (m_flag == 4)
              m_nSci++;
            if (lane == 0)
              m_nLane0++;
            if (lane > 2)
              m_nRPC++;

            m_copperId = copperId;
            m_finesse = finesse_num;
            m_lane = lane;
            m_layer = debugLayer;
            m_tdc = tdc;
            m_tdcnano = tdc / (127.216e6 * 8) * 1e9;
            m_rawchannel = channel;
            m_axis = plane;
            m_ctime = ctime;
            m_ctimeDiff = ctime - m_trigger;
            m_charge = charge - m_scintADCOffset;

            int sector = (moduleId & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT;
            m_sector = sector + 1;
            int isForward = (moduleId & BKLM_END_MASK) >> BKLM_END_BIT;
            m_isForward = isForward;
            //int plane = (moduleId & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;
            //m_layer=layer+1;
            m_channel = channel;
            m_outofrange = (outRange ? 1 : 0);
            m_hitTree->Fill();

            if (m_format == 1)
              continue;

            if (isForward == 1 && sector == 2 && debugLayer == 1)
              m_nBF2L2++;
            //cnthit[isForward][sector][debugLayer][axis]++;
            int tmpindex = m_cntsRPC[isForward][sector];
            m_nDigit[isForward][sector][debugLayer]++;
            if (debugLayer > 1) {
              m_ctimeOfRPC[isForward][sector][tmpindex] = ctime;
              m_cntsRPC[isForward][sector]++;
            }
          }

        } //iHit for cycle

      } //finesse for cycle

    } //copper for cycle

  } //events for cycle: should be only 1...

  if (m_enableDebugFile) {
    for (int ii = 0; ii < 2; ii++) {
      for (int jj = 0; jj < 8; jj++) {
        m_nRPCOnSector[ii][jj] = m_cntsRPC[ii][jj];
        for (int ihit = 0; ihit < m_cntsRPC[ii][jj]; ihit++) {
          //m_ctimeOfRPC[ii][jj][ihit] = ctimeOfRPC[ii][jj][ihit];
          if (m_ctimeOfRPC[ii][jj][ihit] < m_minctime[ii][jj])
            m_minctime[ii][jj] = m_ctimeOfRPC[ii][jj][ihit];
          if (m_ctimeOfRPC[ii][jj][ihit] > m_maxctime[ii][jj])
            m_maxctime[ii][jj] = m_ctimeOfRPC[ii][jj][ihit];
        }
        m_ctimeCount[ii][jj] = m_maxctime[ii][jj] - m_minctime[ii][jj];
        //for (int ly = 0; ly < 15; ly++) {
        //  m_ndigit[ii][jj][ly] = cntdigit[ii][jj][ly];
        //}
      }
    }
    m_eventTree->Fill();//event level
  }
}


int BKLMUnpackerModule::electCooToInt(int copper, int finesse, int lane, int axis, int channel)
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
  for (const auto& message : m_rejected) {
    B2INFO(message.first << "(occured " << message.second << " times)");
  }

  if (m_enableDebugFile) {
    m_file->cd();
    m_hitTree->Write();
    m_eventTree->Write();
  }
}


int BKLMUnpackerModule::getDefaultModuleId(int copperId, int finesse, int lane, int axis, int channel, bool& outOfRange)
{

  int sector = 0;
  int isForward = 0;
  int layer = 0;
  int plane = 0;
  int stripId = 0;
  if (copperId == 117440513 || copperId == 117440514) isForward = 1;
  if (copperId == 117440515 || copperId == 117440516) isForward = 0;
  if (copperId == 117440513 || copperId == 117440515) sector = finesse + 3;
  if (copperId == 117440514 || copperId == 117440516) sector = (finesse + 7 > 8) ? finesse - 1 : finesse + 7;

  if (lane > 2) layer = lane - 5;
  else layer = lane;
  if (lane > 2) plane = axis;
  else { if (axis == 0) plane = 1; else plane = 0; }

  stripId =  getChannel(isForward, sector, layer, plane, channel);
  stripId =  flipChannel(isForward, sector, layer, plane, stripId, outOfRange);
  //attention: moduleId counts are zero based
  int moduleId = (isForward ? BKLM_END_MASK : 0)
                 | (uint(sector - 1) << BKLM_SECTOR_BIT)
                 | ((layer - 1) << BKLM_LAYER_BIT)
                 | ((plane) << BKLM_PLANE_BIT)
                 | ((stripId - 1) << BKLM_STRIP_BIT);

  return moduleId;

}

unsigned short BKLMUnpackerModule::getChannel(int isForward, int sector, int layer, int axis, unsigned short channel)
{

  if (axis == 0 && layer < 3) { //scintillator z
    if (isForward == 0 && sector == 3) { //sector #3 is the top sector, backward sector#3 is the chimney sector.
      if (layer == 1) {
        if (channel > 8 && channel < 16) channel = 0;
        else if (channel > 0 && channel < 9) channel = 9 - channel;
        else if (channel > 30 && channel < 46) channel = 54 - channel;
        else if (channel > 15 && channel < 31) channel = 54 - channel;
      } else if (layer == 2) {
        if (channel == 16) channel = 0;
        else if (channel > 9 && channel < 16) channel = 0;
        else if (channel > 0 && channel < 10) channel = 10 - channel;
        else if (channel > 16 && channel < 31) channel = 40 - channel;
        else if (channel > 30 && channel < 46) channel = 69 - channel;
      }
    } else { //all sectors except backward sector#3
      if (channel > 0 && channel < 16) channel = 15 - channel + 1;
      else if (channel > 15 && channel < 31) channel = 45 - channel + 1;
      else if (channel > 30 && channel < 46) channel = 75 - channel + 1;
      else if (channel > 45 && channel < 61) channel = 105 - channel + 1;
    }
  }

  if (layer == 1) {
    if (axis == 1) { //phi strips
      if (channel > 0 && channel < 5) channel = 0;
      //else channel = channel - 4;
      if (channel > 4 && channel < 42) channel = channel - 4;
      //if(channel>41) channel=0;
      if (channel > 41) channel = channel - 4;
    }

    if (axis == 0 && !(isForward == 0 && sector == 3)) { //z strips
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 0 && channel < 7) channel = 0;
      if (channel > 6 && channel < 61) channel = channel - 6;
      if (channel > 60) channel = channel - 6;
    }
  }
  if (layer == 2) {
    if (axis == 1) { //phi
      if (channel > 0 && channel < 3) channel = 0;
      if (channel > 2 && channel < 45) channel = channel - 2;
      if (channel > 44) channel = channel - 2;;
    }
    if (axis == 0 && !(isForward == 0 && sector == 3)) {
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 0 && channel < 7) channel = 0;
      if (channel > 6 && channel < 61) channel = channel - 6;
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
    if (layer < 3) MaxiChannel = 38;
    if (layer > 2) MaxiChannel = 34;
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
  if (!(dontFlip && layer > 2 && plane == 1) && (channel > 0 && channel < (MaxiChannel + 1))) channel = MaxiChannel - channel + 1;

  if (channel < 1 || channel > MaxiChannel)  isOutRange = true;

  return channel;
}

void BKLMUnpackerModule::bookDebugFile()
{
  m_file = new TFile(m_nameDebugFile.c_str(), "RECREATE");

  //hit-based tree, filled hit by hit
  m_hitTree = new TTree("hits", "Hit-based info");
  m_hitTree->Branch("exp", &m_exp, "exp/I");
  m_hitTree->Branch("run", &m_run, "run/I");
  m_hitTree->Branch("event", &m_event, "event/I");
  m_hitTree->Branch("format", &m_format, "format/I");
  m_hitTree->Branch("trigger", &m_trigger, "trigger/I");
  m_hitTree->Branch("window_2", &m_window_2, "window_2/I");
  m_hitTree->Branch("asic_2", &m_asic_2, "asic_2/I");
  m_hitTree->Branch("window_3", &m_window_3, "window_3/I");
  m_hitTree->Branch("asic_3", &m_asic_3, "asic_3/I");
  m_hitTree->Branch("window_4", &m_window_4, "window_4/I");
  m_hitTree->Branch("asic_4", &m_asic_4, "asic_4/I");
  m_hitTree->Branch("layer", &m_layer, "layer/I");
  m_hitTree->Branch("flag", &m_flag, "flag/I");
  m_hitTree->Branch("sector", &m_sector, "sector/I");
  m_hitTree->Branch("time", &m_time, "time/I");
  m_hitTree->Branch("copperId", &m_copperId, "copperId/I");
  m_hitTree->Branch("finesse", &m_finesse, "finesse/I");
  m_hitTree->Branch("lane", &m_lane, "lane/I");
  m_hitTree->Branch("isForward", &m_isForward, "isForward/I");
  m_hitTree->Branch("rawChannel", &m_rawchannel, "rawChannel/I");
  m_hitTree->Branch("channel", &m_channel, "channel/I");
  m_hitTree->Branch("outOfRange", &m_outofrange, "outOfRange/I");
  m_hitTree->Branch("axis", &m_axis, "axis/I");
  m_hitTree->Branch("ctime", &m_ctime, "ctime/I");
  m_hitTree->Branch("unused_word3", &m_unused_word3, "unused_word3/I");
  m_hitTree->Branch("unused_word4", &m_unused_word4, "unused_word4/I");
  //m_hitTree->Branch("ctimecheck",&m_ctime_check,"m_ctime_check/I");
  m_hitTree->Branch("tdc", &m_tdc, "tdc/I");
  m_hitTree->Branch("tdc_nano", &m_tdcnano, "tdc_nano/F");
  //m_hitTree->Branch("ctimenano",&m_ctimenano,"m_ctimenano/F");
  m_hitTree->Branch("charge", &m_charge, "charge/I");
  m_hitTree->Branch("ctime_diff", &m_ctimeDiff, "ctime_diff/I");
  m_hitTree->Branch("triggerTime", &m_triggerctime, "triggerTime/I");
  //m_hitTree->Branch("ctimedelta",&m_hctimedelta,"m_hctimedelta/I");

  //event-based tree, filled event by event
  m_eventTree = new TTree("events", "Event-based info");
  m_eventTree->Branch("run", &m_run, "run/I");
  m_eventTree->Branch("event", &m_event, "event/I");
  m_eventTree->Branch("triggerTime", &m_triggerTime, "triggerTime/I");
  m_eventTree->Branch("triggerInterval", &m_interval, "triggerInterval/D");
  m_eventTree->Branch("triggerUTime", &m_trigUTime, "trigerUTime/I");
  m_eventTree->Branch("windowStart", &m_windowStart, "windowStart/I");
  m_eventTree->Branch("nTotal", &m_nTotal, "nTotal/I");
  m_eventTree->Branch("nLane0", &m_nLane0, "nLane0/I");
  m_eventTree->Branch("nSci", &m_nSci, "nSci/I");
  m_eventTree->Branch("nRPC", &m_nRPC, "nRPC/I");
  m_eventTree->Branch("nBF2_L2", &m_nBF2L2, "nBF2_L2/I");
  m_eventTree->Branch("firstHitCtime", &m_firstHitCtime, "firstHitCtime/I");
  //m_eventTree->Branch("nhit",&m_nrpc,"m_nhit[2][8][15][2]/I");
  m_eventTree->Branch("nRPCOnSector", &m_nRPCOnSector, "m_nRPCOnSector[2][8]/I"); //number of rpc hit in each sector
  m_eventTree->Branch("ndigit", &m_nDigit, "nDigit[2][8][15]/I"); //number of digit in each layer
  m_eventTree->Branch("ctimeOfRPC", &m_ctimeOfRPC, "ctimeOfRPC[2][8][1000]/I"); //ctime of rpc on each sector
  //m_eventTree->Branch("ctimecount",&m_ctimecount,"m_ctimecount/I");//difference between the miniCtime and MaxCTIme in eeach event
  m_eventTree->Branch("minctime", &m_minctime, "minctime[2][8]/I");
  m_eventTree->Branch("maxctime", &m_maxctime, "maxctime[2][8]/I");
  m_eventTree->Branch("ctimecount", &m_ctimeCount,
                      "ctimeCount[2][8]/I"); //difference between the miniCtime and MaxCTIme in each sector.

}
