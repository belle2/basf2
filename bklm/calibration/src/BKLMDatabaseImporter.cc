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

#include <bklm/calibration/BKLMDatabaseImporter.h>
#include <bklm/dbobjects/BKLMGeometryPar.h>
#include <bklm/dbobjects/BKLMSimulationPar.h>
#include <bklm/dbobjects/BKLMBadChannels.h>
#include <bklm/dbobjects/BKLMMisAlignment.h>
#include <bklm/dbobjects/BKLMDisplacement.h>
#include <bklm/dbobjects/BKLMTimeWindow.h>
#include <bklm/dbobjects/BKLMStripEfficiency.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <bklm/dataobjects/BKLMElementID.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>

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
  int laneId = 0;
  int axisId = 0;
  int BKLM_ID = 117440512;
  for (int isForward = 0; isForward < 2; isForward++) {
    for (int sector = 1; sector < 9; sector++) {
      for (int layer = 1; layer < 16; layer++) {
        //plane = 0 for z; plane = 1 for phi
        for (int plane = 0; plane < 2; plane++) {

          if (isForward == 1 && (sector == 3 || sector == 4 || sector == 5 || sector == 6)) copperId = 1 + BKLM_ID;
          if (isForward == 1 && (sector == 1 || sector == 2 || sector == 7 || sector == 8)) copperId = 2 + BKLM_ID;
          if (isForward == 0 && (sector == 3 || sector == 4 || sector == 5 || sector == 6)) copperId = 3 + BKLM_ID;
          if (isForward == 0 && (sector == 1 || sector == 2 || sector == 7 || sector == 8)) copperId = 4 + BKLM_ID;
          if (sector == 3 || sector == 4 || sector == 5 || sector == 6) slotId = sector - 2;
          if (sector == 1 || sector == 2) slotId = sector + 2;
          if (sector == 7 || sector == 8) slotId = sector - 6;

          if (layer > 2)  laneId = layer + 5;
          else laneId = layer;

          if (layer < 3) {
            if (plane == 0) axisId = 1;
            else if (plane == 1) axisId = 0;
          } else axisId = plane;

          int MaxiChannel = 0;
          if (isForward == 0 && sector == 3 && plane == 0) {
            if (layer < 3) MaxiChannel = 38;
            if (layer > 2) MaxiChannel = 34;
          } else {
            if (layer == 1 && plane == 1) MaxiChannel = 37;
            if (layer == 2 && plane == 1) MaxiChannel = 42;
            if (layer > 2 && layer < 7 && plane == 1) MaxiChannel = 36;
            if (layer > 6 && plane == 1) MaxiChannel = 48;

            if (layer == 1 && plane == 0) MaxiChannel = 54;
            if (layer == 2 && plane == 0) MaxiChannel = 54;
            if (layer > 2 && plane == 0) MaxiChannel = 48;
          }

          bool dontFlip = false;
          if (isForward == 1 && (sector == 7 ||  sector == 8 ||  sector == 1 ||  sector == 2)) dontFlip = true;
          if (isForward == 0 && (sector == 4 ||  sector == 5 ||  sector == 6 ||  sector == 7)) dontFlip = true;

          for (int iStrip = 1; iStrip < (MaxiChannel + 1);  iStrip++) {
            int channelId = iStrip;
            if (!(dontFlip && layer > 2 && plane == 1)) channelId = MaxiChannel - iStrip + 1;

            if (plane == 1) { //phi strips
              if (layer == 1)  channelId = channelId + 4;
              if (layer == 2)  channelId = channelId + 2;
            } else if (plane == 0) { //z strips
              if (layer < 3 && channelId > 9) channelId = channelId + 6;
            }

            m_bklmMapping.appendNew(1, copperId, slotId, laneId, axisId, channelId, isForward, sector, layer, plane, iStrip);
          }// end of loop channels
        }//end of loop plane
      }//end of loop layers
    }//end of loop sectors
  }//end fb
}

void BKLMDatabaseImporter::setElectronicMappingLane(
  int forward, int sector, int layer, int lane)
{
  int n = m_bklmMapping.getEntries();
  for (int i = 0; i < n; i++) {
    BKLMElectronicMapping* mapping = m_bklmMapping[i];
    if ((mapping->getIsForward() == forward) &&
        (mapping->getSector() == sector) &&
        (mapping->getLayer() == layer))
      mapping->setLane(lane);
  }
}

void BKLMDatabaseImporter::importBklmElectronicMapping()
{
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  m_bklmMapping.import(iov);
  return;
}

