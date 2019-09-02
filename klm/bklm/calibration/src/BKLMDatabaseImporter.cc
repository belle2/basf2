/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Vipin Gaur,                                *
 *               Zachary S. Stottler, Giacomo De Pietro                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <klm/bklm/calibration/BKLMDatabaseImporter.h>
#include <klm/bklm/dbobjects/BKLMAlignment.h>
#include <klm/bklm/dbobjects/BKLMGeometryPar.h>
#include <klm/bklm/dbobjects/BKLMSimulationPar.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;

BKLMDatabaseImporter::BKLMDatabaseImporter()
{}

void BKLMDatabaseImporter::loadDefaultBklmElectronicMapping()
{
  int copperId = 0;
  int slotId = 0;
  int laneId;
  int axisId = 0;
  KLMChannelIndex bklmPlanes(KLMChannelIndex::c_IndexLevelPlane);
  for (KLMChannelIndex bklmPlane = bklmPlanes.beginBKLM();
       bklmPlane != bklmPlanes.endBKLM(); ++bklmPlane) {
    int section = bklmPlane.getSection();
    int sector = bklmPlane.getSector();
    int layer = bklmPlane.getLayer();
    int plane = bklmPlane.getPlane();
    if (section == BKLMElementNumbers::c_ForwardSection) {
      if (sector == 3 || sector == 4 || sector == 5 || sector == 6)
        copperId = 1 + BKLM_ID;
      if (sector == 1 || sector == 2 || sector == 7 || sector == 8)
        copperId = 2 + BKLM_ID;
    }
    if (section == BKLMElementNumbers::c_BackwardSection) {
      if (sector == 3 || sector == 4 || sector == 5 || sector == 6)
        copperId = 3 + BKLM_ID;
      if (sector == 1 || sector == 2 || sector == 7 || sector == 8)
        copperId = 4 + BKLM_ID;
    }
    if (sector == 3 || sector == 4 || sector == 5 || sector == 6) slotId = sector - 2;
    if (sector == 1 || sector == 2) slotId = sector + 2;
    if (sector == 7 || sector == 8) slotId = sector - 6;

    if (layer > 2)  laneId = layer + 5;
    else laneId = layer;

    if (layer < 3) {
      if (plane == 0) axisId = 1;
      else if (plane == 1) axisId = 0;
    } else axisId = plane;

    int MaxiChannel = BKLMElementNumbers::getNStrips(
                        section, sector, layer, plane);

    bool dontFlip = false;
    if (section == BKLMElementNumbers::c_ForwardSection &&
        (sector == 7 ||  sector == 8 || sector == 1 || sector == 2))
      dontFlip = true;
    if (section == BKLMElementNumbers::c_BackwardSection &&
        (sector == 4 ||  sector == 5 || sector == 6 || sector == 7))
      dontFlip = true;

    for (int iStrip = 1; iStrip <= MaxiChannel; iStrip++) {
      int channelId = iStrip;
      if (!(dontFlip && layer > 2 && plane == 1)) channelId = MaxiChannel - iStrip + 1;

      if (plane == 1) { //phi strips
        if (layer == 1)  channelId = channelId + 4;
        if (layer == 2)  channelId = channelId + 2;
      } else if (plane == 0) { //z strips
        if (layer < 3) { //scintillator
          if (section == BKLMElementNumbers::c_BackwardSection
              && sector == 3) { //sector #3 is the top sector, backward sector#3 is the chimney sector.
            if (layer == 1) {
              if (channelId > 0 && channelId < 9) channelId = 9 - channelId;
              else if (channelId > 8 && channelId < 24) channelId = 54 - channelId;
              else if (channelId > 23 && channelId < 39) channelId = 54 - channelId;
            } else {
              if (channelId > 0 && channelId < 10) channelId = 10 - channelId;
              else if (channelId > 9 && channelId < 24) channelId = 40 - channelId;
              else if (channelId > 23 && channelId < 39) channelId = 69 - channelId;
            }
          } else { //all sectors except backward sector #3
            if (channelId > 0 && channelId < 10) channelId = 10 - channelId;
            else if (channelId > 9 && channelId < 25) channelId = 40 - channelId;
            else if (channelId > 24 && channelId < 40) channelId = 70 - channelId;
            else if (channelId > 39 && channelId < 55) channelId = 100 - channelId;
          }
        }
      }

      uint16_t detectorChannel = BKLMElementNumbers::channelNumber(
                                   section, sector, layer, plane, iStrip);
      m_ElectronicsChannels.push_back(
        std::pair<uint16_t, BKLMElectronicsChannel>(
          detectorChannel,
          BKLMElectronicsChannel(copperId, slotId, laneId, axisId, channelId)));
    }
  }
}

