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
#include <klm/bklm/dbobjects/BKLMGeometryPar.h>
#include <klm/bklm/dbobjects/BKLMSimulationPar.h>
#include <klm/bklm/dbobjects/BKLMMisAlignment.h>
#include <klm/bklm/dbobjects/BKLMDisplacement.h>
#include <klm/bklm/dbobjects/BKLMTimeWindow.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <klm/bklm/dataobjects/BKLMElementID.h>
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

void BKLMDatabaseImporter::exportBklmGeometryPar()
{
  DBObjPtr<BKLMGeometryPar> element("BKLMGeometryPar");

  B2INFO("BKLMGeometryPar version: " << element->getVersion() <<
         ", global rotation angle " << element->getRotation() <<
         ", module frame width: " << element->getModuleFrameWidth() <<
         ", module frame thickness: " << element->getModuleFrameThickness() <<
         ", local reconstruction shift (x,y,z) of forward sector 1 layer 1: (" <<
         element->getLocalReconstructionShiftX(1, 1, 1) << ", " <<
         element->getLocalReconstructionShiftY(1, 1, 1) << ", " <<
         element->getLocalReconstructionShiftZ(1, 1, 1) << ")");
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

void BKLMDatabaseImporter::exportBklmSimulationPar()
{

  DBObjPtr<BKLMSimulationPar> element("BKLMSimulationPar");

  B2INFO("HitTimeMax: " << element->getHitTimeMax());
  B2INFO("weight table: ");
  for (int ii = 0; ii < element->getNPhiDivision(); ii++) {
    for (int jj = 1; jj <= element->getNPhiMultiplicity(ii); jj++) {
      B2INFO(ii << ", " << jj << ", :" << element->getPhiWeight(ii, jj) << endl);
    }
  }

}

void BKLMDatabaseImporter::importBklmMisAlignment()
{

  DBImportObjPtr<BKLMMisAlignment> mal;
  mal.construct();
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 15; k++) {
        BKLMElementID bklmid(i, j, k);
        mal->set(bklmid, 1, 0.);
        mal->set(bklmid, 2, 0.);
        mal->set(bklmid, 3, 0.);
        mal->set(bklmid, 4, 0.);
        mal->set(bklmid, 5, 0.);
        mal->set(bklmid, 6, 0.);
      }
    }
  }

  IntervalOfValidity Iov(0, 0, -1, -1);
  mal.import(Iov);
}

void BKLMDatabaseImporter::exportBklmMisAlignment()
{

  DBObjPtr<BKLMMisAlignment> element("BKLMMisAlignment");

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 15; k++) {
        B2INFO("bklm misalignment parameter of section " << i << ", sector " << j + 1 << ", layer " << k + 1);
        for (int p = 1; p < 7; p++) { //six parameter
          BKLMElementID bklmid(i, j, k);
          double par = element->get(bklmid, p);
          B2INFO(" p [" << p << "] : " << par);
        }
        //B2INFO(" " << endl);
      }//end loop layer
    }//end loop sector
  }
}

void BKLMDatabaseImporter::importBklmAlignment()
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

  IntervalOfValidity Iov(0, 0, -1, -1);
  al.import(Iov);
}

void BKLMDatabaseImporter::exportBklmAlignment()
{

  DBObjPtr<BKLMAlignment> element("BKLMAlignment");

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 15; k++) {
        B2INFO("bklm alignment parameter of section " << i << ", sector " << j + 1 << ", layer " << k + 1);
        for (int p = 1; p < 7; p++) { //six parameter
          BKLMElementID bklmid(i, j, k);
          double par = element->get(bklmid, p);
          B2INFO(" p [" << p << "] : " << par);
        }
        //B2INFO(" " << endl);
      }//end loop layer
    }//end loop sector
  }
}


void BKLMDatabaseImporter::importBklmDisplacement()
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

  IntervalOfValidity Iov(0, 0, -1, -1);
  m_displacement.import(Iov);
}

void BKLMDatabaseImporter::exportBklmDisplacement()
{
  DBArray<BKLMDisplacement> displacements;
  for (const auto& disp : displacements) {
    unsigned short bklmElementID = disp.getElementID();
    BKLMElementID bklmid(bklmElementID);
    unsigned short section = bklmid.getSection();
    unsigned short sector = bklmid.getSectorNumber();
    unsigned short layer = bklmid.getLayerNumber();
    B2INFO("displacement of " << section << ", " << sector << ", " << layer << ": " << disp.getUShift() << ", " << disp.getVShift() <<
           ", " <<
           disp.getWShift() << ", " << disp.getAlphaRotation() << ", " << disp.getBetaRotation() << ", " << disp.getGammaRotation());
  }//end loop layer
}

void BKLMDatabaseImporter::importBklmADCThreshold(BKLMADCThreshold* threshold)
{
  DBImportObjPtr<BKLMADCThreshold> adcParam;
  adcParam.construct(*threshold);
  IntervalOfValidity iov(0, 0, -1, -1);
  adcParam.import(iov);
}

void BKLMDatabaseImporter::exportBklmADCThreshold()
{

  DBObjPtr<BKLMADCThreshold> element("BKLMADCThreshold");
  B2INFO("MPPC gain " << element->getMPPCGain());
  B2INFO("ADC offset " << element->getADCOffset());
  B2INFO("ADC threshold " << element->getADCThreshold());
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

void BKLMDatabaseImporter::exportBklmTimeWindow()
{

  DBObjPtr<BKLMTimeWindow> m_timing("BKLMTimeWindow");
  B2INFO("z/phi coincidence window " << m_timing->getCoincidenceWindow());
  B2INFO(" timing cut reference " << m_timing->getPromptTime());
  B2INFO(" timing window " << m_timing->getPromptWindow());
}
