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
using namespace Belle2::KLM;

REG_MODULE(KLMUnpacker)

KLMUnpackerModule::KLMUnpackerModule() : Module(),
  m_Time(&(KLMTime::Instance())),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
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
  if (!m_FEEParameters.isValid())
    B2FATAL("KLM scintillator FEE parameters are not available.");
  m_triggerCTimeOfPreviousEvent = 0;
  m_Time->updateConstants();
}

void KLMUnpackerModule::createDigit(
  const KLM::RawData* raw, const KLMDigitRaw* klmDigitRaw,
  KLMDigitEventInfo* klmDigitEventInfo, int subdetector, int section,
  int sector, int layer, int plane, int strip, int lastStrip)
{
  KLMDigit* klmDigit = m_Digits.appendNew();
  klmDigit->addRelationTo(klmDigitEventInfo);
  if (m_WriteDigitRaws)
    klmDigit->addRelationTo(klmDigitRaw);
  bool isRPC = (subdetector == KLMElementNumbers::c_BKLM) &&
               (layer >= BKLMElementNumbers::c_FirstRPCLayer);
  if (isRPC) {
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
      m_Time->getRPCTimes(raw->getCTime(), raw->getTDC(), triggerTime);
    klmDigit->setTime(rpcTimes.second);
  } else {
    /*
     * For scintillator hits, store the ctime relative to the event header's
     * trigger ctime.
     */
    klmDigitEventInfo->increaseSciHits();
    double time = m_Time->getScintillatorTime(
                    raw->getCTime(), klmDigitEventInfo->getTriggerCTime());
    klmDigit->setTime(time);
    KLMChannelNumber channelNumber = m_ElementNumbers->channelNumber(subdetector, section, sector, layer, plane, strip);
    const KLMScintillatorFEEData* FEEData =
      m_FEEParameters->getFEEData(channelNumber);
    if (FEEData == nullptr)
      B2FATAL("Incomplete KLM scintillator FEE data.");
    if (raw->getCharge() < FEEData->getThreshold())
      klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
    else
      klmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
  }
  klmDigit->setSubdetector(subdetector);
  klmDigit->setSection(section);
  klmDigit->setLayer(layer);
  klmDigit->setSector(sector);
  klmDigit->setPlane(plane);
  klmDigit->setStrip(strip);
  if (lastStrip > 0)
    klmDigit->setLastStrip(lastStrip);
  klmDigit->setCharge(raw->getCharge());
  klmDigit->setCTime(raw->getCTime());
  klmDigit->setTDC(raw->getTDC());
}

void KLMUnpackerModule::unpackKLMDigit(
  const int* rawData, int copper, int hslb, int daqSubdetector,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  KLMDigitRaw* klmDigitRaw;
  KLM::RawData raw(copper, hslb + 1, rawData,
                   &m_klmDigitRaws, &klmDigitRaw, m_WriteDigitRaws);
  const KLMChannelNumber* detectorChannel;
  int subdetector, section, sector, layer, plane, strip;
  /* Get channel groups. */
  std::vector<ChannelGroup> channelGroups;
  raw.getChannelGroups(channelGroups);
  /* Get detector channels. */
  KLMElectronicsChannel electronicsChannel(
    copper, hslb + 1, raw.getLane(), raw.getAxis(), raw.getChannel());
  bool channelFound = false;
  for (ChannelGroup& channelGroup : channelGroups) {
    if (channelGroup.lastChannel == 0) {
      /* Single-strip hit. */
      detectorChannel =
        m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
      if (detectorChannel != nullptr) {
        /* The channel is found, get element numbers. */
        channelFound = true;
        m_ElementNumbers->channelNumberToElementNumbers(
          *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
          &strip);
        channelGroup.firstStrip = strip;
        channelGroup.lastStrip = 0;
      } else {
        /* The channel is not found, print error message. */
        if (!(m_IgnoreWrongHits || (raw.getChannel() == 0 && m_IgnoreStrip0))) {
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
      }
    } else {
      /* Do not process multiple-strip hit in the electronics map debug mode. */
      if (m_DebugElectronicsMap)
        return;
      /*
       * Multiple-strip hit. It is necessary to find matching detector channels
       * for all DAQ channels, because all channels in the group may not
       * be necessary connected to strips in case of BKLM.
       */
      bool firstMatchedChannel = true;
      for (int channel = channelGroup.firstChannel;
           channel <= channelGroup.lastChannel; ++channel) {
        electronicsChannel.setChannel(channel);
        detectorChannel =
          m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
        /* The channel is found, get element numbers. */
        if (detectorChannel != nullptr) {
          channelFound = true;
          m_ElementNumbers->channelNumberToElementNumbers(
            *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
            &strip);
          if (firstMatchedChannel) {
            firstMatchedChannel = false;
            channelGroup.firstStrip = strip;
            channelGroup.lastStrip = 0;
          } else {
            channelGroup.lastStrip = strip;
          }
        }
      }
      /* No matches found for this group at all. */
      if (firstMatchedChannel) {
        B2DEBUG(20, "No matching channels exist in the KLM electronics map."
                << LogVar("Copper", electronicsChannel.getCopper())
                << LogVar("Slot", electronicsChannel.getSlot())
                << LogVar("Lane", electronicsChannel.getLane())
                << LogVar("Axis", electronicsChannel.getAxis())
                << LogVar("First channel", channelGroup.firstChannel)
                << LogVar("Last channel", channelGroup.lastChannel)
                << LogVar("Trigger bits", raw.getTriggerBits()));
      }
    }
  }
  /* No detector channel is found. */
  if (!channelFound) {
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
      klmDigitOutOfRange->setCharge(raw.getCharge());
      klmDigitOutOfRange->setCTime(raw.getCTime());
      klmDigitOutOfRange->setTDC(raw.getTDC());
      return;
    }
  }
  /* Debug mode: write raw channel number to strip number. */
  if (m_DebugElectronicsMap) {
    if (m_DAQChannelBKLMScintillators) {
      if ((subdetector == KLMElementNumbers::c_BKLM) &&
          (layer < BKLMElementNumbers::c_FirstRPCLayer))
        strip = raw.getChannel();
    }
    if (m_DAQChannelModule >= 0) {
      KLMModuleNumber klmModule =
        m_ElementNumbers->moduleNumberByChannel(*detectorChannel);
      if (klmModule == m_DAQChannelModule)
        strip = raw.getChannel();
    }
  }
  /* Create KLM digits. */
  for (const ChannelGroup& channelGroup : channelGroups) {
    if (channelGroup.firstStrip != 0) {
      createDigit(&raw, klmDigitRaw, klmDigitEventInfo, subdetector, section,
                  sector, layer, plane, channelGroup.firstStrip,
                  channelGroup.lastStrip);
    }
  }
}

