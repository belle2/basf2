/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMDigitizer/KLMDigitizerModule.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>
#include <klm/simulation/ScintillatorSimulator.h>

/* Basf2 headers. */
#include <framework/dataobjects/BackgroundMetaData.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <TRandom.h>

using namespace Belle2;

REG_MODULE(KLMDigitizer);

KLMDigitizerModule::KLMDigitizerModule() :
  Module(),
  m_Time(&(KLMTime::Instance())),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_ChannelSpecificSimulation(false),
  m_EfficiencyMode(c_Plane),
  m_Fitter(nullptr)
{
  setDescription("KLM digitization module: create KLMDigits from KLMSimHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_SimulationMode,
           "Simulation mode (\"Generic\" or \"ChannelSpecific\").",
           std::string("Generic"));
  /*
   * Initial digitization time is negative to work with cosmic events (the part
   * of the track directed to the interaction pointhas a negative time).
   */
  addParam("DigitizationInitialTime", m_DigitizationInitialTime,
           "Initial digitization time in CTIME periods.", -5);
  addParam("SaveFPGAFit", m_SaveFPGAFit, "Save FPGA fit data and set a relation with KLMDigits.", false);
  addParam("Efficiency", m_Efficiency,
           "Efficiency determination mode (\"Strip\" or \"Plane\").",
           std::string("Plane"));
  addParam("CreateMultiStripDigits", m_CreateMultiStripDigits,
           "Whether to create multi-strip digits in Run 1 data (not used for Run 2+).", true);
  addParam("Debug", m_Debug,
           "Debug mode (generates additional output files with histograms).",
           false);
}

KLMDigitizerModule::~KLMDigitizerModule()
{
}

void KLMDigitizerModule::initialize()
{
  m_SimHits.isRequired();
  m_Digits.registerInDataStore();
  m_Digits.registerRelationTo(m_SimHits);
  if (m_SaveFPGAFit) {
    m_FPGAFits.registerInDataStore();
    m_Digits.registerRelationTo(m_FPGAFits);
  }
  if (m_SimulationMode == "Generic") {
    /* Nothing to do. */
  } else if (m_SimulationMode == "ChannelSpecific") {
    m_ChannelSpecificSimulation = true;
  } else {
    B2FATAL("Unknown simulation mode: " << m_SimulationMode);
  }
  if (m_Efficiency == "Strip")
    m_EfficiencyMode = c_Strip;
  else if (m_Efficiency == "Plane")
    m_EfficiencyMode = c_Plane;
  else
    B2FATAL("Unknown efficiency mode: " << m_EfficiencyMode);
}

void KLMDigitizerModule::checkScintillatorFEEParameters()
{
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    KLMChannelNumber channel = m_ElementNumbers->channelNumber(klmChannel.getSubdetector(), klmChannel.getSection(),
                                                               klmChannel.getSector(), klmChannel.getLayer(),
                                                               klmChannel.getPlane(), klmChannel.getStrip());
    const KLMScintillatorFEEData* FEEData = m_FEEPar->getFEEData(channel);
    if (FEEData == nullptr)
      B2FATAL("Incomplete scintillator FEE data.");
    if (FEEData->getPhotoelectronAmplitude() <= 0) {
      B2ERROR("Non-positive photoelectron amplitude. The requested "
              "channel-specific simulation is impossible. "
              "KLMDigitizer is switched to the generic mode."
              << LogVar("Section", klmChannel.getSection())
              << LogVar("Layer", klmChannel.getLayer())
              << LogVar("Sector", klmChannel.getSector())
              << LogVar("Plane", klmChannel.getPlane())
              << LogVar("Strip", klmChannel.getStrip()));
      m_ChannelSpecificSimulation = false;
      return;
    }
  }
}

