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
  uint16_t channel, module;
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
  for (int i = 0; i < n; ++i) {
    calibrationData->GetEntry(i);
    m_HitMapChannel.setChannelData(
      channel, m_HitMapChannel.getChannelData(channel) + hits);
    m_TotalHitNumber += hits;
  }
  m_ChannelStatus = new KLMChannelStatus();
  /* Fill module hit map. */
  BKLMChannelIndex bklmModules(BKLMChannelIndex::c_IndexLevelLayer);
  for (BKLMChannelIndex& bklmModule : bklmModules)
    m_HitMapModule.setChannelData(bklmModule.getKLMModuleNumber(), 0);
  EKLMChannelIndex eklmModules(EKLMChannelIndex::c_IndexLevelLayer);
  for (EKLMChannelIndex& eklmModule : eklmModules)
    m_HitMapModule.setChannelData(eklmModule.getKLMModuleNumber(), 0);
  BKLMChannelIndex bklmChannels;
  for (BKLMChannelIndex& bklmChannel : bklmChannels) {
    channel = bklmChannel.getKLMChannelNumber();
    module = bklmChannel.getKLMModuleNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModule.setChannelData(
      module, m_HitMapModule.getChannelData(module) + hits);
  }
  EKLMChannelIndex eklmChannels;
  for (EKLMChannelIndex& eklmChannel : eklmChannels) {
    channel = eklmChannel.getKLMChannelNumber();
    module = eklmChannel.getKLMModuleNumber();
    hits = m_HitMapChannel.getChannelData(channel);
    m_HitMapModule.setChannelData(
      module, m_HitMapModule.getChannelData(module) + hits);
  }
  /* Module status. */
  if (m_ModuleStatus == nullptr)
    m_ModuleStatus = new KLMChannelStatus();
  for (BKLMChannelIndex& bklmModule : bklmModules)
    calibrateModule(bklmModule.getKLMModuleNumber());
  for (EKLMChannelIndex& eklmModule : eklmModules)
    calibrateModule(eklmModule.getKLMModuleNumber());
  /* Channel-based calibration. */
  for (BKLMChannelIndex& bklmModule : bklmModules) {
    moduleHits = m_HitMapModule.getChannelData(bklmModule.getKLMModuleNumber());
    BKLMChannelIndex bklmNextModule(bklmModule);
    ++bklmNextModule;
    BKLMChannelIndex bklmChannel(bklmModule);
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; bklmChannel != bklmNextModule; ++bklmChannel) {
        channel = bklmChannel.getKLMChannelNumber();
        m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      continue;
    }
    unsigned int activeChannels = 0;
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      channel = bklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber && !m_ForcedCalibration) {
      B2INFO("BKLM module " << bklmModule.getKLMModuleNumber() << " (forward "
             << bklmModule.getForward() << ", sector " <<
             bklmModule.getSector() << ", layer " << bklmModule.getLayer() <<
             "): " << moduleHits << " hits, average: " << averageHits << ".");
      return CalibrationAlgorithm::c_NotEnoughData;
    }
    bklmChannel = bklmModule;
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    for (; bklmChannel != bklmNextModule; ++bklmChannel)
      calibrateChannel(bklmChannel.getKLMChannelNumber());
  }
  for (EKLMChannelIndex& eklmModule : eklmModules) {
    moduleHits = m_HitMapModule.getChannelData(eklmModule.getKLMModuleNumber());
    EKLMChannelIndex eklmNextModule(eklmModule);
    ++eklmNextModule;
    EKLMChannelIndex eklmChannel(eklmModule);
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; eklmChannel != eklmNextModule; ++eklmChannel) {
        channel = eklmChannel.getKLMChannelNumber();
        m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      continue;
    }
    unsigned int activeChannels = 0;
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      channel = eklmChannel.getKLMChannelNumber();
      hits = m_HitMapChannel.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber && !m_ForcedCalibration) {
      B2INFO("EKLM module " << eklmModule.getKLMModuleNumber() << " (endcap "
             << eklmModule.getEndcap() << ", sector " <<
             eklmModule.getSector() << ", layer " << eklmModule.getLayer() <<
             "): " << moduleHits << " hits, average: " << averageHits << ".");
      return CalibrationAlgorithm::c_NotEnoughData;
    }
    eklmChannel = eklmModule;
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    for (; eklmChannel != eklmNextModule; ++eklmChannel)
      calibrateChannel(eklmChannel.getKLMChannelNumber());
  }
  saveCalibration(m_ChannelStatus, "KLMChannelStatus");
  return CalibrationAlgorithm::c_OK;
}

void KLMChannelStatusCalibrationAlgorithm::calibrateModule(uint16_t module)
{
  unsigned int hits = m_HitMapModule.getChannelData(module);
  if (hits >= m_MinimalModuleHitNumber)
    m_ModuleStatus->setChannelStatus(module, KLMChannelStatus::c_Normal);
  else
    m_ModuleStatus->setChannelStatus(module, KLMChannelStatus::c_Dead);
}

void KLMChannelStatusCalibrationAlgorithm::calibrateChannel(uint16_t channel)
{
  unsigned int hits = m_HitMapChannel.getChannelData(channel);
  if (hits > 0)
    m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
  else
    m_ChannelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
}
