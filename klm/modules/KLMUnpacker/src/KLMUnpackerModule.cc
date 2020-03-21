/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMUnpacker/KLMUnpackerModule.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>
#include <klm/rawdata/RawData.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <cstdint>

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
  addParam("WriteDigitRaws", m_WriteDigitRaws,
           "Record raw data in dataobject format (e.g. for debugging).", false);
  addParam("WriteWrongHits", m_WriteWrongHits,
           "Record wrong hits (e.g. for debugging).", false);
  addParam("DebugElectronicsMap", m_DebugElectronicsMap,
           "Debug electronics map (record DAQ channel instead of strip).",
           false);
  addParam("DAQChannelBKLMScintillators", m_DAQChannelBKLMScintillators,
           "Record DAQ channel for BKLM scintillators.", false);
  addParam("DAQChannelModule", m_DAQChannelModule,
           "Record DAQ channel for specific module.", -1);
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
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_eklmElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMUnpackerModule::~KLMUnpackerModule()
{
}

void KLMUnpackerModule::initialize()
{
  m_RawKLMs.isRequired();
  /* Digits. */
  m_bklmDigits.registerInDataStore(m_outputBKLMDigitsName);
  m_bklmDigitsOutOfRange.registerInDataStore("BKLMDigitsOutOfRange");
  m_eklmDigits.registerInDataStore(m_outputEKLMDigitsName);
  /* Event information. */
  m_DigitEventInfos.registerInDataStore();
  m_bklmDigits.registerRelationTo(m_DigitEventInfos);
  m_bklmDigitsOutOfRange.registerRelationTo(m_DigitEventInfos);
  m_eklmDigits.registerRelationTo(m_DigitEventInfos);
  /* Raw data in dataobject format. */
  if (m_WriteDigitRaws) {
    m_klmDigitRaws.registerInDataStore();
    m_bklmDigits.registerRelationTo(m_klmDigitRaws);
    m_bklmDigitsOutOfRange.registerRelationTo(m_klmDigitRaws);
    m_eklmDigits.registerRelationTo(m_klmDigitRaws);
  }
}

void KLMUnpackerModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("KLM electronics map is not available.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_eklmChannels.isValid())
    B2FATAL("EKLM channel data are not available.");
  if (m_loadThresholdFromDB) {
    if (!m_bklmADCParams.isValid())
      B2FATAL("BKLM ADC threshold paramenters are not available.");
    m_scintThreshold = m_bklmADCParams->getADCThreshold();
  }
  m_triggerCTimeOfPreviousEvent = 0;
}

