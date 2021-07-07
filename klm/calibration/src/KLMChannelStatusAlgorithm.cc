/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMChannelStatusAlgorithm.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TTree.h>

using namespace Belle2;

KLMChannelStatusAlgorithm::Results::Results()
{
}

KLMChannelStatusAlgorithm::Results::Results(const Results& results)
{
  m_ModuleStatus = new KLMChannelStatus(*results.m_ModuleStatus);
  m_ChannelStatus = new KLMChannelStatus(*results.m_ChannelStatus);
  m_HitMapChannel = results.m_HitMapChannel;
  m_HitMapModule = results.m_HitMapModule;
  m_HitMapSector = results.m_HitMapSector;
  m_HitMapModuleNoHot = results.m_HitMapModuleNoHot;
  m_HitMapSectorNoHot = results.m_HitMapSectorNoHot;
  m_ModuleActiveChannelMap = results.m_ModuleActiveChannelMap;
  m_TotalHitNumber = results.m_TotalHitNumber;
  m_HitNumberBKLM = results.m_HitNumberBKLM;
  m_HitNumberEKLM = results.m_HitNumberEKLM;
  m_HitNumberBKLMNoHot = results.m_HitNumberBKLMNoHot;
  m_HitNumberEKLMNoHot = results.m_HitNumberEKLMNoHot;
}

KLMChannelStatusAlgorithm::Results::~Results()
{
  if (m_ModuleStatus != nullptr)
    delete m_ModuleStatus;
  if (m_ChannelStatus != nullptr)
    delete m_ChannelStatus;
}

KLMChannelStatusAlgorithm::KLMChannelStatusAlgorithm() :
  CalibrationAlgorithm("KLMChannelStatusCollector"),
  m_ElementNumbers(&(KLMElementNumbers::Instance()))
{
}

KLMChannelStatusAlgorithm::~KLMChannelStatusAlgorithm()
{
}

