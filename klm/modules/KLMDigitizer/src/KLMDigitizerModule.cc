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
#include <framework/core/RandomNumbers.h>
#include <framework/dataobjects/BackgroundMetaData.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

REG_MODULE(KLMDigitizer)

KLMDigitizerModule::KLMDigitizerModule() : Module(),
  m_ChannelSpecificSimulation(false),
  m_EfficiencyMode(c_Plane)
{
  setDescription("KLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_SimulationMode,
           "Simulation mode (\"Generic\" or \"ChannelSpecific\")",
           std::string("Generic"));
  addParam("DigitizationInitialTime", m_DigitizationInitialTime,
           "Initial digitization time (ns).", double(-40.));
  addParam("SaveFPGAFit", m_SaveFPGAFit, "Save FPGA fit data", false);
  addParam("Efficiency", m_Efficiency,
           "Efficiency determination mode (\"Strip\" or \"Plane\")",
           std::string("Plane"));
  addParam("Debug", m_Debug,
           "Debug mode (generates additional output files with histograms).",
           false);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_eklmElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_Fitter = nullptr;
}

KLMDigitizerModule::~KLMDigitizerModule()
{
}

void KLMDigitizerModule::initialize()
{
  m_bklmSimHits.isRequired();
  m_eklmSimHits.isRequired();
  m_bklmDigits.registerInDataStore();
  m_bklmDigits.registerRelationTo(m_bklmSimHits);
  m_eklmDigits.registerInDataStore();
  m_eklmDigits.registerRelationTo(m_eklmSimHits);
  if (m_SaveFPGAFit) {
    m_FPGAFits.registerInDataStore();
    m_bklmDigits.registerRelationTo(m_FPGAFits);
    m_eklmDigits.registerRelationTo(m_FPGAFits);
  }
  m_Fitter = new KLM::ScintillatorFirmware(m_DigPar->getNDigitizations());
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

void KLMDigitizerModule::checkChannelParameters()
{
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex eklmChannel = klmChannels.beginEKLM();
       eklmChannel != klmChannels.endEKLM(); ++eklmChannel) {
    int stripGlobal = m_eklmElementNumbers->stripNumber(
                        eklmChannel.getSection(), eklmChannel.getLayer(),
                        eklmChannel.getSector(), eklmChannel.getPlane(),
                        eklmChannel.getStrip());
    const EKLMChannelData* channel = m_Channels->getChannelData(stripGlobal);
    if (channel == nullptr)
      B2FATAL("Incomplete channel data.");
    if (channel->getPhotoelectronAmplitude() <= 0) {
      B2ERROR("Non-positive photoelectron amplitude. The requested "
              "channel-specific simulation is impossible. "
              "KLMDigitizer is switched to the generic mode."
              << LogVar("Section", eklmChannel.getSection())
              << LogVar("Layer", eklmChannel.getLayer())
              << LogVar("Sector", eklmChannel.getSector())
              << LogVar("Plane", eklmChannel.getPlane())
              << LogVar("Strip", eklmChannel.getStrip()));
      m_ChannelSpecificSimulation = false;
      return;
    }
  }
}

