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
  setDescription("KLM unpacker (creates KLMDigits from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputKLMDigitsName", m_outputKLMDigitsName,
           "Name of KLMDigit store array.", string(""));
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
  m_Digits.registerInDataStore(m_outputKLMDigitsName);
  m_klmDigitsOutOfRange.registerInDataStore("KLMDigitsOutOfRange");
  /* Event information. */
  m_DigitEventInfos.registerInDataStore();
  m_Digits.registerRelationTo(m_DigitEventInfos);
  m_klmDigitsOutOfRange.registerRelationTo(m_DigitEventInfos);
  /* Raw data in dataobject format. */
  if (m_WriteDigitRaws) {
    m_klmDigitRaws.registerInDataStore();
    m_Digits.registerRelationTo(m_klmDigitRaws);
    m_klmDigitsOutOfRange.registerRelationTo(m_klmDigitRaws);
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

void KLMUnpackerModule::unpackKLMDigit(
  const int* rawData, int copper, int hslb, int daqSubdetector,
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
    /* The channel is not found, print error message. */
    if (!(m_IgnoreWrongHits || (raw.channel == 0 && m_IgnoreStrip0))) {
      if (daqSubdetector == KLMElementNumbers::c_BKLM) {
        B2DEBUG(20, "Channel does not exist in the KLM electronics map."
                << LogVar("Copper", electronicsChannel.getCopper())
                << LogVar("Slot", electronicsChannel.getSlot())
                << LogVar("Lane", electronicsChannel.getLane())
                << LogVar("Axis", electronicsChannel.getAxis())
                << LogVar("Channel", electronicsChannel.getChannel()));
      } else {
        B2ERROR("Channel does not exist in the KLM electronics map."
                << LogVar("Copper", electronicsChannel.getCopper())
                << LogVar("Slot", electronicsChannel.getSlot())
                << LogVar("Lane", electronicsChannel.getLane())
                << LogVar("Axis", electronicsChannel.getAxis())
                << LogVar("Channel", electronicsChannel.getChannel()));
      }
    }
    if (!(m_WriteWrongHits || m_DebugElectronicsMap))
      return;
    /*
     * Try to find channel from the same plane.
     * BKLM phi-plane channels may start from 3 or 5.
     */
    electronicsChannel.setChannel(5);
    detectorChannel = m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr)
      return;
    /* The channel is found, store out-of-range digit. */
    m_ElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    if (m_WriteWrongHits) {
      klmDigitEventInfo->increaseOutOfRangeHits();
      KLMDigit* klmDigitOutOfRange =
        m_klmDigitsOutOfRange.appendNew();
      klmDigitOutOfRange->addRelationTo(klmDigitEventInfo);
      if (m_WriteDigitRaws)
        klmDigitOutOfRange->addRelationTo(klmDigitRaw);
      klmDigitOutOfRange->setSubdetector(KLMElementNumbers::c_BKLM);
      klmDigitOutOfRange->setSection(section);
      klmDigitOutOfRange->setLayer(layer);
      klmDigitOutOfRange->setSector(sector);
      klmDigitOutOfRange->setPlane(plane);
      klmDigitOutOfRange->setStrip(strip);
      klmDigitOutOfRange->setCharge(raw.charge);
      klmDigitOutOfRange->setCTime(raw.ctime);
      klmDigitOutOfRange->setTDC(raw.tdc);
      return;
    }
    /* Debug mode: write raw channel number to strip number. */
    bool recordDebugHit = false;
    if (m_DAQChannelBKLMScintillators) {
      if ((subdetector == KLMElementNumbers::c_BKLM) &&
          (layer < BKLMElementNumbers::c_FirstRPCLayer)) {
        strip = raw.channel;
        recordDebugHit = true;
      }
    }
    if (m_DAQChannelModule >= 0) {
      uint16_t klmModule =
        m_ElementNumbers->moduleNumberByChannel(*detectorChannel);
      if (klmModule == m_DAQChannelModule) {
        strip = raw.channel;
        recordDebugHit = true;
      }
    }
    if (!recordDebugHit)
      return;
  } else {
    /* The channel is found, get element numbers. */
    m_ElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    /* Debug mode: write raw channel number to strip number. */
    if (m_DebugElectronicsMap) {
      if (m_DAQChannelBKLMScintillators) {
        if ((subdetector == KLMElementNumbers::c_BKLM) &&
            (layer < BKLMElementNumbers::c_FirstRPCLayer))
          strip = raw.channel;
      }
      if (m_DAQChannelModule >= 0) {
        uint16_t klmModule =
          m_ElementNumbers->moduleNumberByChannel(*detectorChannel);
        if (klmModule == m_DAQChannelModule)
          strip = raw.channel;
      }
    }
  }
  /* Ignore multi-strip scintillator hits. */
  bool isRPC = (subdetector == KLMElementNumbers::c_BKLM) &&
               (layer >= BKLMElementNumbers::c_FirstRPCLayer);
  if (!isRPC && ((raw.triggerBits & 0x10) != 0))
    return;
  /* Create KLM digit. */
  KLMDigit* klmDigit;
  if (layer >= BKLMElementNumbers::c_FirstRPCLayer) {
    /*
     * For RPC hits, digitize both the coarse (ctime) and fine (tdc) times
     * relative to the revo9 trigger time rather than the event header's
     * TriggerCTime. For the fine-time (tdc) measurement (11 bits), shift
     * the revo9Trig time by 10 ticks to align the new prompt-time peak
     * with the TriggerCtime-relative peak.
     */
    klmDigitEventInfo->increaseRPCHits();
    float triggerTime = klmDigitEventInfo->getRevo9TriggerWord();
    std::pair<int, double> rpcTimes =
      m_TimeConversion->getRPCTimes(raw.ctime, raw.tdc, triggerTime);
    klmDigit = m_Digits.appendNew();
    klmDigit->setTime(rpcTimes.second);
  } else {
    /*
     * For scintillator hits, store the ctime relative to the event header's
     * trigger ctime.
     */
    klmDigitEventInfo->increaseSciHits();
    klmDigit = m_Digits.appendNew();
    double time = m_TimeConversion->getScintillatorTime(
                    raw.ctime, klmDigitEventInfo->getTriggerCTime());
    klmDigit->setTime(time);
    if (subdetector == KLMElementNumbers::c_BKLM) {
      if (raw.charge < m_scintThreshold)
        klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
      else
        klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
    } else {
      int stripGlobal = m_eklmElementNumbers->stripNumber(
                          section, layer, sector, plane, strip);
      const EKLMChannelData* channelData =
        m_eklmChannels->getChannelData(stripGlobal);
      if (channelData == nullptr)
        B2FATAL("Incomplete EKLM channel data.");
      if (raw.charge < channelData->getThreshold())
        klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
      else
        klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
    }
  }
  klmDigit->addRelationTo(klmDigitEventInfo);
  if (m_WriteDigitRaws)
    klmDigit->addRelationTo(klmDigitRaw);
  klmDigit->setSubdetector(subdetector);
  klmDigit->setSection(section);
  klmDigit->setLayer(layer);
  klmDigit->setSector(sector);
  klmDigit->setPlane(plane);
  klmDigit->setStrip(strip);
  klmDigit->setCharge(raw.charge);
  klmDigit->setCTime(raw.ctime);
  klmDigit->setTDC(raw.tdc);
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
      int subdetector;
      if ((copperId >= EKLM_ID) && (copperId <= EKLM_ID + 4))
        subdetector = KLMElementNumbers::c_EKLM;
      else if (!((copperId >= BKLM_ID) && (copperId <= BKLM_ID + 4)))
        subdetector = KLMElementNumbers::c_BKLM;
      else
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
          unpackKLMDigit(&buf_slot[iHit * hitLength], copperId, finesse_num,
                         subdetector, klmDigitEventInfo);
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
