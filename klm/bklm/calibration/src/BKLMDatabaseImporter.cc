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

/* Own header. */
#include <klm/bklm/calibration/BKLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMElementID.h>
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/bklm/dbobjects/BKLMDisplacement.h>
#include <klm/bklm/dbobjects/BKLMElectronicsMap.h>
#include <klm/bklm/dbobjects/BKLMGeometryPar.h>
#include <klm/bklm/dbobjects/BKLMSimulationPar.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <klm/dataobjects/KLMChannelIndex.h>

/* Belle 2 headers. */
#include <alignment/dbobjects/BKLMAlignment.h>
#include <framework/database/Database.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/GearDir.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

using namespace std;
using namespace Belle2;

BKLMDatabaseImporter::BKLMDatabaseImporter() :
  m_ExperimentLow(0),
  m_RunLow(0),
  m_ExperimentHigh(-1),
  m_RunHigh(-1)
{}

void BKLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                  int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void BKLMDatabaseImporter::loadDefaultElectronicMapping()
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

void BKLMDatabaseImporter::importElectronicMapping()
{
  DBImportObjPtr<BKLMElectronicsMap> electronicsMap;
  electronicsMap.construct();
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    electronicsMap->addChannel(
      m_ElectronicsChannels[i].first,
      m_ElectronicsChannels[i].second.getCopper(),
      m_ElectronicsChannels[i].second.getSlot(),
      m_ElectronicsChannels[i].second.getLane(),
      m_ElectronicsChannels[i].second.getAxis(),
      m_ElectronicsChannels[i].second.getChannel());
  }
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  electronicsMap.import(iov);
}

void BKLMDatabaseImporter::importGeometryPar()
{
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"));
  BKLMGeometryPar bklmGeometryPar(content);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  Database::Instance().storeData("BKLMGeometryPar", &bklmGeometryPar, iov);

}

void BKLMDatabaseImporter::importSimulationPar()
{
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"), "BKLM/SimulationParameters");
  BKLMSimulationPar bklmSimulationPar(content);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  Database::Instance().storeData("BKLMSimulationPar", &bklmSimulationPar, iov);
}

void BKLMDatabaseImporter::importAlignment()
{
  DBImportObjPtr<BKLMAlignment> al;
  al.construct();
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 15; k++) {
        BKLMElementID bklmid(i, j, k);
        al->set(bklmid, 1, 0.);
        al->set(bklmid, 2, 0.);
        al->set(bklmid, 3, 0.);
        al->set(bklmid, 4, 0.);
        al->set(bklmid, 5, 0.);
        al->set(bklmid, 6, 0.);
      }
    }
  }
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  al.import(iov);
}

void BKLMDatabaseImporter::importDisplacement()
{
  DBImportArray<BKLMDisplacement> m_displacement;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 15; k++) {
        BKLMElementID bklmid(i, j, k);
        m_displacement.appendNew(bklmid, 0, 0, 0, 0, 0, 0);
      }
    }
  }
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_displacement.import(iov);
}

void BKLMDatabaseImporter::importADCThreshold(BKLMADCThreshold* inputThreshold)
{
  DBImportObjPtr<BKLMADCThreshold> adcThreshold;
  adcThreshold.construct(*inputThreshold);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  adcThreshold.import(iov);
}

void BKLMDatabaseImporter::importTimeWindow(BKLMTimeWindow* inputWindow)
{
  DBImportObjPtr<BKLMTimeWindow> timeWindow;
  timeWindow.construct(*inputWindow);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeWindow.import(iov);
}
