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
#include <klm/dbobjects/KLMChannelStatus.h>

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
  uint16_t channel;
  unsigned int hits;
  KLMChannelMapValue<unsigned int> hitMap;
  std::shared_ptr<TTree> calibrationData;
  calibrationData = getObjectPtr<TTree>("calibration_data");
  calibrationData->SetBranchAddress("channel", &channel);
  calibrationData->SetBranchAddress("hits", &hits);
  int n = calibrationData->GetEntries();
  for (int i = 0; i < n; ++i) {
    calibrationData->GetEntry(i);
    hitMap.setChannelData(channel, hits);
  }
  KLMChannelStatus* channelStatus = new KLMChannelStatus();
  BKLMChannelIndex bklmChannels;
  for (BKLMChannelIndex& bklmChannel : bklmChannels) {
    channel = bklmChannel.getKLMChannelNumber();
    /* One hit from the previous run may be recorded for RPCs. */
    if (hitMap.getChannelData(channel) > 1)
      channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
    else
      channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
  }
  EKLMChannelIndex eklmChannels;
  for (EKLMChannelIndex& eklmChannel : eklmChannels) {
    channel = eklmChannel.getKLMChannelNumber();
    if (hitMap.getChannelData(channel) > 0)
      channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Normal);
    else
      channelStatus->setChannelStatus(channel, KLMChannelStatus::c_Dead);
  }
  saveCalibration(channelStatus, "KLMChannelStatus");
  return CalibrationAlgorithm::c_OK;
}
