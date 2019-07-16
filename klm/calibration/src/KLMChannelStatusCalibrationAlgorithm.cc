/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TTree.h>

/* Belle2 headers. */
#include <framework/logging/Logger.h>
#include <klm/calibration/KLMChannelStatusCalibrationAlgorithm.h>

using namespace Belle2;

KLMChannelStatusCalibrationAlgorithm::KLMChannelStatusCalibrationAlgorithm() :
  CalibrationAlgorithm("KLMChannelStatusCalibrationCollector")
{
}

KLMChannelStatusCalibrationAlgorithm::~KLMChannelStatusCalibrationAlgorithm()
{
  if (m_ModuleStatus != nullptr)
    delete m_ModuleStatus;
}

CalibrationAlgorithm::EResult KLMChannelStatusCalibrationAlgorithm::calibrate()
{
  uint16_t channel, module, sector;
  unsigned int hits, moduleHits;
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
  m_HitMapChannel.setDataAllChannels(0);
  m_TotalHitNumber = 0;
  m_HitNumberBKLM = 0;
  m_HitNumberEKLM = 0;
  for (int i = 0; i < n; ++i) {
    calibrationData->GetEntry(i);
    m_HitMapChannel.setChannelData(
      channel, m_HitMapChannel.getChannelData(channel) + hits);
    m_TotalHitNumber += hits;
    if (m_ElementNumbers->isEKLMChannel(channel))
      m_HitNumberEKLM += hits;
    else
      m_HitNumberBKLM += hits;
  }
  m_ChannelStatus = new KLMChannelStatus();
  m_ChannelStatus->setStatusAllChannels(KLMChannelStatus::c_Undetermined);
  /* If there are no hits, then mark all channels as dead. */
  BKLMChannelIndex bklmChannels;
  EKLMChannelIndex eklmChannels;
  if (m_TotalHitNumber == 0) {
    for (BKLMChannelIndex& bklmChannel : bklmChannels) {
      channel = bklmChannel.getKLMChannelNumber();
      m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
    }
    for (EKLMChannelIndex& eklmChannel : eklmChannels) {
      channel = eklmChannel.getKLMChannelNumber();
      m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
    }
    saveCalibration(m_ChannelStatus, "KLMChannelStatus");
    return CalibrationAlgorithm::c_OK;
  }
  /* Fill module and sector hit maps. */
  BKLMChannelIndex bklmModules(BKLMChannelIndex::c_IndexLevelLayer);
  EKLMChannelIndex eklmModules(EKLMChannelIndex::c_IndexLevelLayer);
  for (BKLMChannelIndex& bklmModule : bklmModules)
    m_HitMapModule.setChannelData(bklmModule.getKLMModuleNumber(), 0);
  for (EKLMChannelIndex& eklmModule : eklmModules)
    m_HitMapModule.setChannelData(eklmModule.getKLMModuleNumber(), 0);
  BKLMChannelIndex bklmSectors(BKLMChannelIndex::c_IndexLevelSector);
  EKLMChannelIndex eklmSectors(EKLMChannelIndex::c_IndexLevelSector);
  for (BKLMChannelIndex& bklmSector : bklmSectors)
    m_HitMapSector.setChannelData(bklmSector.getKLMSectorNumber(), 0);
  for (EKLMChannelIndex& eklmSector : eklmSectors)
    m_HitMapSector.setChannelData(eklmSector.getKLMSectorNumber(), 0);
  for (BKLMChannelIndex& bklmChannel : bklmChannels) {
    channel = bklmChannel.getKLMChannelNumber();
    module = bklmChannel.getKLMModuleNumber();
    sector = bklmChannel.getKLMSectorNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModule.setChannelData(
      module, m_HitMapModule.getChannelData(module) + hits);
    m_HitMapSector.setChannelData(
      sector, m_HitMapSector.getChannelData(sector) + hits);
  }
  for (EKLMChannelIndex& eklmChannel : eklmChannels) {
    channel = eklmChannel.getKLMChannelNumber();
    module = eklmChannel.getKLMModuleNumber();
    sector = eklmChannel.getKLMSectorNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModule.setChannelData(
      module, m_HitMapModule.getChannelData(module) + hits);
    m_HitMapSector.setChannelData(
      sector, m_HitMapSector.getChannelData(sector) + hits);
  }
  /*
   * Mark all channels in modules without hits as dead.
   * Search for hot channels.
   */
  for (BKLMChannelIndex& bklmModule : bklmModules) {
    module = bklmModule.getKLMModuleNumber();
    moduleHits = m_HitMapModule.getChannelData(module);
    BKLMChannelIndex bklmNextModule(bklmModule);
    ++bklmNextModule;
    BKLMChannelIndex bklmChannel(bklmModule);
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; bklmChannel != bklmNextModule; ++bklmChannel) {
        channel = bklmChannel.getKLMChannelNumber();
        m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      m_ModuleActiveChannelMap.setChannelData(module, 0);
      continue;
    }
    unsigned int activeChannels = 0;
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      channel = bklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    m_ModuleActiveChannelMap.setChannelData(module, activeChannels);
    bklmChannel = bklmModule;
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      markHotChannels(bklmChannel.getKLMChannelNumber(), moduleHits,
                      activeChannels);
    }
  }
  for (EKLMChannelIndex& eklmModule : eklmModules) {
    module = eklmModule.getKLMModuleNumber();
    moduleHits = m_HitMapModule.getChannelData(module);
    EKLMChannelIndex eklmNextModule(eklmModule);
    ++eklmNextModule;
    EKLMChannelIndex eklmChannel(eklmModule);
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; eklmChannel != eklmNextModule; ++eklmChannel) {
        channel = eklmChannel.getKLMChannelNumber();
        m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      m_ModuleActiveChannelMap.setChannelData(module, 0);
      continue;
    }
    unsigned int activeChannels = 0;
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      channel = eklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    m_ModuleActiveChannelMap.setChannelData(module, activeChannels);
    eklmChannel = eklmModule;
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      markHotChannels(eklmChannel.getKLMChannelNumber(), moduleHits,
                      activeChannels);
    }
  }
  /* Fill module and sector hit maps with hot channels subtracted. */
  for (BKLMChannelIndex& bklmModule : bklmModules)
    m_HitMapModuleNoHot.setChannelData(bklmModule.getKLMModuleNumber(), 0);
  for (EKLMChannelIndex& eklmModule : eklmModules)
    m_HitMapModuleNoHot.setChannelData(eklmModule.getKLMModuleNumber(), 0);
  for (BKLMChannelIndex& bklmSector : bklmSectors)
    m_HitMapSectorNoHot.setChannelData(bklmSector.getKLMSectorNumber(), 0);
  for (EKLMChannelIndex& eklmSector : eklmSectors)
    m_HitMapSectorNoHot.setChannelData(eklmSector.getKLMSectorNumber(), 0);
  m_HitNumberBKLMNoHot = 0;
  m_HitNumberEKLMNoHot = 0;
  for (BKLMChannelIndex& bklmChannel : bklmChannels) {
    channel = bklmChannel.getKLMChannelNumber();
    if (m_ChannelStatus->getChannelStatus(channel) == KLMChannelStatus::c_Hot)
      continue;
    module = bklmChannel.getKLMModuleNumber();
    sector = bklmChannel.getKLMSectorNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModuleNoHot.setChannelData(
      module, m_HitMapModuleNoHot.getChannelData(module) + hits);
    m_HitMapSectorNoHot.setChannelData(
      sector, m_HitMapSectorNoHot.getChannelData(sector) + hits);
    m_HitNumberBKLMNoHot += hits;
  }
  for (EKLMChannelIndex& eklmChannel : eklmChannels) {
    channel = eklmChannel.getKLMChannelNumber();
    if (m_ChannelStatus->getChannelStatus(channel) == KLMChannelStatus::c_Hot)
      continue;
    module = eklmChannel.getKLMModuleNumber();
    sector = eklmChannel.getKLMSectorNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModuleNoHot.setChannelData(
      module, m_HitMapModuleNoHot.getChannelData(module) + hits);
    m_HitMapSectorNoHot.setChannelData(
      sector, m_HitMapSectorNoHot.getChannelData(sector) + hits);
    m_HitNumberEKLMNoHot += hits;
  }
  /* Sector status. */
  int activeSectorsBKLM = 0;
  for (BKLMChannelIndex& bklmSector : bklmSectors) {
    sector = bklmSector.getKLMSectorNumber();
    hits = m_HitMapSectorNoHot.getChannelData(sector);
    if (hits > 0)
      activeSectorsBKLM++;
  }
  int activeSectorsEKLM = 0;
  for (EKLMChannelIndex& eklmSector : eklmSectors) {
    sector = eklmSector.getKLMSectorNumber();
    hits = m_HitMapSectorNoHot.getChannelData(sector);
    if (hits > 0)
      activeSectorsEKLM++;
  }
  double averageHitsActiveSector = 0;
  if (activeSectorsBKLM > 0)
    averageHitsActiveSector = double(m_HitNumberBKLMNoHot) / activeSectorsBKLM;
  for (BKLMChannelIndex& bklmSector : bklmSectors) {
    sector = bklmSector.getKLMSectorNumber();
    calibrateSector(sector, averageHitsActiveSector);
  }
  if (activeSectorsEKLM > 0)
    averageHitsActiveSector = double(m_HitNumberEKLMNoHot) / activeSectorsBKLM;
  for (EKLMChannelIndex& eklmSector : eklmSectors) {
    sector = eklmSector.getKLMSectorNumber();
    calibrateSector(sector, averageHitsActiveSector);
  }
  /* Module status. */
  if (m_ModuleStatus == nullptr)
    m_ModuleStatus = new KLMChannelStatus();
  for (BKLMChannelIndex& bklmModule : bklmModules)
    calibrateModule(bklmModule.getKLMModuleNumber());
  for (EKLMChannelIndex& eklmModule : eklmModules)
    calibrateModule(eklmModule.getKLMModuleNumber());
  /* Channel-based calibration. */
  bool notEnoughData = false;
  for (BKLMChannelIndex& bklmModule : bklmModules) {
    module = bklmModule.getKLMModuleNumber();
    moduleHits = m_HitMapModule.getChannelData(module);
    BKLMChannelIndex bklmNextModule(bklmModule);
    ++bklmNextModule;
    BKLMChannelIndex bklmChannel(bklmModule);
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0)
      continue;
    unsigned int activeChannels = 0;
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      channel = bklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    m_ModuleActiveChannelMap.setChannelData(module, activeChannels);
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber && !m_ForcedCalibration) {
      if (!notEnoughData) {
        B2INFO("BKLM module " << bklmModule.getKLMModuleNumber() << " (forward "
               << bklmModule.getForward() << ", sector " <<
               bklmModule.getSector() << ", layer " << bklmModule.getLayer() <<
               "): " << moduleHits << " hits, average: " << averageHits << ".");
      }
      notEnoughData = true;
      continue;
    }
    bklmChannel = bklmModule;
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    for (; bklmChannel != bklmNextModule; ++bklmChannel)
      calibrateChannel(bklmChannel.getKLMChannelNumber());
  }
  for (EKLMChannelIndex& eklmModule : eklmModules) {
    module = eklmModule.getKLMModuleNumber();
    moduleHits = m_HitMapModule.getChannelData(module);
    EKLMChannelIndex eklmNextModule(eklmModule);
    ++eklmNextModule;
    EKLMChannelIndex eklmChannel(eklmModule);
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0)
      continue;
    unsigned int activeChannels = 0;
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      channel = eklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    m_ModuleActiveChannelMap.setChannelData(module, activeChannels);
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber && !m_ForcedCalibration) {
      if (!notEnoughData) {
        B2INFO("EKLM module " << eklmModule.getKLMModuleNumber() << " (endcap "
               << eklmModule.getEndcap() << ", sector " <<
               eklmModule.getSector() << ", layer " << eklmModule.getLayer() <<
               "): " << moduleHits << " hits, average: " << averageHits << ".");
      }
      notEnoughData = true;
      continue;
    }
    eklmChannel = eklmModule;
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    for (; eklmChannel != eklmNextModule; ++eklmChannel)
      calibrateChannel(eklmChannel.getKLMChannelNumber());
  }
  if (notEnoughData)
    return CalibrationAlgorithm::c_NotEnoughData;
  saveCalibration(m_ChannelStatus, "KLMChannelStatus");
  return CalibrationAlgorithm::c_OK;
}