void BKLMDatabaseImporter::setElectronicMappingLane(
  int section, int sector, int layer, int lane)
{
  int channelSection, channelSector, channelLayer, plane, strip;
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    uint16_t channel = m_ElectronicsChannels[i].first;
    BKLMElementNumbers::channelNumberToElementNumbers(
      channel, &channelSection, &channelSector, &channelLayer, &plane, &strip);
    if ((channelSection == section) &&
        (channelSector == sector) &&
        (channelLayer == layer))
      m_ElectronicsChannels[i].second.setLane(lane);
  }
}

void BKLMDatabaseImporter::importBklmElectronicMapping()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  DBImportObjPtr<BKLMElectronicsMap> m_ElectronicsMap;
  m_ElectronicsMap.construct();
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    m_ElectronicsMap->addChannel(
      m_ElectronicsChannels[i].first,
      m_ElectronicsChannels[i].second.getCopper(),
      m_ElectronicsChannels[i].second.getSlot(),
      m_ElectronicsChannels[i].second.getLane(),
      m_ElectronicsChannels[i].second.getAxis(),
      m_ElectronicsChannels[i].second.getChannel());
  }
  m_ElectronicsMap.import(iov);
  return;
}

void BKLMDatabaseImporter::importBklmGeometryPar()
{
  GearDir content("/Detector/DetectorComponent[@name=\"BKLM\"]/Content");

  // define the data
  BKLMGeometryPar bklmGeometryPar;

  // Get Gearbox parameters for BKLM
  bklmGeometryPar.setVersion(0);
  bklmGeometryPar.read(content);

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("BKLMGeometryPar", &bklmGeometryPar, iov);

}

void BKLMDatabaseImporter::importBklmSimulationPar(int expStart, int runStart, int expStop, int runStop)
{
  BKLMSimulationPar bklmSimulationPar;
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"), "BKLM/SimulationParameters");

  // Get Gearbox simulation parameters for BKLM
  bklmSimulationPar.read(content);

  // Define the IOV and store data to the DB
  IntervalOfValidity iov(expStart, runStart, expStop, runStop);
  Database::Instance().storeData("BKLMSimulationPar", &bklmSimulationPar, iov);
}

void BKLMDatabaseImporter::importBklmDisplacement(const char* payloadName)
{
  DBImportObjPtr<BKLMAlignment> bklmDisplacement(payloadName);
  bklmDisplacement.construct();
  KLMAlignmentData alignmentData(0, 0, 0, 0, 0, 0);
  KLMChannelIndex bklmModules(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex bklmModule = bklmModules.beginBKLM();
       bklmModule != bklmModules.endBKLM(); ++bklmModule) {
    uint16_t module = bklmModule.getKLMModuleNumber();
    bklmDisplacement->setModuleAlignment(module, &alignmentData);
  }
  IntervalOfValidity iov(0, 0, -1, -1);
  bklmDisplacement.import(iov);
}

void BKLMDatabaseImporter::importBklmADCThreshold(BKLMADCThreshold* threshold)
{
  DBImportObjPtr<BKLMADCThreshold> adcParam;
  adcParam.construct(*threshold);
  IntervalOfValidity iov(0, 0, -1, -1);
  adcParam.import(iov);
}

void BKLMDatabaseImporter::importBklmTimeWindow()
{

  DBImportObjPtr<BKLMTimeWindow> m_timing;
  m_timing.construct();
  m_timing->setCoincidenceWindow(50);
  m_timing->setPromptTime(0);
  m_timing->setPromptWindow(2000);

  IntervalOfValidity iov(0, 0, -1, -1);
  m_timing.import(iov);
}
