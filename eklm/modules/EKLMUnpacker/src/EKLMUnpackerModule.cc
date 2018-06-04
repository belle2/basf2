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
#include <eklm/modules/EKLMUnpacker/EKLMUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(EKLMUnpacker)

EKLMUnpackerModule::EKLMUnpackerModule() : Module()
{
  setDescription("EKLM unpacker (creates EKLMDigit from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputDigitsName", m_outputDigitsName,
           "Name of EKLMDigit store array", string(""));
  addParam("PrintData", m_PrintData, "Print data.", false);
  addParam("CheckCalibration", m_CheckCalibration,
           "Check calibration-mode data.", false);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMUnpackerModule::~EKLMUnpackerModule()
{
}

void EKLMUnpackerModule::initialize()
{
  m_RawKLMs.isRequired();
  m_Digits.registerInDataStore(m_outputDigitsName);
}

void EKLMUnpackerModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
}

void EKLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4-byte words. This is needed to find the hits in the
   * detector buffer.
   */
  const int hitLength = 2;
  int i1, i2;
  int endcap, layer, sector, stripGlobal;
  int laneNumber;
  int nBlocks;
  uint16_t dataWords[4];
  const int* sectorGlobal;
  EKLMDataConcentratorLane lane;
  EKLMDigit* eklmDigit;
  EKLMChannelData* channelData;
  if (m_PrintData)
    printf("  w1   w2   w3   w4 e la s p st\n");
  for (int i = 0; i < m_RawKLMs.getEntries(); i++) {
    if (m_RawKLMs[i]->GetNumEvents() != 1) {
      B2ERROR("RawKLM with index " << i << " has " <<
              m_RawKLMs[i]->GetNumEvents() << " entries (should be 1). ");
      continue;
    }
    /*
     * getNumEntries is defined in RawDataBlock.h and gives the
     * numberOfNodes*numberOfEvents. Number of nodes is num copper boards.
     */
    for (int j = 0; j < m_RawKLMs[i]->GetNumEntries(); j++) {
      unsigned int copperId = m_RawKLMs[i]->GetNodeID(j);
      if (copperId < EKLM_ID || copperId > EKLM_ID + 4)
        continue;
      uint16_t copperN = copperId - EKLM_ID;
      lane.setCopper(copperN);
      m_RawKLMs[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = m_RawKLMs[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2ERROR("Incorrect number of data words: " << numDetNwords);
          continue;
        }
        if (m_CheckCalibration) {
          std::map<int, int> blockLanes;
          std::map<int, int>::iterator it;
          uint16_t blockData[15];
          if (numHits % 75 != 0) {
            B2ERROR("The number of hits in the calibration mode (" <<
                    numHits << ") is not a multiple of 75.");
          } else {
            nBlocks = numHits / 15;
            for (i1 = 0; i1 < nBlocks; i1++) {
              blockLanes.clear();
              for (i2 = 0; i2 < 15; i2++) {
                blockData[i2] = (buf_slot[(i1 * 15 + i2) * hitLength + 0]
                                 >> 16) & 0xFFFF;
                laneNumber = (blockData[i2] >> 8) & 0x1F;
                it = blockLanes.find(laneNumber);
                if (it == blockLanes.end())
                  blockLanes.insert(std::pair<int, int>(laneNumber, 1));
                else
                  it->second++;
              }
              if (blockLanes.size() != 1) {
                char buf[1024];
                std::string errorMessage;
                errorMessage = "Corrupted data block found. Lane numbers:\n";
                for (it = blockLanes.begin(); it != blockLanes.end(); ++it) {
                  errorMessage += (std::string("Lane ") +
                                   std::to_string(it->first) + ": " +
                                   std::to_string(it->second) + " time(s).\n");
                }
                errorMessage += "All first data words:\n";
                for (i2 = 0; i2 < 15; i2++) {
                  snprintf(buf, 1024, "%04x", blockData[i2]);
                  errorMessage += buf;
                  if (i2 < 15)
                    errorMessage += " ";
                }
                errorMessage += "\n";
                B2ERROR(errorMessage);
              }
            }
          }
        }
        for (int iHit = 0; iHit < numHits; iHit++) {
          dataWords[0] = (buf_slot[iHit * hitLength + 0] >> 16) & 0xFFFF;
          dataWords[1] =  buf_slot[iHit * hitLength + 0] & 0xFFFF;
          dataWords[2] = (buf_slot[iHit * hitLength + 1] >> 16) & 0xFFFF;
          dataWords[3] =  buf_slot[iHit * hitLength + 1] & 0xFFFF;
          uint16_t strip = dataWords[0] & 0x7F;
          /**
           * The possible values of the strip number in the raw data are
           * from 0 to 127, while the actual range of strip numbers is from
           * 1 to 75. A check is required.
           */
          if (!m_ElementNumbers->checkStrip(strip, false)) {
            B2ERROR("Incorrect strip number (" << strip << ") in raw data.");
          }
          uint16_t plane = ((dataWords[0] >> 7) & 1) + 1;
          /*
           * The possible values of the plane number in the raw data are from
           * 1 to 2. The range is the same as in the detector geometry.
           * Consequently, a check of the plane number is useless: it is
           * always correct.
           */
          lane.setLane((dataWords[0] >> 8) & 0x1F);
          uint16_t ctime = dataWords[1] & 0xFFFF;
          uint16_t tdc = dataWords[2] & 0x7FF;
          uint16_t charge = dataWords[3] & 0xFFF;
          sectorGlobal = m_ElectronicsMap->getSectorByLane(&lane);
          if (sectorGlobal == NULL) {
            B2ERROR("Lane with copper = " << lane.getCopper() <<
                    ", data concentrator = " << lane.getDataConcentrator() <<
                    ", lane = " << lane.getLane() << " does not exist in the "
                    "EKLM electronics map.");
            continue;
          }
          m_ElementNumbers->sectorNumberToElementNumbers(
            *sectorGlobal, &endcap, &layer, &sector);
          if (m_PrintData) {
            printf("%04x %04x %04x %04x %1d %2d %1d %1d %2d\n",
                   dataWords[0], dataWords[1], dataWords[2], dataWords[3],
                   endcap, layer, sector, plane, strip);
          }
          stripGlobal = m_ElementNumbers->stripNumber(
                          endcap, layer, sector, plane, strip);
          channelData = m_Channels->getChannelData(stripGlobal);
          if (channelData == NULL)
            B2FATAL("Incomplete EKLM channel data.");
          eklmDigit = m_Digits.appendNew();
          eklmDigit->setCTime(ctime);
          eklmDigit->setTDC(tdc);
          eklmDigit->setTime(m_TimeConversion->getTimeByTDC(tdc));
          eklmDigit->setEndcap(endcap);
          eklmDigit->setLayer(layer);
          eklmDigit->setSector(sector);
          eklmDigit->setPlane(plane);
          eklmDigit->setStrip(strip);
          if (charge < channelData->getThreshold())
            eklmDigit->setFitStatus(EKLM::c_FPGASuccessfulFit);
          else
            eklmDigit->setFitStatus(EKLM::c_FPGANoSignal);
          eklmDigit->setCharge(charge);
        }
      }
    }
  }
}

void EKLMUnpackerModule::endRun()
{
}

void EKLMUnpackerModule::terminate()
{
}