void KLMUnpackerModule::convertPCIe40ToCOPPER(int channel, unsigned int* copper, int* hslb) const
{
  if (channel >= 0 && channel < 16) {
    int id = channel / 4;
    *copper = BKLM_ID + id + 1;
    *hslb = channel - id * 4;
  } else if (channel >= 16 && channel < 32) {
    int id = (channel - 16) / 4;
    *copper = EKLM_ID + id + 1;
    *hslb = (channel - 16) - id * 4;
  } else
    B2FATAL("The PCIe40 channel is invalid."
            << LogVar("Channel", channel));
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
     * GetNumEntries is defined in RawDataBlock.h and gives the numberOfNodes*numberOfEvents.
     * Number of nodes is the number of COPPER boards.
     */
    for (int j = 0; j < m_RawKLMs[i]->GetNumEntries(); j++) {
      unsigned int copper = m_RawKLMs[i]->GetNodeID(j);
      int hslb, subdetector;
      m_RawKLMs[i]->GetBuffer(j);
      for (int channelReadoutBoard = 0; channelReadoutBoard < m_RawKLMs[i]->GetMaxNumOfCh(j); channelReadoutBoard++) {
        if (m_RawKLMs[i]->GetMaxNumOfCh(j) == 4) { // COPPER data
          hslb = channelReadoutBoard;
          if ((copper >= EKLM_ID) && (copper <= EKLM_ID + 4))
            subdetector = KLMElementNumbers::c_EKLM;
          else if ((copper >= BKLM_ID) && (copper <= BKLM_ID + 4))
            subdetector = KLMElementNumbers::c_BKLM;
          else
            continue;
        } else if (m_RawKLMs[i]->GetMaxNumOfCh(j) == 48) { // PCIe40 data
          if (channelReadoutBoard >= 0 && channelReadoutBoard < 16)
            subdetector = KLMElementNumbers::c_BKLM;
          else if (channelReadoutBoard >= 16 && channelReadoutBoard < 32)
            subdetector = KLMElementNumbers::c_EKLM;
          else
            continue;
          convertPCIe40ToCOPPER(channelReadoutBoard, &copper, &hslb);
        } else {
          B2FATAL("The maximum number of channels per readout board is invalid."
                  << LogVar("Number of channels", m_RawKLMs[i]->GetMaxNumOfCh(j)));
        }
        KLMDigitEventInfo* klmDigitEventInfo =
          m_DigitEventInfos.appendNew(m_RawKLMs[i], j);
        klmDigitEventInfo->setPreviousEventTriggerCTime(
          m_triggerCTimeOfPreviousEvent);
        m_triggerCTimeOfPreviousEvent = klmDigitEventInfo->getTriggerCTime();
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, channelReadoutBoard);
        int* hslbBuffer = m_RawKLMs[i]->GetDetectorBuffer(j, channelReadoutBoard);
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
          unsigned int revo9TriggerWord =
            (hslbBuffer[numDetNwords - 1] >> 16) & 0xFFFF;
          klmDigitEventInfo->setRevo9TriggerWord(revo9TriggerWord);
          unsigned int userWord = hslbBuffer[numDetNwords - 1] & 0xFFFF;
          klmDigitEventInfo->setUserWord(userWord);
        } else {
          klmDigitEventInfo->setRevo9TriggerWord(0);
          klmDigitEventInfo->setUserWord(0);
        }
        for (int iHit = 0; iHit < numHits; iHit++) {
          unpackKLMDigit(&hslbBuffer[iHit * hitLength], copper, hslb,
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
}