void KLMChannelStatusCalibrationAlgorithm::calibrateSector(
  uint16_t sector, double averageHitsActiveSector)
{
  unsigned int hits = m_HitMapSectorNoHot.getChannelData(sector);
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

void KLMChannelStatusCalibrationAlgorithm::calibrateModule(uint16_t module)
{
  unsigned int hits = m_HitMapModule.getChannelData(module);
  if (hits >= m_MinimalModuleHitNumber)
    m_ModuleStatus->setChannelStatus(module, KLMChannelStatus::c_Normal);
  else
    m_ModuleStatus->setChannelStatus(module, KLMChannelStatus::c_Dead);
}

void KLMChannelStatusCalibrationAlgorithm::markHotChannels(
  uint16_t channel, unsigned int moduleHits, int activeChannels)
{
  unsigned int hits = m_HitMapChannel.getChannelData(channel);
  if (hits == 0)
    return;
  if (activeChannels == 1) {
    if (hits >= m_MinimalHitNumberSingleHotChannel)
      m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Hot);
  } else {
    double r = hits / (double(moduleHits - hits) / (activeChannels - 1));
    if (r > m_MinimalHitNumberRatioHotChannel)
      m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Hot);
  }
}

void KLMChannelStatusCalibrationAlgorithm::calibrateChannel(uint16_t channel)
{
  unsigned int hits = m_HitMapChannel.getChannelData(channel);
  if (hits > 0)
    m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
  else
    m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
}