CalibrationAlgorithm::EResult KLMChannelStatusAlgorithm::calibrate()
{
  KLMChannelNumber channel, module, sector;
  unsigned int hits, moduleHits, maxHits;
  /*
   * Fill channel hit map. Note that more than one entry can exist for the same
   * channel due to merging of collected data for several runs. Thus, the
   * number of hits is summed.
   */
  std::shared_ptr<TTree> calibrationData;
  calibrationData = getObjectPtr<TTree>("calibration_data");
  calibrationData->SetBranchAddress("channel", &channel);
  calibrationData->SetBranchAddress("hits", &hits);
  int n = calibrationData->GetEntries();
  m_Results.m_HitMapChannel.setDataAllChannels(0);
  m_Results.m_TotalHitNumber = 0;
  m_Results.m_HitNumberBKLM = 0;
  m_Results.m_HitNumberEKLM = 0;
  for (int i = 0; i < n; ++i) {
    calibrationData->GetEntry(i);
    m_Results.m_HitMapChannel.setChannelData(
      channel, m_Results.m_HitMapChannel.getChannelData(channel) + hits);
    m_Results.m_TotalHitNumber += hits;
    if (m_ElementNumbers->isBKLMChannel(channel))
      m_Results.m_HitNumberBKLM += hits;
    else
      m_Results.m_HitNumberEKLM += hits;
  }
  clearCalibrationData();
  /*
   * A new object is created, because saveCalibration() stores a pointer
   * to KLMChannelStatus, and it is necessary to save the payloads to commit
   * them at the end of calibration.
   */
  m_Results.m_ChannelStatus = new KLMChannelStatus();
  m_Results.m_ChannelStatus->setStatusAllChannels(KLMChannelStatus::c_Undetermined);
  /* If there are no hits, then mark all channels as dead. */
  KLMChannelIndex klmChannels;
  if (m_Results.m_TotalHitNumber == 0) {
    for (KLMChannelIndex& klmChannel : klmChannels) {
      channel = klmChannel.getKLMChannelNumber();
      m_Results.m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
    }
    saveCalibration(m_Results.m_ChannelStatus, "KLMChannelStatus");
    return CalibrationAlgorithm::c_OK;
  }
  /* Fill module and sector hit maps. */
  KLMChannelIndex klmModules(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex& klmModule : klmModules)
    m_Results.m_HitMapModule.setChannelData(klmModule.getKLMModuleNumber(), 0);
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors)
    m_Results.m_HitMapSector.setChannelData(klmSector.getKLMSectorNumber(), 0);
  for (KLMChannelIndex& klmChannel : klmChannels) {
    channel = klmChannel.getKLMChannelNumber();
    module = klmChannel.getKLMModuleNumber();
    sector = klmChannel.getKLMSectorNumber();
    hits = m_Results.m_HitMapChannel.getChannelData(channel);
    m_Results.m_HitMapModule.setChannelData(
      module, m_Results.m_HitMapModule.getChannelData(module) + hits);
    m_Results.m_HitMapSector.setChannelData(
      sector, m_Results.m_HitMapSector.getChannelData(sector) + hits);
  }
  /*
   * Mark all channels in modules without hits as dead.
   * Search for hot channels.
   */
  for (KLMChannelIndex& klmModule : klmModules) {
    module = klmModule.getKLMModuleNumber();
    moduleHits = m_Results.m_HitMapModule.getChannelData(module);
    KLMChannelIndex klmNextModule(klmModule);
    ++klmNextModule;
    KLMChannelIndex klmChannel(klmModule);
    klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; klmChannel != klmNextModule; ++klmChannel) {
        channel = klmChannel.getKLMChannelNumber();
        m_Results.m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      m_Results.m_ModuleActiveChannelMap.setChannelData(module, 0);
      continue;
    }
    unsigned int activeChannels = 0;
    for (; klmChannel != klmNextModule; ++klmChannel) {
      channel = klmChannel.getKLMChannelNumber();
      hits = m_Results.m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    m_Results.m_ModuleActiveChannelMap.setChannelData(module, activeChannels);
    KLMChannelIndex klmChannelMaxHits;
    while (1) {
      klmChannel = klmModule;
      klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
      maxHits = 0;
      for (; klmChannel != klmNextModule; ++klmChannel) {
        channel = klmChannel.getKLMChannelNumber();
        if (m_Results.m_ChannelStatus->getChannelStatus(channel) ==
            KLMChannelStatus::c_Hot)
          continue;
        hits = m_Results.m_HitMapChannel.getChannelData(channel);
        if (hits > maxHits) {
          klmChannelMaxHits = klmChannel;
          maxHits = hits;
        }
      }
      if (maxHits == 0)
        break;
      if (!markHotChannel(klmChannelMaxHits.getKLMChannelNumber(),
                          moduleHits, activeChannels))
        break;
      moduleHits -= maxHits;
      activeChannels--;
    }
  }
  /* Fill module and sector hit maps with hot channels subtracted. */
  for (KLMChannelIndex& klmModule : klmModules)
    m_Results.m_HitMapModuleNoHot.setChannelData(klmModule.getKLMModuleNumber(), 0);
  for (KLMChannelIndex& klmSector : klmSectors)
    m_Results.m_HitMapSectorNoHot.setChannelData(klmSector.getKLMSectorNumber(), 0);
  m_Results.m_HitNumberBKLMNoHot = 0;
  m_Results.m_HitNumberEKLMNoHot = 0;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    channel = klmChannel.getKLMChannelNumber();
    if (m_Results.m_ChannelStatus->getChannelStatus(channel) == KLMChannelStatus::c_Hot)
      continue;
    module = klmChannel.getKLMModuleNumber();
    sector = klmChannel.getKLMSectorNumber();
    hits = m_Results.m_HitMapChannel.getChannelData(channel);
    m_Results.m_HitMapModuleNoHot.setChannelData(
      module, m_Results.m_HitMapModuleNoHot.getChannelData(module) + hits);
    m_Results.m_HitMapSectorNoHot.setChannelData(
      sector, m_Results.m_HitMapSectorNoHot.getChannelData(sector) + hits);
    if (m_ElementNumbers->isBKLMChannel(channel))
      m_Results.m_HitNumberBKLMNoHot += hits;
    else
      m_Results.m_HitNumberEKLMNoHot += hits;
  }
  /* Sector status. */
  int activeSectorsBKLM = 0;
  int activeSectorsEKLM = 0;
  for (KLMChannelIndex& klmSector : klmSectors) {
    sector = klmSector.getKLMSectorNumber();
    hits = m_Results.m_HitMapSectorNoHot.getChannelData(sector);
    if (hits > 0) {
      if (m_ElementNumbers->isBKLMChannel(sector))
        activeSectorsBKLM++;
      else
        activeSectorsEKLM++;
    }
  }
  double averageHitsActiveSector = 0;
  if (activeSectorsBKLM > 0) {
    averageHitsActiveSector = double(m_Results.m_HitNumberBKLMNoHot) /
                              activeSectorsBKLM;
  }
  for (KLMChannelIndex bklmSector = klmSectors.beginBKLM();
       bklmSector != klmSectors.endBKLM(); ++bklmSector) {
    sector = bklmSector.getKLMSectorNumber();
    calibrateSector(sector, averageHitsActiveSector);
  }
  if (activeSectorsEKLM > 0) {
    averageHitsActiveSector = double(m_Results.m_HitNumberEKLMNoHot) /
                              activeSectorsBKLM;
  }
  for (KLMChannelIndex eklmSector = klmSectors.beginEKLM();
       eklmSector != klmSectors.endEKLM(); ++eklmSector) {
    sector = eklmSector.getKLMSectorNumber();
    calibrateSector(sector, averageHitsActiveSector);
  }
  /* Module status. */
  if (m_Results.m_ModuleStatus == nullptr)
    m_Results.m_ModuleStatus = new KLMChannelStatus();
  for (KLMChannelIndex& klmModule : klmModules)
    calibrateModule(klmModule.getKLMModuleNumber());
  /* Channel-based calibration. */
  bool notEnoughData = false;
  for (KLMChannelIndex& klmModule : klmModules) {
    module = klmModule.getKLMModuleNumber();
    moduleHits = m_Results.m_HitMapModuleNoHot.getChannelData(module);
    KLMChannelIndex klmNextModule(klmModule);
    ++klmNextModule;
    KLMChannelIndex klmChannel(klmModule);
    klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0)
      continue;
    unsigned int activeChannels = 0;
    for (; klmChannel != klmNextModule; ++klmChannel) {
      channel = klmChannel.getKLMChannelNumber();
      if (m_Results.m_ChannelStatus->getChannelStatus(channel) == KLMChannelStatus::c_Hot)
        continue;
      hits = m_Results.m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber && !m_ForcedCalibration) {
      if (!notEnoughData) {
        B2INFO("KLM module " << klmModule.getKLMModuleNumber() <<
               " (subdetector " << klmModule.getSubdetector() <<
               ", section " << klmModule.getSection() <<
               ", sector " << klmModule.getSector() <<
               ", layer " << klmModule.getLayer() <<
               "): " << moduleHits << " hits, average: " << averageHits << ".");
      }
      notEnoughData = true;
      continue;
    }
    klmChannel = klmModule;
    klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
    for (; klmChannel != klmNextModule; ++klmChannel)
      calibrateChannel(klmChannel.getKLMChannelNumber());
  }
  if (notEnoughData)
    return CalibrationAlgorithm::c_NotEnoughData;
  saveCalibration(m_Results.m_ChannelStatus, "KLMChannelStatus");
  return CalibrationAlgorithm::c_OK;
}