void KLMDigitizerModule::beginRun()
{
  if (!m_DigPar.isValid())
    B2FATAL("KLM scintillator digitization parameters are not available.");
  if (!m_FEEPar.isValid())
    B2FATAL("KLM scintillator FEE parameters are not available.");
  if (!m_ChannelStatus.isValid())
    B2FATAL("KLM channel status data are not available.");
  if (!m_StripEfficiency.isValid())
    B2FATAL("KLM strip efficiency data are not available.");
  if (!m_ScintillatorFirmware.isValid())
    B2FATAL("KLM scintillator firmware version is not available.");
  if (m_ChannelSpecificSimulation)
    checkScintillatorFEEParameters();
  m_Time->updateConstants();
  m_Fitter = new KLM::ScintillatorFirmware(m_DigPar->getNDigitizations());
  KLMScintillatorFirmware::FirmwareVersion fwVersion = m_ScintillatorFirmware->getFirmwareVersion();
  m_CreateMultiStripDigitsByRun = false; // do not make multi-strip KLMDigits for Run 2+ events
  if ((fwVersion == KLMScintillatorFirmware::FirmwareVersion::c_Invalid) || // this should never happen!
      (fwVersion == KLMScintillatorFirmware::FirmwareVersion::c_Phase2) ||  // for very early data (deprecated)
      (fwVersion == KLMScintillatorFirmware::FirmwareVersion::c_Run1)) {  // for data up to and including 2022b
    m_CreateMultiStripDigitsByRun = m_CreateMultiStripDigits;
  }
  B2INFO("KLM multi-strip digits are " << (m_CreateMultiStripDigitsByRun ? "" : "NOT") << " simulated");
}

/*
 * Digitization. Light propagation into the fiber, SiPM and electronics effects
 * are simulated in KLM::ScintillatorSimulator class.
 */

bool KLMDigitizerModule::checkActive(KLMChannelNumber channel)
{
  enum KLMChannelStatus::ChannelStatus status =
    m_ChannelStatus->getChannelStatus(channel);
  if (status == KLMChannelStatus::c_Unknown)
    B2FATAL("Incomplete KLM channel status data.");
  return (status != KLMChannelStatus::c_Dead);
}

bool KLMDigitizerModule::efficiencyCorrection(float efficiency)
{
  if (isnan(efficiency))
    B2FATAL("Incomplete KLM efficiency data.");
  double selection = gRandom->Rndm();
  return (selection < efficiency);
}

