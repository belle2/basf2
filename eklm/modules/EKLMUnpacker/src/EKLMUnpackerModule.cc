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
#include <klm/rawdata/RawData.h>
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
  addParam("WriteWrongHits", m_WriteWrongHits,
           "Record wrong hits (e.g. for debugging).", false);
  addParam("IgnoreWrongHits", m_IgnoreWrongHits,
           "Ignore wrong hits (i.e. no B2ERROR).", false);
  addParam("IgnoreStrip0", m_IgnoreStrip0,
           "Ignore hits with strip = 0 (normally expected for certain firmware "
           "versions).", true);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMUnpackerModule::~EKLMUnpackerModule()
{
}

void EKLMUnpackerModule::initialize()
{
  m_RawKLMs.isRequired();
  m_Digits.registerInDataStore(m_outputDigitsName);
  m_DigitEventInfos.registerInDataStore();
  m_Digits.registerRelationTo(m_DigitEventInfos);
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
  bool correctHit;
  int endcap, layer, sector, strip = 0, stripGlobal;
  int laneNumber;
  int nBlocks;
  uint16_t dataWords[4];
  const int* sectorGlobal;
  EKLMDataConcentratorLane lane;
  EKLMDigit* eklmDigit;
  const EKLMChannelData* channelData;
  if (m_PrintData)
    printf("  w1   w2   w3   w4 e la s p st\n");
  for (int i = 0; i < m_RawKLMs.getEntries(); i++) {
    if (m_RawKLMs[i]->GetNumEvents() != 1) {
      B2ERROR("RawKLM a wrong number of entries (should be 1)."
              << LogVar("RawKLM index", i)
              << LogVar("Number of entries", m_RawKLMs[i]->GetNumEvents()));
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
        KLMDigitEventInfo* klmDigitEventInfo =
          m_DigitEventInfos.appendNew(m_RawKLMs[i], j);
        int triggerCTime = m_RawKLMs[i]->GetTTCtime(j) & 0xFFFF;
        klmDigitEventInfo->setTriggerCTime(triggerCTime);
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = m_RawKLMs[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2ERROR("Incorrect number of data words."
                  << LogVar("Number of data words", numDetNwords));
          continue;
        }
        if (m_CheckCalibration) {
          std::map<int, int> blockLanes;
          std::map<int, int>::iterator it;
          /* cppcheck-suppress variableScope */
          uint16_t blockData[15];
          if (numHits % 75 != 0) {
            B2ERROR("The number of hits in the calibration mode is not a "
                    "multiple of 75." << LogVar("Number of hits", numHits));
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
                std::string errorMessage1, errorMessage2;
                for (it = blockLanes.begin(); it != blockLanes.end(); ++it) {
                  errorMessage1 += (std::string("Lane ") +
                                    std::to_string(it->first) + ": " +
                                    std::to_string(it->second) + " time(s).\n");
                }
                for (i2 = 0; i2 < 15; i2++) {
                  snprintf(buf, 1024, "%04x", blockData[i2]);
                  errorMessage2 += buf;
                  if (i2 < 14)
                    errorMessage2 += " ";
                }
                B2ERROR("Corrupted data block found."
                        << LogVar("Lane numbers", errorMessage1)
                        << LogVar("All first data words", errorMessage2));
              }
            }
          }
        }
        // in the last word there is the user word (from DCs)
        int userWord = (buf_slot[numDetNwords - 1] >> 16) & 0xFFFF;
        klmDigitEventInfo->setUserWord(userWord);
        for (int iHit = 0; iHit < numHits; iHit++) {
          KLM::RawData raw;
          KLM::unpackRawData(&buf_slot[iHit * hitLength], &raw,
                             nullptr, nullptr, false);
          /**
           * The possible values of the strip number in the raw data are
           * from 0 to 127, while the actual range of strip numbers is from
           * 1 to 75. A check is required. The unpacker continues to work
           * with B2ERROR because otherwise debugging is not possible.
           */
          correctHit = m_ElementNumbers->checkStrip(raw.channel, false);
          if (!correctHit) {
            if (!(m_IgnoreWrongHits ||
                  (raw.channel == 0 && m_IgnoreStrip0))) {
              B2ERROR("Incorrect strip number in raw data."
                      << LogVar("Strip number", raw.channel));
            }
            if (!m_WriteWrongHits)
              continue;
            strip = raw.channel;
          } else {
            strip = m_ElementNumbers->getStripSoftwareByFirmware(
                      raw.channel);
          }
          uint16_t plane = raw.axis + 1;
          /*
           * The possible values of the plane number in the raw data are from
           * 1 to 2. The range is the same as in the detector geometry.
           * Consequently, a check of the plane number is useless: it is
           * always correct.
           */
          lane.setLane(raw.lane);
          sectorGlobal = m_ElectronicsMap->getSectorByLane(&lane);
          if (sectorGlobal == nullptr) {
            if (!m_IgnoreWrongHits) {
              B2ERROR("Lane does not exist in the EKLM electronics map."
                      << LogVar("Copper", lane.getCopper())
                      << LogVar("Data concentrator", lane.getDataConcentrator())
                      << LogVar("Lane", lane.getLane()));
            }
            if (!m_WriteWrongHits)
              continue;
            endcap = 0;
            layer = 0;
            sector = 0;
            correctHit = false;
          } else {
            m_ElementNumbers->sectorNumberToElementNumbers(
              *sectorGlobal, &endcap, &layer, &sector);
          }
          if (m_PrintData) {
            printf("%04x %04x %04x %04x %1d %2d %1d %1d %2d\n",
                   dataWords[0], dataWords[1], dataWords[2], dataWords[3],
                   endcap, layer, sector, plane, strip);
          }
          eklmDigit = m_Digits.appendNew();
          eklmDigit->addRelationTo(klmDigitEventInfo);
          eklmDigit->setCTime(raw.ctime);
          eklmDigit->setTDC(raw.tdc);
          eklmDigit->setTime(
            m_TimeConversion->getTime(raw.ctime, raw.tdc,
                                      triggerCTime, true));
          eklmDigit->setEndcap(endcap);
          eklmDigit->setLayer(layer);
          eklmDigit->setSector(sector);
          eklmDigit->setPlane(plane);
          eklmDigit->setStrip(strip);
          eklmDigit->setCharge(raw.charge);
          if (correctHit) {
            stripGlobal = m_ElementNumbers->stripNumber(
                            endcap, layer, sector, plane, strip);
            channelData = m_Channels->getChannelData(stripGlobal);
            if (channelData == nullptr)
              B2FATAL("Incomplete EKLM channel data.");
            if (raw.charge < channelData->getThreshold())
              eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
            else
              eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
          }
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

