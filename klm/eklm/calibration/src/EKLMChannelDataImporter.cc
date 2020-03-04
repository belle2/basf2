/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/calibration/EKLMChannelDataImporter.h>

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/dbobjects/EKLMChannels.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

using namespace Belle2;

EKLMChannelDataImporter::EKLMChannelDataImporter()
{
  m_ExperimentLow = 0;
  m_RunLow = 0;
  m_ExperimentHigh = -1;
  m_RunHigh = -1;
}

EKLMChannelDataImporter::~EKLMChannelDataImporter()
{
}

void EKLMChannelDataImporter::setIOV(int experimentLow, int runLow,
                                     int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void EKLMChannelDataImporter::loadChannelData(EKLMChannelData* channelData)
{
  m_Channels.construct();
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  int iSection, iLayer, iSector, iPlane, iStrip, strip;
  for (iSection = 1; iSection <= geoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iStrip = 1; iStrip <= geoDat->getNStrips(); iStrip++) {
            strip = geoDat->stripNumber(iSection, iLayer, iSector, iPlane,
                                        iStrip);
            m_Channels->setChannelData(strip, channelData);
          }
        }
      }
    }
  }
}

void EKLMChannelDataImporter::setChannelData(
  int section, int layer, int sector, int plane, int strip,
  EKLMChannelData* channelData)
{
  int stripGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  stripGlobal = elementNumbers->stripNumber(section, layer, sector, plane,
                                            strip);
  m_Channels->setChannelData(stripGlobal, channelData);
}

void EKLMChannelDataImporter::loadActiveChannels(const char* activeChannelsData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel, active;
  /* cppcheck-suppress variableScope */
  int subdetector, section, layer, sector, plane, strip, stripGlobal;
  /* cppcheck-suppress variableScope */
  const uint16_t* detectorChannel;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* klmElementNumbers =
    &(KLMElementNumbers::Instance());
  DBObjPtr<KLMElectronicsMap> electronicsMap;
  KLMElectronicsChannel electronicsChannel;
  TFile* file;
  TTree* tree;
  file = new TFile(activeChannelsData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("active", &active);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    electronicsChannel.setCopper(copper);
    electronicsChannel.setSlot(dataConcentrator);
    electronicsChannel.setLane(lane);
    electronicsChannel.setAxis(1);
    electronicsChannel.setChannel(1);
    detectorChannel = electronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    klmElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    stripGlobal = elementNumbers->stripNumber(section, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadHighVoltage(const char* highVoltageData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  float voltage;
  /* cppcheck-suppress variableScope */
  int subdetector, section, layer, sector, plane, strip, stripGlobal;
  /* cppcheck-suppress variableScope */
  const uint16_t* detectorChannel;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* klmElementNumbers =
    &(KLMElementNumbers::Instance());
  DBObjPtr<KLMElectronicsMap> electronicsMap;
  KLMElectronicsChannel electronicsChannel;
  TFile* file;
  TTree* tree;
  file = new TFile(highVoltageData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("voltage", &voltage);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    electronicsChannel.setCopper(copper);
    electronicsChannel.setSlot(dataConcentrator);
    electronicsChannel.setLane(lane);
    electronicsChannel.setAxis(1);
    electronicsChannel.setChannel(1);
    detectorChannel = electronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    klmElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    stripGlobal = elementNumbers->stripNumber(section, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setVoltage(voltage);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadLookbackWindow(const char* lookbackWindowData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  int lookbackTime, lookbackWindowWidth;
  /* cppcheck-suppress variableScope */
  int subdetector, section, layer, sector, plane, strip, stripGlobal;
  /* cppcheck-suppress variableScope */
  const uint16_t* detectorChannel;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* klmElementNumbers =
    &(KLMElementNumbers::Instance());
  DBObjPtr<KLMElectronicsMap> electronicsMap;
  KLMElectronicsChannel electronicsChannel;
  TFile* file;
  TTree* tree;
  file = new TFile(lookbackWindowData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("lookback_time", &lookbackTime);
  tree->SetBranchAddress("lookback_window_width", &lookbackWindowWidth);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    electronicsChannel.setCopper(copper);
    electronicsChannel.setSlot(dataConcentrator);
    electronicsChannel.setLane(lane);
    electronicsChannel.setAxis(1);
    electronicsChannel.setChannel(1);
    detectorChannel = electronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    klmElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    stripGlobal = elementNumbers->stripNumber(section, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setLookbackTime(lookbackTime);
    channelData->setLookbackWindowWidth(lookbackWindowWidth);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadThresholds(const char* thresholdsData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  int active, pedestalMin, threshold, adjustmentVoltage;
  /* cppcheck-suppress variableScope */
  int subdetector, section, layer, sector, plane, strip, stripGlobal;
  /* cppcheck-suppress variableScope */
  const uint16_t* detectorChannel;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* klmElementNumbers =
    &(KLMElementNumbers::Instance());
  DBObjPtr<KLMElectronicsMap> electronicsMap;
  KLMElectronicsChannel electronicsChannel;
  TFile* file;
  TTree* tree;
  file = new TFile(thresholdsData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("active", &active);
  tree->SetBranchAddress("pedestal_min", &pedestalMin);
  tree->SetBranchAddress("threshold", &threshold);
  tree->SetBranchAddress("adjustment_voltage", &adjustmentVoltage);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    electronicsChannel.setCopper(copper);
    electronicsChannel.setSlot(dataConcentrator);
    electronicsChannel.setLane(lane);
    electronicsChannel.setAxis(1);
    electronicsChannel.setChannel(1);
    detectorChannel = electronicsMap->getDetectorChannel(&electronicsChannel);
    if (detectorChannel == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    klmElementNumbers->channelNumberToElementNumbers(
      *detectorChannel, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    stripGlobal = elementNumbers->stripNumber(section, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setPedestal(pedestalMin);
    channelData->setThreshold(threshold);
    channelData->setAdjustmentVoltage(adjustmentVoltage);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::importChannelData()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_Channels.import(iov);
}