void KLMDigitizerModule::digitizeRPC()
{
  std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator
  it, it2, lowerBound, upperBound;
  it = m_MapChannelSimHit.begin();
  while (it != m_MapChannelSimHit.end()) {
    lowerBound = it;
    upperBound = m_MapChannelSimHit.upper_bound(it->first);
    it = upperBound;
    if (not lowerBound->second->inRPC())
      B2FATAL("KLMDigitizer::digitizeRPC is trying to process a scintillator hit.");
    if (m_EfficiencyMode == c_Strip) {
      float efficiency = m_StripEfficiency->getEfficiency(lowerBound->first);
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    int strip = BKLMElementNumbers::getStripByModule(
                  m_ElementNumbers->localChannelNumberBKLM(lowerBound->first));
    /* Select hit that has the smallest time. */
    it2 = lowerBound;
    const KLMSimHit* hit = lowerBound->second;
    double time = hit->getTime();
    ++it2;
    while (it2 != upperBound) {
      if (it2->second->getTime() < time) {
        time = it2->second->getTime();
        hit = it2->second;
      }
      ++it2;
    }
    KLMDigit* digit = m_Digits.appendNew(hit, strip);
    it2 = lowerBound;
    while (it2 != upperBound) {
      digit->addRelationTo(it2->second);
      ++it2;
    }
  }
}

void KLMDigitizerModule::digitizeScintillator()
{
  uint16_t tdc;
  KLM::ScintillatorSimulator simulator(
    &(*m_DigPar), m_Fitter,
    m_DigitizationInitialTime * m_Time->getCTimePeriod(), m_Debug);
  const KLMScintillatorFEEData* FEEData;
  std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator
  it, lowerBound, upperBound;
  for (int i = 0; i < KLM::c_NChannelsAsic; ++i)
    m_AsicDigits[i] = nullptr;
  it = m_MapChannelSimHit.begin();
  while (it != m_MapChannelSimHit.end()) {
    lowerBound = it;
    upperBound = m_MapChannelSimHit.upper_bound(it->first);
    it = upperBound;
    if (lowerBound->second->inRPC())
      B2FATAL("KLMDigitizer::digitizeScintillator is trying to process a RPC hit.");
    const KLMSimHit* simHit = lowerBound->second;
    if (m_EfficiencyMode == c_Strip) {
      float efficiency = m_StripEfficiency->getEfficiency(lowerBound->first);
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    if (m_ChannelSpecificSimulation) {
      FEEData = m_FEEPar->getFEEData(lowerBound->first);
      if (FEEData == nullptr)
        B2FATAL("Incomplete KLM scintillator FEE data.");
      simulator.setFEEData(FEEData);
    }
    simulator.simulate(lowerBound, upperBound);
    if (simulator.getNGeneratedPhotoelectrons() == 0)
      continue;
    const KLMElectronicsChannel* electronicsChannel =
      m_ElectronicsMap->getElectronicsChannel(lowerBound->first);
    int channel = (electronicsChannel->getChannel() - 1) %
                  KLM::c_NChannelsAsic;
    KLMDigit* digit = new KLMDigit(simHit);
    m_AsicDigits[channel] = digit;
    m_AsicDigitSimHitsLowerBound[channel] = lowerBound;
    m_AsicDigitSimHitsUpperBound[channel] = upperBound;
    digit->setMCTime(simulator.getMCTime());
    digit->setSiPMMCTime(simulator.getSiPMMCTime());
    digit->setNGeneratedPhotoelectrons(
      simulator.getNGeneratedPhotoelectrons());
    if (simulator.getFitStatus() ==
        KLM::c_ScintillatorFirmwareSuccessfulFit) {
      tdc = simulator.getFPGAFit()->getStartTime();
      digit->setCharge(simulator.getFPGAFit()->getMinimalAmplitude());
    } else {
      tdc = 0;
      digit->setCharge(m_DigPar->getADCRange() - 1);
    }
    digit->setTDC(tdc);
    digit->setTime(m_Time->getTimeSimulation(tdc, true));
    digit->setFitStatus(simulator.getFitStatus());
    digit->setNPhotoelectrons(simulator.getNPhotoelectrons());
    digit->setEnergyDeposit(simulator.getEnergy());
    if (m_SaveFPGAFit && (simulator.getFitStatus() ==
                          KLM::c_ScintillatorFirmwareSuccessfulFit)) {
      KLMScintillatorFirmwareFitResult* fit =
        m_FPGAFits.appendNew(*simulator.getFPGAFit());
      digit->addRelationTo(fit);
    }
  }
}

void KLMDigitizerModule::digitizeAsic()
{
  std::multimap<KLMElectronicsChannel, const KLMSimHit*>::iterator
  it, it2, upperBound;
  std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator it3;
  it = m_MapAsicSimHit.begin();
  while (it != m_MapAsicSimHit.end()) {
    upperBound = m_MapAsicSimHit.upper_bound(it->first);
    m_MapChannelSimHit.clear();
    for (it2 = it; it2 != upperBound; ++it2) {
      const KLMSimHit* hit = it2->second;
      KLMChannelNumber channel =
        m_ElementNumbers->channelNumber(
          hit->getSubdetector(), hit->getSection(), hit->getSector(),
          hit->getLayer(), hit->getPlane(), hit->getStrip());
      m_MapChannelSimHit.insert(
        std::pair<KLMChannelNumber, const KLMSimHit*>(channel, hit));
    }
    digitizeScintillator();
    if (m_CreateMultiStripDigitsByRun) {
      int nDigits = 0;
      for (int i = 0; i < KLM::c_NChannelsAsic; ++i) {
        if (m_AsicDigits[i] != nullptr)
          nDigits += 1;
      }
      bool multiStripMode = (nDigits >= 2);
      int i = 0;
      while (i < KLM::c_NChannelsAsic) {
        KLMDigit* digit = m_AsicDigits[i];
        if (digit == nullptr) {
          ++i;
          continue;
        }
        // Firmware bug (used OR of struck channel numbers in range 1..15) defeated the
        // expected by-4 grouping so we assume the worst case: all 15 channels are struck.
        // This will be reduced for BKLM scintillators if there are missing detectorChannels.
        int minGroupChannel = 1;
        int maxGroupChannel = KLM::c_NChannelsAsic;
        KLMDigit* arrayDigit = m_Digits.appendNew(*digit);
        KLMElectronicsChannel electronicsChannel(it->first);
        int asic = electronicsChannel.getChannel();
        bool connectedChannelFound = false;
        int minStrip, maxStrip;
        for (int j = minGroupChannel; j <= maxGroupChannel; ++j) {
          electronicsChannel.setChannel(KLM::c_NChannelsAsic * asic + j);
          const KLMChannelNumber* detectorChannel =
            m_ElectronicsMap->getDetectorChannel(&electronicsChannel);
          if (detectorChannel != nullptr) {
            int subdetector, section, sector, layer, plane, strip;
            m_ElementNumbers->channelNumberToElementNumbers(
              *detectorChannel, &subdetector, &section, &sector, &layer,
              &plane, &strip);
            if (!connectedChannelFound) {
              connectedChannelFound = true;
              minStrip = strip;
              maxStrip = strip;
            } else {
              if (strip < minStrip)
                minStrip = strip;
              if (strip > maxStrip)
                maxStrip = strip;
            }
          }
        }
        /* This should never happen. */
        if (!connectedChannelFound)
          B2FATAL("Cannot find connected electronics channels.");
        if (multiStripMode) {
          arrayDigit->setStrip(minStrip);
          arrayDigit->setLastStrip(maxStrip);
        }
        for (int j = i; j < maxGroupChannel; ++j) {
          if (m_AsicDigits[j] == nullptr)
            continue;
          for (it3 = m_AsicDigitSimHitsLowerBound[j];
               it3 != m_AsicDigitSimHitsUpperBound[j]; ++it3) {
            arrayDigit->addRelationTo(it3->second);
          }
        }
        i = maxGroupChannel;
      }
      for (i = 0; i < KLM::c_NChannelsAsic; ++i) {
        if (m_AsicDigits[i] != nullptr)
          delete m_AsicDigits[i];
      }
    } else {
      for (int i = 0; i < KLM::c_NChannelsAsic; ++i) {
        KLMDigit* digit = m_AsicDigits[i];
        if (digit == nullptr)
          continue;
        KLMDigit* arrayDigit = m_Digits.appendNew(*digit);
        for (it3 = m_AsicDigitSimHitsLowerBound[i];
             it3 != m_AsicDigitSimHitsUpperBound[i]; ++it3) {
          arrayDigit->addRelationTo(it3->second);
        }
        delete digit;
      }
    }
    it = upperBound;
  }
}

void KLMDigitizerModule::event()
{
  int i;
  KLMChannelNumber channel;
  m_MapChannelSimHit.clear();
  m_MapAsicSimHit.clear();
  if (m_EfficiencyMode == c_Plane) {
    m_MapPlaneSimHit.clear();
    for (i = 0; i < m_SimHits.getEntries(); i++) {
      const KLMSimHit* hit = m_SimHits[i];
      /* For RPCs. */
      if (hit->getStrip() <= 0)
        continue;
      const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
      /*
       * We do not simulate the plane efficiency for KLMSimHits
       * from beam background because there are no MCParticles associated
       * to them.
       */
      if (particle != nullptr) {
        KLMPlaneNumber plane =
          m_ElementNumbers->planeNumber(
            hit->getSubdetector(), hit->getSection(), hit->getSector(),
            hit->getLayer(), hit->getPlane());
        m_MapPlaneSimHit.insert(
          std::pair<KLMPlaneNumber, const KLMSimHit*>(plane, hit));
      } else {
        B2ASSERT("The KLMSimHit is not related to any MCParticle and "
                 "it is also not a beam background hit.",
                 hit->getBackgroundTag() != BackgroundMetaData::bg_none);
        channel =
          m_ElementNumbers->channelNumber(
            hit->getSubdetector(), hit->getSection(), hit->getSector(),
            hit->getLayer(), hit->getPlane(), hit->getStrip());
        if (checkActive(channel)) {
          bool rpc = hit->inRPC();
          if (rpc) {
            m_MapChannelSimHit.insert(std::pair<KLMChannelNumber, const KLMSimHit*>(channel, hit));
          } else {
            const KLMElectronicsChannel* electronicsChannel =
              m_ElectronicsMap->getElectronicsChannel(channel);
            if (electronicsChannel == nullptr)
              B2FATAL("Incomplete electronics map.");
            KLMElectronicsChannel asic = electronicsChannel->getAsic();
            m_MapAsicSimHit.insert(std::pair<KLMElectronicsChannel, const KLMSimHit*>(asic, hit));
          }
        }
      }
    }
    std::multimap<KLMPlaneNumber, const KLMSimHit*>::iterator it, it2;
    std::multimap<const MCParticle*, const KLMSimHit*> particleHitMap;
    std::multimap<const MCParticle*, const KLMSimHit*>::iterator
    itParticle, it2Particle;
    it = m_MapPlaneSimHit.begin();
    while (it != m_MapPlaneSimHit.end()) {
      particleHitMap.clear();
      it2 = it;
      while (true) {
        const KLMSimHit* hit = it2->second;
        const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
        particleHitMap.insert(
          std::pair<const MCParticle*, const KLMSimHit*>(particle, hit));
        ++it2;
        if (it2 == m_MapPlaneSimHit.end())
          break;
        if (it2->first != it->first)
          break;
      }
      itParticle = particleHitMap.begin();
      while (itParticle != particleHitMap.end()) {
        it2Particle = itParticle;
        const KLMSimHit* hit = it2Particle->second;
        channel =
          m_ElementNumbers->channelNumber(
            hit->getSubdetector(), hit->getSection(), hit->getSector(),
            hit->getLayer(), hit->getPlane(), hit->getStrip());
        float efficiency = m_StripEfficiency->getEfficiency(channel);
        bool hitSelected = efficiencyCorrection(efficiency);
        while (true) {
          hit = it2Particle->second;
          bool rpc = hit->inRPC();
          if (hitSelected) {
            for (int s = hit->getStrip(); s <= hit->getLastStrip(); ++s) {
              channel =
                m_ElementNumbers->channelNumber(
                  hit->getSubdetector(), hit->getSection(), hit->getSector(),
                  hit->getLayer(), hit->getPlane(), s);
              if (!checkActive(channel))
                continue;
              if (rpc) {
                m_MapChannelSimHit.insert(
                  std::pair<KLMChannelNumber, const KLMSimHit*>(channel, hit));
              } else {
                const KLMElectronicsChannel* electronicsChannel =
                  m_ElectronicsMap->getElectronicsChannel(channel);
                if (electronicsChannel == nullptr)
                  B2FATAL("Incomplete electronics map.");
                KLMElectronicsChannel asic = electronicsChannel->getAsic();
                m_MapAsicSimHit.insert(
                  std::pair<KLMElectronicsChannel, const KLMSimHit*>(
                    asic, hit));
              }
            }
          }
          ++it2Particle;
          if (it2Particle == particleHitMap.end())
            break;
          if (it2Particle->first != itParticle->first)
            break;
        }
        itParticle = it2Particle;
      }
      it = it2;
    }
  } else {
    for (i = 0; i < m_SimHits.getEntries(); i++) {
      const KLMSimHit* hit = m_SimHits[i];
      if (hit->inRPC()) {
        if (hit->getStrip() <= 0)
          continue;
        for (int s = hit->getStrip(); s <= hit->getLastStrip(); ++s) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hit->getSection(), hit->getSector(), hit->getLayer(),
                      hit->getPlane(), s);
          if (checkActive(channel)) {
            m_MapChannelSimHit.insert(
              std::pair<KLMChannelNumber, const KLMSimHit*>(channel, hit));
          }
        }
      } else {
        channel = m_ElementNumbers->channelNumber(
                    hit->getSubdetector(), hit->getSection(), hit->getSector(),
                    hit->getLayer(), hit->getPlane(), hit->getStrip());
        if (checkActive(channel)) {
          const KLMElectronicsChannel* electronicsChannel =
            m_ElectronicsMap->getElectronicsChannel(channel);
          if (electronicsChannel == nullptr)
            B2FATAL("Incomplete electronics map.");
          KLMElectronicsChannel asic = electronicsChannel->getAsic();
          m_MapAsicSimHit.insert(
            std::pair<KLMElectronicsChannel, const KLMSimHit*>(asic, hit));
        }
      }
    }
  }
  digitizeRPC();
  digitizeAsic();
}

void KLMDigitizerModule::endRun()
{
  delete m_Fitter;
}

void KLMDigitizerModule::terminate()
{
}
