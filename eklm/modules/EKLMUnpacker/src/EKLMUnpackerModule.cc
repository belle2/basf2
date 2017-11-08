/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cstdint>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/modules/EKLMUnpacker/EKLMUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawKLM.h>

using namespace std;
using namespace Belle2;

REG_MODULE(EKLMUnpacker)

EKLMUnpackerModule::EKLMUnpackerModule() : Module()
{
  setDescription("EKLM unpacker (creates EKLMDigit from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputDigitsName", m_outputDigitsName,
           "Name of EKLMDigit store array", string("EKLMDigits"));
  m_GeoDat = NULL;
}

EKLMUnpackerModule::~EKLMUnpackerModule()
{
}

void EKLMUnpackerModule::initialize()
{
  StoreArray<EKLMDigit>eklmDigits(m_outputDigitsName);
  eklmDigits.registerInDataStore();
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void EKLMUnpackerModule::beginRun()
{
}

void EKLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4 byte words. This is needed find the hits in the
   * detector buffer
   */
  const int hitLength = 2;
  int endcap, layer, sector;
  const int* sectorGlobal;
  EKLMDataConcentratorLane lane;
  StoreArray<RawKLM> rawKLM;
  StoreArray<EKLMDigit> eklmDigits(m_outputDigitsName);
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  B2DEBUG(1, "Unpacker has have " << rawKLM.getEntries() << " entries");
  for (int i = 0; i < rawKLM.getEntries(); i++) {
    if (rawKLM[i]->GetNumEvents() != 1) {
      B2DEBUG(1, "rawKLM index " << i << " has more than one entry: " <<
              rawKLM[i]->GetNumEvents());
      continue;
    }
    B2DEBUG(1, "num events in buffer: " << rawKLM[i]->GetNumEvents() <<
            " number of nodes (copper boards) " << rawKLM[i]->GetNumNodes());
    /*
     * getNumEntries is defined in RawDataBlock.h and gives the
     * numberOfNodes*numberOfEvents. Number of nodes is num copper boards.
     */
    for (int j = 0; j < rawKLM[i]->GetNumEntries(); j++) {
      unsigned int copperId = rawKLM[i]->GetNodeID(j);
      B2DEBUG(1, "Opening data package from the COPPER " << copperId);
      if (copperId < EKLM_ID || copperId > EKLM_ID + 4)
        continue;
      uint16_t copperN = copperId - EKLM_ID;
      lane.setCopper(copperN);
      rawKLM[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        //addendum: There is always an additional word (count) in the end
        int numDetNwords = rawKLM[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = rawKLM[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2DEBUG(1, "word count incorrect: " << numDetNwords);
          continue;
        }
        if (numDetNwords > 0) B2DEBUG(1, "Opening finesse " << finesse_num <<
                                        " number of words is " << numDetNwords <<
                                        ", counter is " << (buf_slot[numDetNwords - 1] & 0xFFFF));


        for (int iHit = 0; iHit < numHits; iHit++) {
          uint16_t bword2 =  buf_slot[iHit * hitLength + 0] & 0xFFFF;
          uint16_t bword1 = (buf_slot[iHit * hitLength + 0] >> 16) & 0xFFFF;
          uint16_t bword4 =  buf_slot[iHit * hitLength + 1] & 0xFFFF;
          uint16_t bword3 = (buf_slot[iHit * hitLength + 1] >> 16) & 0xFFFF;
          B2DEBUG(1, "unpacking " << bword1 << ", " << bword2 << ", " <<
                  bword3 << ", " << bword4);
          uint16_t strip  =   bword1 & 0x7F;
          uint16_t plane  = ((bword1 >> 7) & 1) + 1;
          lane.setLane((bword1 >> 8) & 0x1F);
//        uint16_t ctime  =   bword2 & 0xFFFF; //full bword      unused yet
          uint16_t tdc    =   bword3 & 0x7FF;
          uint16_t charge =   bword4 & 0xFFFF;  // !!! THERE IS 15 BITS NOW!!!
          // !!! SHOULD BE 12 BITS !!!

          sectorGlobal = m_ElectronicsMap->getSectorByLane(&lane);
          if (sectorGlobal == NULL)
            B2FATAL("Incomplete EKLM electronics map.");
          m_GeoDat->sectorNumberToElementNumbers(*sectorGlobal,
                                                 &endcap, &layer, &sector);
          EKLMDigit* idigit = eklmDigits.appendNew();
          idigit->setTime(tdc);
          idigit->setEndcap(endcap);
          idigit->setLayer(layer);
          idigit->setSector(sector);
          idigit->setPlane(plane);
          idigit->setStrip(strip);
          idigit->isGood(true);
          idigit->setCharge(charge);
          B2DEBUG(1, " Digit: endcap=" << idigit->getEndcap() <<
                  " layer=" << idigit->getLayer() <<
                  " strip=" << idigit->getSector() <<
                  " plane=" << idigit->getPlane() <<
                  " strip=" << idigit->getStrip() <<
                  " charge=" << idigit->getEDep()  <<
                  " time=" << idigit->getTime());
        }
      } //finesse boards
    }  //copper boards
  }   // events... There should be only 1...
}

void EKLMUnpackerModule::endRun()
{
}

void EKLMUnpackerModule::terminate()
{
}