void KLMUnpackerModule::unpackEKLMDigit(
  const int* rawData, int copper, int hslb,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  int subdetector, section, sector, layer, plane, strip;
  KLM::RawData raw;
  KLMDigitRaw* klmDigitRaw;
  KLM::unpackRawData(copper, hslb + 1, rawData, &raw, &m_klmDigitRaws,
                     &klmDigitRaw, m_WriteDigitRaws);
  const uint16_t* detectorChannel;
  KLMElectronicsChannel electronicsChannel(
    copper, hslb + 1, raw.lane, raw.axis, raw.channel);
  detectorChannel =
    m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
  if ((raw.triggerBits & 0x10) != 0)
    return;
  bool correctHit = true;
  if (detectorChannel == nullptr) {
    if (!m_IgnoreWrongHits) {
      B2ERROR("Channel does not exist in the KLM electronics map."
              << LogVar("Copper", electronicsChannel.getCopper())
              << LogVar("Slot", electronicsChannel.getSlot())
              << LogVar("Lane", electronicsChannel.getLane())
              << LogVar("Axis", electronicsChannel.getAxis())
              << LogVar("Channel", electronicsChannel.getChannel()));
    }
    if (!m_WriteWrongHits)
      return;
    section = 0;
    sector = 0;
    layer = 0;
    plane = 0;
    strip = 0;
    correctHit = false;
  } else {
    m_ElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
  }
  EKLMDigit* eklmDigit = m_eklmDigits.appendNew();
  eklmDigit->addRelationTo(klmDigitEventInfo);
  if (m_WriteDigitRaws)
    eklmDigit->addRelationTo(klmDigitRaw);
  eklmDigit->setTime(
    m_TimeConversion->getScintillatorTime(raw.ctime, klmDigitEventInfo->getTriggerCTime()));
  eklmDigit->setSubdetector(KLMElementNumbers::c_EKLM);
  eklmDigit->setSection(section);
  eklmDigit->setLayer(layer);
  eklmDigit->setSector(sector);
  eklmDigit->setPlane(plane);
  eklmDigit->setStrip(strip);
  eklmDigit->setCharge(raw.charge);
  eklmDigit->setCTime(raw.ctime);
  eklmDigit->setTDC(raw.tdc);
  if (correctHit) {
    int stripGlobal = m_eklmElementNumbers->stripNumber(
                        section, layer, sector, plane, strip);
    const EKLMChannelData* channelData =
      m_eklmChannels->getChannelData(stripGlobal);
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
  KLM::unpackRawData(copper, hslb + 1, rawData, &raw,
                     &m_klmDigitRaws, &klmDigitRaw, m_WriteDigitRaws);
  const uint16_t* detectorChannel;
  int subdetector, section, sector, layer, plane, strip;
  KLMElectronicsChannel electronicsChannel(
    copper, hslb + 1, raw.lane, raw.axis, raw.channel);
  detectorChannel =
    m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
  if (detectorChannel == nullptr) {
    B2DEBUG(20, "Channel does not exist in the KLM electronics map."
            << LogVar("Copper", electronicsChannel.getCopper())
            << LogVar("Slot", electronicsChannel.getSlot())
            << LogVar("Lane", electronicsChannel.getLane())
            << LogVar("Axis", electronicsChannel.getAxis())
            << LogVar("Channel", electronicsChannel.getChannel()));
    if (!(m_WriteWrongHits || m_DebugElectronicsMap))
      return;
    /*
     * Try to find channel from the same plane.
     * Phi-plane channels may start from 3 or 5.
     */
    electronicsChannel.setChannel(5);
    detectorChannel = m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr)
      return;
    m_ElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    if (m_WriteWrongHits) {
      // increase by 1 the event-counter of outOfRange-flagged hits
      klmDigitEventInfo->increaseOutOfRangeHits();

      // store the digit in the appropriate dataobject
      BKLMDigit* bklmDigitOutOfRange =
        m_bklmDigitsOutOfRange.appendNew();
      bklmDigitOutOfRange->addRelationTo(klmDigitEventInfo);
      if (m_WriteDigitRaws)
        bklmDigitOutOfRange->addRelationTo(klmDigitRaw);
      bklmDigitOutOfRange->setSubdetector(KLMElementNumbers::c_BKLM);
      bklmDigitOutOfRange->setSection(section);
      bklmDigitOutOfRange->setLayer(layer);
      bklmDigitOutOfRange->setSector(sector);
      bklmDigitOutOfRange->setPlane(plane);
      bklmDigitOutOfRange->setStrip(strip);
      bklmDigitOutOfRange->setCharge(raw.charge);
      bklmDigitOutOfRange->setCTime(raw.ctime);
      bklmDigitOutOfRange->setTDC(raw.tdc);

      std::string message = "channel number is out of range";
      m_rejected[message] += 1;
      m_rejectedCount++;
      B2DEBUG(21, "KLMUnpackerModule:: raw channel number is out of range"
              << LogVar("Channel", raw.channel));
      return;
    }
    bool recordDebugHit = false;
    if (m_DAQChannelBKLMScintillators) {
      /* The strip is 1-based, but stored as 0-based. Do not set channel to 0. */
      if (layer < BKLMElementNumbers::c_FirstRPCLayer && raw.channel > 0) {
        strip = raw.channel;
        recordDebugHit = true;
      }
    }
    if (m_DAQChannelModule) {
      uint16_t klmModule = m_ElementNumbers->moduleNumberByChannel(*detectorChannel);
      if (klmModule == m_DAQChannelModule && raw.channel > 0) {
        strip = raw.channel;
        recordDebugHit = true;
      }
    }
    if (!recordDebugHit)
      return;
  } else {
    m_ElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    if (m_DebugElectronicsMap) {
      if (m_DAQChannelBKLMScintillators) {
        if (layer < BKLMElementNumbers::c_FirstRPCLayer && raw.channel > 0)
          strip = raw.channel;
      }
      if (m_DAQChannelModule) {
        uint16_t klmModule = m_ElementNumbers->moduleNumberByChannel(*detectorChannel);
        if (klmModule == m_DAQChannelModule && raw.channel > 0)
          strip = raw.channel;
      }
    }
  }

  if ((layer < BKLMElementNumbers::c_FirstRPCLayer) && ((raw.triggerBits & 0x10) != 0))
    return;
  if (layer > BKLMElementNumbers::getMaximalLayerNumber()) {
    B2DEBUG(20, "KLMUnpackerModule:: strange that the layer number is larger than 15 "
            << LogVar("Layer", layer));
    return;
  }

  BKLMDigit* bklmDigit;
  if (layer >= BKLMElementNumbers::c_FirstRPCLayer) {
    klmDigitEventInfo->increaseRPCHits();
    // For RPC hits, digitize both the coarse (ctime) and fine (tdc) times relative
    // to the revo9 trigger time rather than the event header's TriggerCTime.
    // For the fine-time (tdc) measurement (11 bits), shift the revo9Trig time by
    // 10 ticks to align the new prompt-time peak with the TriggerCtime-relative peak.
    float triggerTime = klmDigitEventInfo->getRevo9TriggerWord();
    std::pair<int, double> rpcTimes = m_TimeConversion->getRPCTimes(raw.ctime, raw.tdc, triggerTime);
    bklmDigit = m_bklmDigits.appendNew();
    bklmDigit->setTime(rpcTimes.second);
  } else {
    klmDigitEventInfo->increaseSciHits();
    // For scintillator hits, store the ctime relative to the event header's trigger ctime
    bklmDigit = m_bklmDigits.appendNew();
    bklmDigit->setTime(
      m_TimeConversion->getScintillatorTime(raw.ctime, klmDigitEventInfo->getTriggerCTime()));
    if (raw.charge < m_scintThreshold)
      bklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
    else
      bklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
  }
  bklmDigit->addRelationTo(klmDigitEventInfo);
  if (m_WriteDigitRaws)
    bklmDigit->addRelationTo(klmDigitRaw);
  bklmDigit->setSubdetector(KLMElementNumbers::c_BKLM);
  bklmDigit->setSection(section);
  bklmDigit->setLayer(layer);
  bklmDigit->setSector(sector);
  bklmDigit->setPlane(plane);
  bklmDigit->setStrip(strip);
  bklmDigit->setCharge(raw.charge);
  bklmDigit->setCTime(raw.ctime);
  bklmDigit->setTDC(raw.tdc);
}

void KLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4-byte words. This is needed to find the hits in the
   * detector buffer.
   */
  const int hitLength = 2;
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
            unpackEKLMDigit(&buf_slot[iHit * hitLength], copperId, finesse_num,
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
