/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMFPGAFit.h>
#include <eklm/modules/EKLMDigitizer/EKLMDigitizerModule.h>
#include <eklm/simulation/FiberAndElectronics.h>

using namespace Belle2;

REG_MODULE(EKLMDigitizer)

EKLMDigitizerModule::EKLMDigitizerModule() : Module(),
  m_ChannelSpecificSimulation(false)
{
  setDescription("EKLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_SimulationMode,
           "Simulation mode (\"Generic\" or \"ChannelSpecific\")",
           std::string("Generic"));
  addParam("DigitizationInitialTime", m_DigitizationInitialTime,
           "Initial digitization time (ns).", double(-40.));
  addParam("SaveFPGAFit", m_SaveFPGAFit, "Save FPGA fit data", false);
  addParam("Debug", m_Debug,
           "Debug mode (generates additional output files with histograms).",
           false);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_Fitter = nullptr;
}

EKLMDigitizerModule::~EKLMDigitizerModule()
{
}

void EKLMDigitizerModule::initialize()
{
  m_Digits.registerInDataStore();
  m_Digits.registerRelationTo(m_SimHits);
  if (m_SaveFPGAFit) {
    m_FPGAFits.registerInDataStore();
    m_Digits.registerRelationTo(m_FPGAFits);
  }
  m_Fitter = new EKLM::FPGAFitter(m_DigPar->getNDigitizations());
  if (m_SimulationMode == "Generic") {
    /* Nothing to do. */
  } else if (m_SimulationMode == "ChannelSpecific") {
    m_ChannelSpecificSimulation = true;
  } else {
    B2FATAL("Unknown simulation mode: " << m_SimulationMode);
  }
}

void EKLMDigitizerModule::checkChannelParameters()
{
  int endcap, layer, sector, plane, strip, stripGlobal;
  int nEndcaps, nLayers[2], nSectors, nPlanes, nStrips;
  const EKLMChannelData* channel;
  nEndcaps = m_ElementNumbers->getMaximalEndcapNumber();
  nLayers[0] = m_ElementNumbers->getMaximalDetectorLayerNumber(1);
  nLayers[1] = m_ElementNumbers->getMaximalDetectorLayerNumber(2);
  nSectors = m_ElementNumbers->getMaximalSectorNumber();
  nPlanes = m_ElementNumbers->getMaximalPlaneNumber();
  nStrips = m_ElementNumbers->getMaximalStripNumber();
  for (endcap = 1; endcap <= nEndcaps; endcap++) {
    for (layer = 1; layer <= nLayers[endcap - 1]; layer++) {
      for (sector = 1; sector <= nSectors; sector++) {
        for (plane = 1; plane <= nPlanes; plane++) {
          for (strip = 1; strip <= nStrips; strip++) {
            stripGlobal = m_ElementNumbers->stripNumber(endcap, layer, sector,
                                                        plane, strip);
            channel = m_Channels->getChannelData(stripGlobal);
            if (channel == nullptr)
              B2FATAL("Incomplete channel data.");
            if (channel->getPhotoelectronAmplitude() <= 0) {
              B2ERROR("Non-positive photoelectron amplitude. The requested "
                      "channel-specific simulation is impossible. "
                      "EKLMDigitizer is switched to the generic mode."
                      << LogVar("Endcap", endcap) << LogVar("Layer", layer)
                      << LogVar("Sector", sector) << LogVar("Plane", plane)
                      << LogVar("Strip", strip));
              m_ChannelSpecificSimulation = false;
              return;
            }
          }
        }
      }
    }
  }
}

void EKLMDigitizerModule::beginRun()
{
  if (!m_DigPar.isValid())
    B2FATAL("EKLM digitization parameters are not available.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  if (m_ChannelSpecificSimulation)
    checkChannelParameters();
}

void EKLMDigitizerModule::readAndSortSimHits()
{
  EKLMSimHit* hit;
  int i, strip, maxStrip;
  maxStrip = m_ElementNumbers->getMaximalStripGlobalNumber();
  m_SimHitVolumeMap.clear();
  for (i = 0; i < m_SimHits.getEntries(); i++) {
    hit = m_SimHits[i];
    strip = hit->getVolumeID();
    if (strip <= 0)
      B2FATAL("Incorrect (non-positive) strip number in EKLM digitizer.");
    /* Background study mode: ignore hits from SiPMs and boards. */
    if (strip > maxStrip)
      continue;
    m_SimHitVolumeMap.insert(std::pair<int, EKLMSimHit*>(strip, hit));
  }
}

/*
 * Light propagation into the fiber, SiPM and electronics effects
 * are simulated in EKLM::FiberAndElectronics class.
 */
void EKLMDigitizerModule::mergeSimHitsToStripHits()
{
  uint16_t tdc;
  int strip;
  EKLM::FiberAndElectronics fes(&(*m_DigPar), m_Fitter,
                                m_DigitizationInitialTime, m_Debug);
  const EKLMChannelData* channelData;
  std::multimap<int, EKLMSimHit*>::iterator it, ub;
  for (it = m_SimHitVolumeMap.begin(); it != m_SimHitVolumeMap.end();
       it = m_SimHitVolumeMap.upper_bound(it->first)) {
    EKLMSimHit* simHit = it->second;
    ub = m_SimHitVolumeMap.upper_bound(it->first);
    /* Set hits. */
    fes.setHitRange(it, ub);
    if (m_ChannelSpecificSimulation) {
      strip = m_ElementNumbers->stripNumber(
                simHit->getEndcap(), simHit->getLayer(), simHit->getSector(),
                simHit->getPlane(), simHit->getStrip());
      channelData = m_Channels->getChannelData(strip);
      if (channelData == nullptr)
        B2FATAL("Incomplete EKLM channel data.");
      fes.setChannelData(channelData);
    }
    /* Simulation for a strip. */
    fes.processEntry();
    if (fes.getGeneratedNPE() == 0)
      continue;
    EKLMDigit* eklmDigit = m_Digits.appendNew(simHit);
    eklmDigit->setMCTime(simHit->getTime());
    eklmDigit->setSiPMMCTime(fes.getMCTime());
    eklmDigit->setPosition(simHit->getPosition());
    eklmDigit->setGeneratedNPE(fes.getGeneratedNPE());
    eklmDigit->addRelationTo(simHit);
    if (fes.getFitStatus() == EKLM::c_FPGASuccessfulFit) {
      tdc = fes.getFPGAFit()->getStartTime();
      eklmDigit->setCharge(fes.getFPGAFit()->getMinimalAmplitude());
    } else {
      tdc = 0;
      eklmDigit->setCharge(0);
    }
    eklmDigit->setTDC(tdc);
    eklmDigit->setTime(m_TimeConversion->getTimeByTDC(tdc));
    eklmDigit->setFitStatus(fes.getFitStatus());
    if (fes.getFitStatus() == EKLM::c_FPGASuccessfulFit && m_SaveFPGAFit) {
      EKLMFPGAFit* fit = m_FPGAFits.appendNew(*fes.getFPGAFit());
      eklmDigit->addRelationTo(fit);
    }
  }
}

void EKLMDigitizerModule::event()
{
  readAndSortSimHits();
  mergeSimHitsToStripHits();
}

void EKLMDigitizerModule::endRun()
{
}

void EKLMDigitizerModule::terminate()
{
  delete m_Fitter;
}
