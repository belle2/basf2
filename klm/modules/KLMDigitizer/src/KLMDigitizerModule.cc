/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMDigitizer/KLMDigitizerModule.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>
#include <klm/simulation/ScintillatorSimulator.h>

/* Belle 2 headers. */
#include <framework/dataobjects/BackgroundMetaData.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <TRandom.h>

using namespace Belle2;

REG_MODULE(KLMDigitizer)

KLMDigitizerModule::KLMDigitizerModule() :
  Module(),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance())),
  m_ChannelSpecificSimulation(false),
  m_EfficiencyMode(c_Plane),
  m_Fitter(nullptr)
{
  setDescription("KLM digitization module: create KLMDigits from BKLMSimHits and EKLMSimHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_SimulationMode,
           "Simulation mode (\"Generic\" or \"ChannelSpecific\").",
           std::string("Generic"));
  addParam("DigitizationInitialTime", m_DigitizationInitialTime,
           "Initial digitization time in TDC periods.", -40);
  addParam("SaveFPGAFit", m_SaveFPGAFit, "Save FPGA fit data and set a relation with KLMDigits.", false);
  addParam("Efficiency", m_Efficiency,
           "Efficiency determination mode (\"Strip\" or \"Plane\").",
           std::string("Plane"));
  addParam("Debug", m_Debug,
           "Debug mode (generates additional output files with histograms).",
           false);
}

KLMDigitizerModule::~KLMDigitizerModule()
{
}