void KLMDigitizerModule::beginRun()
{
  if (!m_DigPar.isValid())
    B2FATAL("KLM digitization parameters are not available.");
  if (!m_TimeConversion.isValid())
    B2FATAL("KLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  if (!m_ChannelStatus.isValid())
    B2FATAL("KLM channel status data are not available.");
  if (!m_StripEfficiency.isValid())
    B2FATAL("KLM strip efficiency data are not available.");
  if (m_ChannelSpecificSimulation)
    checkChannelParameters();
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
  return (gRandom->Rndm() < efficiency);
}

void KLMDigitizerModule::digitizeBKLM()
{
  int tdc;
  KLM::ScintillatorSimulator simulator(&(*m_DigPar), m_Fitter, 0, false);
  std::multimap<uint16_t, const BKLMSimHit*>::iterator it, ub;
  for (it = m_bklmSimHitChannelMap.begin(); it != m_bklmSimHitChannelMap.end();
       it = m_bklmSimHitChannelMap.upper_bound(it->first)) {
    const BKLMSimHit* simHit = it->second;
    ub = m_bklmSimHitChannelMap.upper_bound(it->first);
    float efficiency = m_StripEfficiency->getBarrelEfficiency(
                         simHit->getSection(), simHit->getSector(),
                         simHit->getLayer(), simHit->getPlane(),
                         simHit->getStrip());
    bool rpc = simHit->inRPC();
    if (m_EfficiencyMode == c_Strip) {
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    if (rpc) {
      int strip = BKLMElementNumbers::getStripByModule(
                    m_ElementNumbers->localChannelNumberBKLM(it->first));
      BKLMDigit* bklmDigit = m_bklmDigits.appendNew(simHit, strip);
      bklmDigit->addRelationTo(simHit);
    } else {
      simulator.simulate(it, ub);
      if (simulator.getGeneratedNPE() == 0)
        continue;
      BKLMDigit* bklmDigit = m_bklmDigits.appendNew(simHit);
      bklmDigit->addRelationTo(simHit);
      // Not implemented in BKLMDigit.
      // eklmDigit->setMCTime(simHit->getTime());
      // eklmDigit->setSiPMMCTime(simulator.getMCTime());
      // eklmDigit->setPosition(simHit->getPosition());
      bklmDigit->setSimNPixel(simulator.getGeneratedNPE());
      if (simulator.getFitStatus() ==
          KLM::c_ScintillatorFirmwareSuccessfulFit) {
        tdc = simulator.getFPGAFit()->getStartTime();
        /* Differs from original BKLM definition! */
        bklmDigit->setCharge(simulator.getFPGAFit()->getMinimalAmplitude());
        bklmDigit->isAboveThreshold(true);
      } else {
        tdc = 0;
        bklmDigit->setCharge(m_DigPar->getADCRange() - 1);
        bklmDigit->isAboveThreshold(false);
      }
      // Not implemented in BKLMDigit.
      // eklmDigit->setTDC(tdc);
      bklmDigit->setTime(m_TimeConversion->getTimeSimulation(tdc, true));
      bklmDigit->setFitStatus(simulator.getFitStatus());
      bklmDigit->setNPixel(simulator.getNPE());
      bklmDigit->setEDep(simulator.getEnergy());
    }
  }
}

void KLMDigitizerModule::digitizeEKLM()
{
  uint16_t tdc;
  int strip;
  KLM::ScintillatorSimulator simulator(&(*m_DigPar), m_Fitter,
                                       m_DigitizationInitialTime, m_Debug);
  const EKLMChannelData* channelData;
  std::multimap<uint16_t, const EKLMSimHit*>::iterator it, ub;
  for (it = m_eklmSimHitChannelMap.begin(); it != m_eklmSimHitChannelMap.end();
       it = m_eklmSimHitChannelMap.upper_bound(it->first)) {
    const EKLMSimHit* simHit = it->second;
    ub = m_eklmSimHitChannelMap.upper_bound(it->first);
    float efficiency = m_StripEfficiency->getEndcapEfficiency(
                         simHit->getSection(), simHit->getSector(),
                         simHit->getLayer(), simHit->getPlane(),
                         simHit->getStrip());
    if (m_EfficiencyMode == c_Strip) {
      if (!efficiencyCorrection(efficiency))
        continue;
    }
    if (m_ChannelSpecificSimulation) {
      strip = m_eklmElementNumbers->stripNumber(
                simHit->getSection(), simHit->getLayer(), simHit->getSector(),
                simHit->getPlane(), simHit->getStrip());
      channelData = m_Channels->getChannelData(strip);
      if (channelData == nullptr)
        B2FATAL("Incomplete EKLM channel data.");
      simulator.setChannelData(channelData);
    }
    /* Simulation for a strip. */
    simulator.simulate(it, ub);
    if (simulator.getGeneratedNPE() == 0)
      continue;
    EKLMDigit* eklmDigit = m_eklmDigits.appendNew(simHit);
    eklmDigit->addRelationTo(simHit);
    eklmDigit->setMCTime(simHit->getTime());
    eklmDigit->setSiPMMCTime(simulator.getMCTime());
    eklmDigit->setPosition(simHit->getPosition());
    eklmDigit->setGeneratedNPE(simulator.getGeneratedNPE());
    eklmDigit->setEDep(simulator.getEnergy());
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
        /* We do not simulate the plane efficiency for BKLMSimHits from beam background
         * because there are no MCParticles associated to them. */
        if (particle != nullptr) {
          uint16_t plane = m_ElementNumbers->planeNumberBKLM(
                             hit->getSection(), hit->getSector(), hit->getLayer(),
                             hit->getPlane());
          m_bklmSimHitPlaneMap.insert(
            std::pair<uint16_t, const BKLMSimHit*>(plane, hit));
        } else {
          if (hit->getBackgroundTag() != BackgroundMetaData::bg_none) {
            channel = m_ElementNumbers->channelNumberBKLM(
                        hit->getSection(), hit->getSector(), hit->getLayer(),
                        hit->getPlane(), hit->getStrip());
            if (checkActive(channel))
              m_bklmSimHitChannelMap.insert(
                std::pair<uint16_t, const BKLMSimHit*>(channel, hit));
          } else
            B2FATAL("No MCParticle is related to BKLMSimHit.");
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
        /* We do not simulate the plane efficiency for EKLMSimHits from beam background
         * because there are no MCParticles associated to them. */
        if (particle != nullptr) {
          uint16_t plane = m_ElementNumbers->planeNumberEKLM(
                             hit->getSection(), hit->getSector(), hit->getLayer(),
                             hit->getPlane());
          m_eklmSimHitPlaneMap.insert(
            std::pair<uint16_t, const EKLMSimHit*>(plane, hit));
        } else {
          if (hit->getBackgroundTag() != BackgroundMetaData::bg_none) {
            channel = m_ElementNumbers->channelNumberEKLM(
                        hit->getSection(), hit->getSector(), hit->getLayer(),
                        hit->getPlane(), hit->getStrip());
            if (checkActive(channel))
              m_eklmSimHitChannelMap.insert(
                std::pair<uint16_t, const EKLMSimHit*>(channel, hit));
          } else
            B2FATAL("No MCParticle is related to EKLMSimHit.");
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
}

void KLMDigitizerModule::terminate()
{
  delete m_Fitter;
}