void KLMChannelStatusAlgorithm::calibrateSector(
  KLMSectorNumber sector, double averageHitsActiveSector)
{
  unsigned int hits = m_Results.m_HitMapSectorNoHot.getChannelData(sector);
  if (hits == 0)
    return;
  double r = log(hits / averageHitsActiveSector) / log(10.);
  if (fabs(r) > m_MaximalLogSectorHitsRatio) {
    B2WARNING("Number of hits in sector " << sector << " (" << hits <<
              ") strongly deviates from the average (" <<
              averageHitsActiveSector <<
              "), the 10-based logarithm of the ratio is " << r << ".");
  }
}

void KLMChannelStatusAlgorithm::calibrateModule(KLMModuleNumber module)
{
  unsigned int hits = m_Results.m_HitMapModule.getChannelData(module);
  if (hits >= m_MinimalModuleHitNumber) {
    m_Results.m_ModuleStatus->setChannelStatus(
      module, KLMChannelStatus::c_Normal);
  } else {
    m_Results.m_ModuleStatus->setChannelStatus(
      module, KLMChannelStatus::c_Dead);
  }
}

bool KLMChannelStatusAlgorithm::markHotChannel(
  KLMChannelNumber channel, unsigned int moduleHits, int activeChannels)
{
  unsigned int hits = m_Results.m_HitMapChannel.getChannelData(channel);
  if (activeChannels == 1) {
    if (hits >= m_MinimalHitNumberSingleHotChannel) {
      m_Results.m_ChannelStatus->setChannelStatus(
        channel, KLMChannelStatus::c_Hot);
      return true;
    }
  } else {
    double r = hits / (double(moduleHits - hits) / (activeChannels - 1));
    if (hits >= m_MinimalHitNumberHotChannel &&
        r > m_MinimalHitNumberRatioHotChannel) {
      m_Results.m_ChannelStatus->setChannelStatus(
        channel, KLMChannelStatus::c_Hot);
      return true;
    }
  }
  return false;
}

void KLMChannelStatusAlgorithm::calibrateChannel(KLMChannelNumber channel)
{
  unsigned int hits = m_Results.m_HitMapChannel.getChannelData(channel);
  if (m_Results.m_ChannelStatus->getChannelStatus(channel) ==
      KLMChannelStatus::c_Hot)
    return;
  if (hits > 0) {
    m_Results.m_ChannelStatus->setChannelStatus(
      channel, KLMChannelStatus::c_Normal);
  } else {
    m_Results.m_ChannelStatus->setChannelStatus(
      channel, KLMChannelStatus::c_Dead);
  }
}