void KLMDigitizerModule::initialize()
{
  m_bklmSimHits.isRequired();
  m_eklmSimHits.isRequired();
  m_Digits.registerInDataStore();
  m_Digits.registerRelationTo(m_bklmSimHits);
  m_Digits.registerRelationTo(m_eklmSimHits);
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
    uint16_t channel = m_ElementNumbers->channelNumber(klmChannel.getSubdetector(), klmChannel.getSection(),
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
  if (!m_TimeConversion.isValid())
    B2FATAL("KLM time conversion parameters are not available.");
  if (!m_ChannelStatus.isValid())
    B2FATAL("KLM channel status data are not available.");
  if (!m_StripEfficiency.isValid())
    B2FATAL("KLM strip efficiency data are not available.");
  if (m_ChannelSpecificSimulation)
    checkScintillatorFEEParameters();
  m_Fitter = new KLM::ScintillatorFirmware(m_DigPar->getNDigitizations());
}

/*
 * Digitization. Light propagation into the fiber, SiPM and electronics effects
 * are simulated in KLM::ScintillatorSimulator class.
 */

bool KLMDigitizerModule::checkActive(uint16_t channel)
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

void KLMDigitizerModule::digitizeBKLM()
{
  int tdc;
  KLM::ScintillatorSimulator simulator(&(*m_DigPar), m_Fitter, 0, false);
  const KLMScintillatorFEEData* FEEData;
  std::multimap<uint16_t, const BKLMSimHit*>::iterator it, it2, ub;
  for (it = m_bklmSimHitChannelMap.begin(); it != m_bklmSimHitChannelMap.end();
       it = m_bklmSimHitChannelMap.upper_bound(it->first)) {
    const BKLMSimHit* simHit = it->second;
    ub = m_bklmSimHitChannelMap.upper_bound(it->first);
    bool rpc = simHit->inRPC();
    if (m_EfficiencyMode == c_Strip) {
      float efficiency = m_StripEfficiency->getBarrelEfficiency(
                           simHit->getSection(), simHit->getSector(),
                           simHit->getLayer(), simHit->getPlane(),
                           simHit->getStrip());
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    if (rpc) {
      int strip = BKLMElementNumbers::getStripByModule(
                    m_ElementNumbers->localChannelNumberBKLM(it->first));
      /* Select hit that has the smallest time. */
      it2 = it;
      const BKLMSimHit* hit = it->second;
      double time = hit->getTime();
      ++it2;
      while (it2 != ub) {
        if (it2->second->getTime() < time) {
          time = it2->second->getTime();
          hit = it2->second;
        }
        ++it2;
      }
      KLMDigit* bklmDigit = m_Digits.appendNew(hit, strip);
      bklmDigit->addRelationTo(simHit);
    } else {
      if (m_ChannelSpecificSimulation) {
        uint16_t channel = m_ElementNumbers->channelNumber(KLMElementNumbers::c_EKLM, simHit->getSection(), simHit->getSector(),
                                                           simHit->getLayer(), simHit->getPlane(), simHit->getStrip());
        FEEData = m_FEEPar->getFEEData(channel);
        if (FEEData == nullptr)
          B2FATAL("Incomplete KLM scintillator FEE data.");
        simulator.setFEEData(FEEData);
      }
      simulator.simulate(it, ub);
      if (simulator.getNGeneratedPhotoelectrons() == 0)
        continue;
      KLMDigit* bklmDigit = m_Digits.appendNew(simHit);
      bklmDigit->addRelationTo(simHit);
      bklmDigit->setMCTime(simulator.getMCTime());
      bklmDigit->setSiPMMCTime(simulator.getSiPMMCTime());
      bklmDigit->setNGeneratedPhotoelectrons(simulator.getNGeneratedPhotoelectrons());
      if (simulator.getFitStatus() ==
          KLM::c_ScintillatorFirmwareSuccessfulFit) {
        tdc = simulator.getFPGAFit()->getStartTime();
        bklmDigit->setCharge(simulator.getFPGAFit()->getMinimalAmplitude());
      } else {
        tdc = 0;
        bklmDigit->setCharge(m_DigPar->getADCRange() - 1);
      }
      bklmDigit->setTDC(tdc);
      bklmDigit->setTime(m_TimeConversion->getTimeSimulation(tdc, true));
      bklmDigit->setFitStatus(simulator.getFitStatus());
      bklmDigit->setNPhotoelectrons(simulator.getNPhotoelectrons());
      bklmDigit->setEnergyDeposit(simulator.getEnergy());
      if (simulator.getFitStatus() == KLM::c_ScintillatorFirmwareSuccessfulFit &&
          m_SaveFPGAFit) {
        KLMScintillatorFirmwareFitResult* fit =
          m_FPGAFits.appendNew(*simulator.getFPGAFit());
        bklmDigit->addRelationTo(fit);
      }
    }
  }
}

void KLMDigitizerModule::digitizeEKLM()
{
  uint16_t tdc;
  KLM::ScintillatorSimulator simulator(
    &(*m_DigPar), m_Fitter,
    m_DigitizationInitialTime * m_TimeConversion->getTDCPeriod(), m_Debug);
  const KLMScintillatorFEEData* FEEData;
  std::multimap<uint16_t, const EKLMSimHit*>::iterator it, ub;
  for (it = m_eklmSimHitChannelMap.begin(); it != m_eklmSimHitChannelMap.end();
       it = m_eklmSimHitChannelMap.upper_bound(it->first)) {
    const EKLMSimHit* simHit = it->second;
    ub = m_eklmSimHitChannelMap.upper_bound(it->first);
    if (m_EfficiencyMode == c_Strip) {
      float efficiency = m_StripEfficiency->getEndcapEfficiency(simHit->getSection(), simHit->getSector(),
                                                                simHit->getLayer(), simHit->getPlane(),
                                                                simHit->getStrip());
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    if (m_ChannelSpecificSimulation) {
      uint16_t channel = m_ElementNumbers->channelNumber(KLMElementNumbers::c_EKLM, simHit->getSection(), simHit->getSector(),
                                                         simHit->getLayer(), simHit->getPlane(), simHit->getStrip());
      FEEData = m_FEEPar->getFEEData(channel);
      if (FEEData == nullptr)
        B2FATAL("Incomplete KLM scintillator FEE data.");
      simulator.setFEEData(FEEData);
    }
    /* Simulation for a strip. */
    simulator.simulate(it, ub);
    if (simulator.getNGeneratedPhotoelectrons() == 0)
      continue;
    KLMDigit* eklmDigit = m_Digits.appendNew(simHit);
    eklmDigit->addRelationTo(simHit);
    eklmDigit->setMCTime(simulator.getMCTime());
    eklmDigit->setSiPMMCTime(simulator.getSiPMMCTime());
    eklmDigit->setNGeneratedPhotoelectrons(simulator.getNGeneratedPhotoelectrons());
    if (simulator.getFitStatus() == KLM::c_ScintillatorFirmwareSuccessfulFit) {
      tdc = simulator.getFPGAFit()->getStartTime();
      eklmDigit->setCharge(simulator.getFPGAFit()->getMinimalAmplitude());
    } else {
      tdc = 0;
      eklmDigit->setCharge(m_DigPar->getADCRange() - 1);
    }
    eklmDigit->setTDC(tdc);
    eklmDigit->setTime(m_TimeConversion->getTimeSimulation(tdc, true));
    eklmDigit->setFitStatus(simulator.getFitStatus());
    eklmDigit->setNPhotoelectrons(simulator.getNPhotoelectrons());
    eklmDigit->setEnergyDeposit(simulator.getEnergy());
    if (simulator.getFitStatus() == KLM::c_ScintillatorFirmwareSuccessfulFit &&
        m_SaveFPGAFit) {
      KLMScintillatorFirmwareFitResult* fit =
        m_FPGAFits.appendNew(*simulator.getFPGAFit());
      eklmDigit->addRelationTo(fit);
    }
  }
}

void KLMDigitizerModule::event()
{
  int i;
  uint16_t channel;
  m_bklmSimHitChannelMap.clear();
  m_eklmSimHitChannelMap.clear();
  if (m_EfficiencyMode == c_Plane) {
    /* BKLM. */
    {
      m_bklmSimHitPlaneMap.clear();
      for (i = 0; i < m_bklmSimHits.getEntries(); i++) {
        const BKLMSimHit* hit = m_bklmSimHits[i];
        if (hit->getStripMin() <= 0)
          continue;
        const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
        /*
         * We do not simulate the plane efficiency for BKLMSimHits
         * from beam background because there are no MCParticles associated
         * to them.
         */
        if (particle != nullptr) {
          uint16_t plane = m_ElementNumbers->planeNumberBKLM(
                             hit->getSection(), hit->getSector(),
                             hit->getLayer(), hit->getPlane());
          m_bklmSimHitPlaneMap.insert(
            std::pair<uint16_t, const BKLMSimHit*>(plane, hit));
        } else {
          B2ASSERT("The BKLMSimHit is not related to any MCParticle and "
                   "it is also not a beam background hit.",
                   hit->getBackgroundTag() != BackgroundMetaData::bg_none);
          channel = m_ElementNumbers->channelNumberBKLM(
                      hit->getSection(), hit->getSector(), hit->getLayer(),
                      hit->getPlane(), hit->getStrip());
          if (checkActive(channel))
            m_bklmSimHitChannelMap.insert(
              std::pair<uint16_t, const BKLMSimHit*>(channel, hit));
        }
      }
      std::multimap<uint16_t, const BKLMSimHit*>::iterator it, it2;
      std::multimap<const MCParticle*, const BKLMSimHit*> particleHitMap;
      std::multimap<const MCParticle*, const BKLMSimHit*>::iterator
      itParticle, it2Particle;
      it = m_bklmSimHitPlaneMap.begin();
      while (it != m_bklmSimHitPlaneMap.end()) {
        particleHitMap.clear();
        it2 = it;
        while (true) {
          const BKLMSimHit* hit = it2->second;
          const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
          particleHitMap.insert(
            std::pair<const MCParticle*, const BKLMSimHit*>(particle, hit));
          ++it2;
          if (it2 == m_bklmSimHitPlaneMap.end())
            break;
          if (it2->first != it->first)
            break;
        }
        itParticle = particleHitMap.begin();
        while (itParticle != particleHitMap.end()) {
          it2Particle = itParticle;
          const BKLMSimHit* hit = it2Particle->second;
          float efficiency = m_StripEfficiency->getBarrelEfficiency(
                               hit->getSection(), hit->getSector(),
                               hit->getLayer(), hit->getPlane(),
                               hit->getStripMin());
          bool hitSelected = efficiencyCorrection(efficiency);
          while (true) {
            hit = it2Particle->second;
            if (hitSelected) {
              for (int s = hit->getStripMin(); s <= hit->getStripMax(); ++s) {
                channel = m_ElementNumbers->channelNumberBKLM(
                            hit->getSection(), hit->getSector(), hit->getLayer(),
                            hit->getPlane(), s);
                if (checkActive(channel)) {
                  m_bklmSimHitChannelMap.insert(
                    std::pair<uint16_t, const BKLMSimHit*>(channel, hit));
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
    }
    /* EKLM. */
    {
      m_eklmSimHitPlaneMap.clear();
      for (i = 0; i < m_eklmSimHits.getEntries(); i++) {
        const EKLMSimHit* hit = m_eklmSimHits[i];
        const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
        /*
         * We do not simulate the plane efficiency for EKLMSimHits
         * from beam background because there are no MCParticles
         * associated to them.
         */
        if (particle != nullptr) {
          uint16_t plane = m_ElementNumbers->planeNumberEKLM(
                             hit->getSection(), hit->getSector(),
                             hit->getLayer(), hit->getPlane());
          m_eklmSimHitPlaneMap.insert(
            std::pair<uint16_t, const EKLMSimHit*>(plane, hit));
        } else {
          B2ASSERT("The EKLMSimHit is not related to any MCParticle and "
                   "it is also not a beam background hit.",
                   hit->getBackgroundTag() != BackgroundMetaData::bg_none);
          channel = m_ElementNumbers->channelNumberEKLM(
                      hit->getSection(), hit->getSector(), hit->getLayer(),
                      hit->getPlane(), hit->getStrip());
          if (checkActive(channel))
            m_eklmSimHitChannelMap.insert(
              std::pair<uint16_t, const EKLMSimHit*>(channel, hit));
        }
      }
      std::multimap<uint16_t, const EKLMSimHit*>::iterator it, it2;
      std::multimap<const MCParticle*, const EKLMSimHit*> particleHitMap;
      std::multimap<const MCParticle*, const EKLMSimHit*>::iterator
      itParticle, it2Particle;
      it = m_eklmSimHitPlaneMap.begin();
      while (it != m_eklmSimHitPlaneMap.end()) {
        particleHitMap.clear();
        it2 = it;
        while (true) {
          const EKLMSimHit* hit = it2->second;
          const MCParticle* particle = hit->getRelatedFrom<MCParticle>();
          particleHitMap.insert(
            std::pair<const MCParticle*, const EKLMSimHit*>(particle, hit));
          ++it2;
          if (it2 == m_eklmSimHitPlaneMap.end())
            break;
          if (it2->first != it->first)
            break;
        }
        itParticle = particleHitMap.begin();
        while (itParticle != particleHitMap.end()) {
          it2Particle = itParticle;
          const EKLMSimHit* hit = it2Particle->second;
          float efficiency = m_StripEfficiency->getEndcapEfficiency(
                               hit->getSection(), hit->getSector(),
                               hit->getLayer(), hit->getPlane(),
                               hit->getStrip());
          bool hitSelected = efficiencyCorrection(efficiency);
          while (true) {
            hit = it2Particle->second;
            if (hitSelected) {
              channel = m_ElementNumbers->channelNumberEKLM(
                          hit->getSection(), hit->getSector(), hit->getLayer(),
                          hit->getPlane(), hit->getStrip());
              if (checkActive(channel)) {
                m_eklmSimHitChannelMap.insert(
                  std::pair<uint16_t, const EKLMSimHit*>(channel, hit));
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
    }
  } else {
    for (i = 0; i < m_bklmSimHits.getEntries(); i++) {
      const BKLMSimHit* hit = m_bklmSimHits[i];
      if (hit->inRPC()) {
        if (hit->getStripMin() <= 0)
          continue;
        for (int s = hit->getStripMin(); s <= hit->getStripMax(); ++s) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hit->getSection(), hit->getSector(), hit->getLayer(),
                      hit->getPlane(), s);
          if (checkActive(channel)) {
            m_bklmSimHitChannelMap.insert(
              std::pair<uint16_t, const BKLMSimHit*>(channel, hit));
          }
        }
      } else {
        channel = m_ElementNumbers->channelNumberBKLM(
                    hit->getSection(), hit->getSector(), hit->getLayer(),
                    hit->getPlane(), hit->getStrip());
        if (checkActive(channel)) {
          m_bklmSimHitChannelMap.insert(
            std::pair<uint16_t, const BKLMSimHit*>(channel, hit));
        }
      }
    }
    for (i = 0; i < m_eklmSimHits.getEntries(); i++) {
      const EKLMSimHit* hit = m_eklmSimHits[i];
      channel = m_ElementNumbers->channelNumberEKLM(
                  hit->getSection(), hit->getSector(), hit->getLayer(),
                  hit->getPlane(), hit->getStrip());
      if (checkActive(channel)) {
        m_eklmSimHitChannelMap.insert(
          std::pair<uint16_t, const EKLMSimHit*>(channel, hit));
      }
    }
  }
  digitizeBKLM();
  digitizeEKLM();
}

void KLMDigitizerModule::endRun()
{
  delete m_Fitter;
}

void KLMDigitizerModule::terminate()
{
}
