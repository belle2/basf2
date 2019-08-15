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
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/modules/KLMUnpacker/KLMUnpackerModule.h>
#include <klm/rawdata/RawData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(KLMUnpacker)

KLMUnpackerModule::KLMUnpackerModule() : Module(),
  m_triggerCTimeOfPreviousEvent(0)
{
  setDescription("KLM unpacker (creates BKLMDigits and EKLMDigits "
                 "from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputBKLMDigitsName", m_outputBKLMDigitsName,
           "Name of BKLMDigit store array.", string(""));
  addParam("outputEKLMDigitsName", m_outputEKLMDigitsName,
           "Name of EKLMDigit store array.", string(""));
  addParam("WriteWrongHits", m_WriteWrongHits,
           "Record wrong hits (e.g. for debugging).", false);
  addParam("IgnoreWrongHits", m_IgnoreWrongHits,
           "Ignore wrong hits (i.e. no B2ERROR).", false);
  addParam("IgnoreStrip0", m_IgnoreStrip0,
           "Ignore hits with strip = 0 (normally expected for certain firmware "
           "versions).", true);
  addParam("keepEvenPackages", m_keepEvenPackages,
           "Keep packages that have even length normally indicating that "
           "data was corrupted ", false);
  addParam("SciThreshold", m_scintThreshold,
           "Scintillator strip hits with charge lower this value will be "
           "marked as bad.", double(140.0));
  addParam("loadThresholdFromDB", m_loadThresholdFromDB,
           "Load threshold from database (true) or not (false)", true);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMUnpackerModule::~KLMUnpackerModule()
{
}

void KLMUnpackerModule::initialize()
{
  /* Common. */
  m_RawKLMs.isRequired();
  /* BKLM. */
  m_bklmDigits.registerInDataStore(m_outputBKLMDigitsName);
  m_klmDigitRaws.registerInDataStore();
  m_bklmDigitOutOfRanges.registerInDataStore();
  m_DigitEventInfos.registerInDataStore();

  m_bklmDigits.registerRelationTo(m_klmDigitRaws);
  m_bklmDigitOutOfRanges.registerRelationTo(m_klmDigitRaws);
  m_DigitEventInfos.registerRelationTo(m_bklmDigits);
  m_DigitEventInfos.registerRelationTo(m_bklmDigitOutOfRanges);

  /* EKLM. */
  m_eklmDigits.registerInDataStore(m_outputEKLMDigitsName);
  m_DigitEventInfos.registerInDataStore();
  m_eklmDigits.registerRelationTo(m_DigitEventInfos);
}

void KLMUnpackerModule::beginRun()
{
  if (!m_eklmElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  if (m_loadThresholdFromDB)
    m_scintThreshold = m_ADCParams->getADCThreshold();
  m_triggerCTimeOfPreviousEvent = 0;
}

void KLMUnpackerModule::unpackEKLMDigit(
  const int* rawData, EKLMDataConcentratorLane* lane,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  int section, layer, sector, strip = 0;
  KLM::RawData raw;
  KLM::unpackRawData(rawData, &raw, nullptr, nullptr, false);
  if ((raw.triggerBits & 0x10) != 0)
    return;
  /**
   * The possible values of the strip number in the raw data are
   * from 0 to 127, while the actual range of strip numbers is from
   * 1 to 75. A check is required. The unpacker continues to work
   * with B2ERROR because otherwise debugging is not possible.
   */
  bool correctHit = m_ElementNumbers->checkStrip(raw.channel, false);
  if (!correctHit) {
    if (!(m_IgnoreWrongHits ||
          (raw.channel == 0 && m_IgnoreStrip0))) {
      B2ERROR("Incorrect strip number in raw data."
              << LogVar("Strip number", raw.channel));
    }
    if (!m_WriteWrongHits)
      return;
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
  lane->setLane(raw.lane);
  const int* sectorGlobal = m_eklmElectronicsMap->getSectorByLane(lane);
  if (sectorGlobal == nullptr) {
    if (!m_IgnoreWrongHits) {
      B2ERROR("Lane does not exist in the EKLM electronics map."
              << LogVar("Copper", lane->getCopper())
              << LogVar("Data concentrator", lane->getDataConcentrator())
              << LogVar("Lane", lane->getLane()));
    }
    if (!m_WriteWrongHits)
      return;
    section = 0;
    layer = 0;
    sector = 0;
    correctHit = false;
  } else {
    m_ElementNumbers->sectorNumberToElementNumbers(
      *sectorGlobal, &section, &layer, &sector);
  }
  EKLMDigit* eklmDigit = m_eklmDigits.appendNew();
  eklmDigit->addRelationTo(klmDigitEventInfo);
  eklmDigit->setCTime(raw.ctime);
  eklmDigit->setTDC(raw.tdc);
  eklmDigit->setTime(
    m_TimeConversion->getTime(raw.ctime, klmDigitEventInfo->getTriggerCTime()));
  eklmDigit->setSection(section);
  eklmDigit->setLayer(layer);
  eklmDigit->setSector(sector);
  eklmDigit->setPlane(plane);
  eklmDigit->setStrip(strip);
  eklmDigit->setCharge(raw.charge);
  if (correctHit) {
    int stripGlobal = m_ElementNumbers->stripNumber(
                        section, layer, sector, plane, strip);
    const EKLMChannelData* channelData =
      m_Channels->getChannelData(stripGlobal);
    if (channelData == nullptr)
      B2FATAL("Incomplete EKLM channel data.");
    if (raw.charge < channelData->getThreshold())
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
    else
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
  }
}

void KLMUnpackerModule::unpackBKLMDigit(
  const int* rawData, int copper, int hslb,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  KLM::RawData raw;
  KLMDigitRaw* klmDigitRaw;
  KLM::unpackRawData(rawData, &raw, &m_klmDigitRaws, &klmDigitRaw, true);
  const uint16_t* detectorChannel;
  BKLMElectronicsChannel electronicsChannel(
    copper, hslb + 1, raw.lane, raw.axis, raw.channel);
  detectorChannel =
    m_bklmElectronicsMap->getDetectorChannel(&electronicsChannel);
  if (detectorChannel == nullptr) {
    B2DEBUG(20, "KLMUnpackerModule:: could not find in mapping"
            << LogVar("Copper", copper)
            << LogVar("Finesse", hslb + 1)
            << LogVar("Lane", raw.lane)
            << LogVar("Axis", raw.axis));
    if (!m_WriteWrongHits)
      return;
    /* Find channel from the same module. */
    electronicsChannel.setAxis(0);
    /* Phi-plane channels may start from 3 or 5. */
    electronicsChannel.setChannel(5);
    detectorChannel = m_bklmElectronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel != nullptr) {
      // increase by 1 the event-counter of outOfRange-flagged hits
      klmDigitEventInfo->increaseOutOfRangeHits();

      // store the digit in the appropriate dataobject
      int moduleId = *detectorChannel;
      BKLMDigitOutOfRange* bklmDigitOutOfRange =
        m_bklmDigitOutOfRanges.appendNew(
          moduleId, raw.ctime, raw.tdc, raw.charge);
      bklmDigitOutOfRange->addRelationTo(klmDigitRaw);
      klmDigitEventInfo->addRelationTo(bklmDigitOutOfRange);

      std::string message = "channel number is out of range";
      m_rejected[message] += 1;
      m_rejectedCount++;
      B2DEBUG(21, "KLMUnpackerModule:: raw channel number is out of range"
              << LogVar("Channel", raw.channel));

    }
    return;
  }

  // moduleId counts are zero based
  int moduleId = *detectorChannel;
  int layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
  if ((layer < 2) && ((raw.triggerBits & 0x10) != 0))
    return;
  int channel = (moduleId & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT;

  if (layer > 14) {
    B2DEBUG(20, "KLMUnpackerModule:: strange that the layer number is larger than 14 "
            << LogVar("Layer", layer));
    return;
  }

  // still have to add channel and axis to moduleId
  moduleId |= (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);

  BKLMDigit* bklmDigit;
  if (layer > 1) {
    moduleId |= BKLM_INRPC_MASK;
    klmDigitEventInfo->increaseRPCHits();
    // For RPC hits, digitize both the coarse (ctime) and fine (tdc) times relative
    // to the revo9 trigger time rather than the event header's TriggerCTime.
    // For the fine-time (tdc) measurement (11 bits), shift the revo9Trig time by
    // 10 ticks to align the new prompt-time peak with the TriggerCtime-relative peak.
    float triggerTime = klmDigitEventInfo->getRevo9TriggerWord();
    std::pair<int, double> rpcTimes = m_TimeConversion->getTimes(raw.ctime, raw.tdc, triggerTime);
    bklmDigit = m_bklmDigits.appendNew(moduleId, rpcTimes.first, raw.tdc, raw.charge);
    bklmDigit->setTime(rpcTimes.second);
  } else {
    klmDigitEventInfo->increaseSciHits();
    // For scintillator hits, store the ctime relative to the event header's trigger ctime
    bklmDigit = m_bklmDigits.appendNew(moduleId, raw.ctime, raw.tdc, raw.charge);
    bklmDigit->setTime(
      m_TimeConversion->getTime(raw.ctime, klmDigitEventInfo->getTriggerCTime()));
    if (raw.charge < m_scintThreshold)
      bklmDigit->isAboveThreshold(true);
  }
  bklmDigit->addRelationTo(klmDigitRaw);
  klmDigitEventInfo->addRelationTo(bklmDigit);
}

void KLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4-byte words. This is needed to find the hits in the
   * detector buffer.
   */
  const int hitLength = 2;
  EKLMDataConcentratorLane lane;
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
      bool eklmHit = false;
      if ((copperId >= EKLM_ID) && (copperId <= EKLM_ID + 4))
        eklmHit = true;
      else if (!((copperId >= BKLM_ID) && (copperId <= BKLM_ID + 4)))
        continue;
      uint16_t copperN = copperId - EKLM_ID;
      lane.setCopper(copperN);
      m_RawKLMs[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        KLMDigitEventInfo* klmDigitEventInfo =
          m_DigitEventInfos.appendNew(m_RawKLMs[i], j);
        klmDigitEventInfo->setPreviousEventTriggerCTime(
          m_triggerCTimeOfPreviousEvent);
        m_triggerCTimeOfPreviousEvent = klmDigitEventInfo->getTriggerCTime();
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = m_RawKLMs[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2ERROR("Incorrect number of data words."
                  << LogVar("Number of data words", numDetNwords));
          if (!m_keepEvenPackages)
            continue;
        }
        if (numDetNwords > 0) {
          /*
           * In the last word there are the revo9 trigger word
          * and the the user word (both from DCs).
           */
          unsigned int revo9TriggerWord = (buf_slot[numDetNwords - 1] >> 16) & 0xFFFF;
          klmDigitEventInfo->setRevo9TriggerWord(revo9TriggerWord);
          unsigned int userWord = buf_slot[numDetNwords - 1] & 0xFFFF;
          klmDigitEventInfo->setUserWord(userWord);
        } else {
          klmDigitEventInfo->setRevo9TriggerWord(0);
          klmDigitEventInfo->setUserWord(0);
        }
        for (int iHit = 0; iHit < numHits; iHit++) {
          if (eklmHit) {
            unpackEKLMDigit(&buf_slot[iHit * hitLength], &lane,
                            klmDigitEventInfo);
          } else {
            unpackBKLMDigit(&buf_slot[iHit * hitLength], copperId, finesse_num,
                            klmDigitEventInfo);
          }
        }
      }
    }
  }
}

void KLMUnpackerModule::endRun()
{
}

void KLMUnpackerModule::terminate()
{
  for (const auto& message : m_rejected) {
    B2DEBUG(20, "KLMUnpackerModule:: " << message.first << " (occured " << message.second << " times)");
  }
}