void BKLMDatabaseImporter::exportBklmElectronicMapping()
{

  DBArray<BKLMElectronicMapping> elements;
  elements.getEntries();

  // Print mapping info
  B2INFO("DBArray<BKLMElectronicMapping> entries " << elements.getEntries());

  for (const auto& element : elements) {
    if (element.getStripId() == 1) {
      B2INFO("Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
             ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
             ", channelId = " << element.getChannelId() <<
             ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
             " plane(z/phi) = " << element.getPlane() << " stripId = " << element.getStripId());
    }
  }
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

void BKLMDatabaseImporter::importBklmSimulationPar()
{
  GearDir content("/Detector/DetectorComponent/Geometry/BKLM/Content/SimulationParameters");

  // define the data
  BKLMSimulationPar bklmSimulationPar;

  // Get Gearbox simulation parameters for BKLM
  bklmSimulationPar.setVersion(0);
  bklmSimulationPar.read(content);

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
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

void BKLMDatabaseImporter::importBklmBadChannels(int expNoStart, int runStart, int expNoStop, int runStop, std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  BKLMBadChannels bklmBadChannels;

  int isForward(0), sector(0), layer(0), plane(0), strip(0);

  while (true) {
    stream >> isForward >> sector >> layer >> plane >> strip;
    if (stream.eof()) break;
    B2INFO("Read in line" << isForward << ", " << sector << ", " << layer  << ", " << plane  << ", " << strip << ".");
    bklmBadChannels.appendDeadChannel(isForward, sector, layer, plane, strip);
  }
  stream.close();
  // define IOV and store data to the DB
  IntervalOfValidity iov(expNoStart, runStart, expNoStop, runStop);
  Database::Instance().storeData("BKLMBadChannels", &bklmBadChannels, iov);
}

void BKLMDatabaseImporter::exportBklmBadChannels()
{
  DBObjPtr<BKLMBadChannels> element("BKLMBadChannels");

  element->printDeadChannels();

  B2INFO("is (1,1,1,0,20) dead ? " << element->isDeadChannel(1, 1, 1, 0, 20) << "; is (1,1,1,0,21) hot ? " << element->isHotChannel(1,
         1, 1, 0, 21));

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
        B2INFO("bklm misalignment parameter of isForward " << i << ", sector " << j + 1 << ", layer " << k + 1);
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
        B2INFO("bklm alignment parameter of isForward " << i << ", sector " << j + 1 << ", layer " << k + 1);
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
    unsigned short isForward = bklmid.getIsForward();
    unsigned short sector = bklmid.getSectorNumber();
    unsigned short layer = bklmid.getLayerNumber();
    B2INFO("displacement of " << isForward << ", " << sector << ", " << layer << ": " << disp.getUShift() << ", " << disp.getVShift() <<
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

void BKLMDatabaseImporter::importBklmStripEfficiency(int expStart, int runStart, int expStop, int runStop, std::string fileName)
{
  DBImportObjPtr<BKLMStripEfficiency> stripEfficiency;
  stripEfficiency.construct();

  TFile* file = TFile::Open(fileName.c_str(), "r");
  if (!file) {
    B2ERROR("Calibration file: " << fileName << " *** failed to open");
  } else {
    TTree* tree = (TTree*)file->Get("tree");
    if (!tree) {
      B2ERROR("Calibration file: " << fileName << " *** no tree named 'tree' found");
      file->Close();
    } else {
      B2INFO("BKLMDatabaseImporter: file " << fileName << " opened for calibration");

      int isForward = 0;
      tree->SetBranchAddress("isForward", &isForward);
      int sector = 0;
      tree->SetBranchAddress("sector", &sector);
      int layer = 0;
      tree->SetBranchAddress("layer", &layer);
      int plane = 0;
      tree->SetBranchAddress("plane", &plane);
      int strip = 0;
      tree->SetBranchAddress("strip", &strip);
      float efficiency = 1.;
      tree->SetBranchAddress("efficiency", &efficiency);
      float efficiencyError = 0.;
      tree->SetBranchAddress("efficiencyError", &efficiencyError);

      for (int iStrip = 0; iStrip < tree->GetEntries(); iStrip++) {
        tree->GetEntry(iStrip);
        stripEfficiency->setEfficiency(isForward, sector, layer, plane, strip, efficiency, efficiencyError);
      }
    }
    file->Close();
  }

  IntervalOfValidity iov(expStart, runStart, expStop, runStop);
  stripEfficiency.import(iov);

  B2INFO("BKLMDatabaseImporter: strip efficiencies imported and file " << fileName << " closed");
}

void BKLMDatabaseImporter::exportBklmStripEfficiency()
{
}
