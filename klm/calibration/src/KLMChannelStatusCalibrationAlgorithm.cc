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
#include <klm/calibration/KLMChannelStatusCalibrationAlgorithm.h>
#include <klm/dataobjects/KLMChannelMapValue.h>

using namespace Belle2;

KLMChannelStatusCalibrationAlgorithm::KLMChannelStatusCalibrationAlgorithm() :
  CalibrationAlgorithm("KLMChannelStatusCalibrationCollector")
{
}

KLMChannelStatusCalibrationAlgorithm::~KLMChannelStatusCalibrationAlgorithm()
{
}

CalibrationAlgorithm::EResult KLMChannelStatusCalibrationAlgorithm::calibrate()
{
  uint16_t channel, module;
  unsigned int hits, moduleHits;
  KLMChannelMapValue<unsigned int> hitMap, hitMapModule;
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
  hitMap.setDataAllChannels(0);
  for (int i = 0; i < n; ++i) {
    calibrationData->GetEntry(i);
    hitMap.setChannelData(channel, hitMap.getChannelData(channel) + hits);
    m_TotalHitNumber += hits;
  }
  KLMChannelStatus* channelStatus = new KLMChannelStatus();
  /* Fill module hit map. */
  BKLMChannelIndex bklmModules(BKLMChannelIndex::c_IndexLevelLayer);
  for (BKLMChannelIndex& bklmModule : bklmModules)
    hitMapModule.setChannelData(bklmModule.getKLMModuleNumber(), 0);
  EKLMChannelIndex eklmModules(EKLMChannelIndex::c_IndexLevelLayer);
  for (EKLMChannelIndex& eklmModule : eklmModules)
    hitMapModule.setChannelData(eklmModule.getKLMModuleNumber(), 0);
  BKLMChannelIndex bklmChannels;
  for (BKLMChannelIndex& bklmChannel : bklmChannels) {
    channel = bklmChannel.getKLMChannelNumber();
    module = bklmChannel.getKLMModuleNumber();
    hits = hitMap.getChannelData(channel);
    hitMapModule.setChannelData(
      module, hitMapModule.getChannelData(module) + hits);
  }
  EKLMChannelIndex eklmChannels;
  for (EKLMChannelIndex& eklmChannel : eklmChannels) {
    channel = eklmChannel.getKLMChannelNumber();
    module = eklmChannel.getKLMModuleNumber();
    hits = hitMap.getChannelData(channel);
    hitMapModule.setChannelData(
      module, hitMapModule.getChannelData(module) + hits);
  }
  /* Determine the channel status. */
  for (BKLMChannelIndex& bklmModule : bklmModules) {
    moduleHits = hitMapModule.getChannelData(bklmModule.getKLMModuleNumber());
    BKLMChannelIndex bklmNextModule(bklmModule);
    ++bklmNextModule;
    BKLMChannelIndex bklmChannel(bklmModule);
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; bklmChannel != bklmNextModule; ++bklmChannel) {
        channel = bklmChannel.getKLMChannelNumber();
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      continue;
    }
    unsigned int activeChannels = 0;
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      channel = bklmChannel.getKLMChannelNumber();
      hits = hitMap.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber)
      return CalibrationAlgorithm::c_NotEnoughData;
    bklmChannel = bklmModule;
    bklmChannel.setIndexLevel(BKLMChannelIndex::c_IndexLevelStrip);
    for (; bklmChannel != bklmNextModule; ++bklmChannel) {
      channel = bklmChannel.getKLMChannelNumber();
      hits = hitMap.getChannelData(channel);
      if (hitMap.getChannelData(channel) > 0)
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
      else
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
    }
  }
  for (EKLMChannelIndex& eklmModule : eklmModules) {
    moduleHits = hitMapModule.getChannelData(eklmModule.getKLMModuleNumber());
    EKLMChannelIndex eklmNextModule(eklmModule);
    ++eklmNextModule;
    EKLMChannelIndex eklmChannel(eklmModule);
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    if (moduleHits == 0) {
      for (; eklmChannel != eklmNextModule; ++eklmChannel) {
        channel = eklmChannel.getKLMChannelNumber();
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
      }
      continue;
    }
    unsigned int activeChannels = 0;
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      channel = eklmChannel.getKLMChannelNumber();
      hits = hitMap.getChannelData(channel);
      if (hits > 0)
        activeChannels++;
    }
    double averageHits = double(moduleHits) / activeChannels;
    if (averageHits < m_MinimalAverageHitNumber)
      return CalibrationAlgorithm::c_NotEnoughData;
    eklmChannel = eklmModule;
    eklmChannel.setIndexLevel(EKLMChannelIndex::c_IndexLevelStrip);
    for (; eklmChannel != eklmNextModule; ++eklmChannel) {
      channel = eklmChannel.getKLMChannelNumber();
      hits = hitMap.getChannelData(channel);
      if (hitMap.getChannelData(channel) > 0)
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
      else
        channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
    }
  }
  saveCalibration(channelStatus, "KLMChannelStatus");
  m_LastCalibrationResult = channelStatus;
  return CalibrationAlgorithm::c_OK;
}
