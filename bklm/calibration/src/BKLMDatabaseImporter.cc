/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/calibration/BKLMDatabaseImporter.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>
#include <bklm/dbobjects/BKLMGeometryPar.h>
#include <bklm/dbobjects/BKLMSimulationPar.h>
#include <bklm/dbobjects/BKLMBadChannels.h>
#include <bklm/dbobjects/BKLMMisAlignment.h>
#include <bklm/dbobjects/BKLMDisplacement.h>
#include <bklm/dbobjects/BKLMScinDigitizationParams.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <bklm/dbobjects/BKLMTimeWindow.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <bklm/dataobjects/BKLMElementID.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBImportArray.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <TClonesArray.h>

using namespace std;
using namespace Belle2;

BKLMDatabaseImporter::BKLMDatabaseImporter()
{}

void BKLMDatabaseImporter::importBklmElectronicMapping()
{
  DBImportArray<BKLMElectronicMapping> m_bklmMapping;
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
            if (plane == 0 && layer < 3) MaxiChannel = 38;
            if (plane == 0 && layer > 2) MaxiChannel = 34;
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
              if (layer < 3) {
                if (channelId > 45 && channelId < 55) channelId = channelId - 45;
                else if (channelId > 30 && channelId < 46) channelId = channelId - 15;
                else if (channelId > 15 && channelId < 31) channelId = channelId + 15;
                else if (channelId > 0 && channelId < 16)  channelId = channelId + 45;
              }
            }

            m_bklmMapping.appendNew(1, copperId, slotId, laneId, axisId, channelId, isForward, sector, layer, plane, iStrip);
          }// end of loop channels
        }//end of loop plane
      }//end of loop layers
    }//end of loop sectors
  }//end fb
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
  GearDir content("/Detector/DetectorComponent/Geometry/BKLM/Content");

  // define the data
  // TObject bklmGeometryPar("Belle2::BKLMGeometryPar");
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
  // TObject bklmGeometryPar("Belle2::BKLMGeometryPar");
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

void BKLMDatabaseImporter::importBklmBadChannels()
{

  BKLMBadChannels bklmBadChannels;
  //bklmBadChannels.appendDeadChannel(1, 1, 1, 0, 20);
  //bklmBadChannels.appendHotChannel(1, 1, 1, 0, 21);

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("BKLMBadChannels", &bklmBadChannels, iov);

}

void BKLMDatabaseImporter::exportBklmBadChannels()
{
  DBObjPtr<BKLMBadChannels> element("BKLMBadChannels");

  element->printHotChannels();
  element->printDeadChannels();

  B2INFO("is (1,1,1,0,20) dead ? " << element->isDeadChannel(1, 1, 1, 0, 20) << "; is (1,1,1,0,21) hot ? " << element->isHotChannel(1,
         1, 1, 0, 21));

}

void BKLMDatabaseImporter::importBklmMisAlignment()
{

  //auto bklm = new BKLMMisAlignment;
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
//auto IoV = IntervalOfValidity(0, 0, -1, -1);
  mal.import(Iov);
//Database::Instance().storeData(mal, IoV);

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

  //auto bklm = new BKLMAlignment;
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
//auto IoV = IntervalOfValidity(0, 0, -1, -1);
  al.import(Iov);
//Database::Instance().storeData(mal, IoV);

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

  //DBImportObjPtr<BKLMDisplacement> dis;
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
  //auto IoV = IntervalOfValidity(0, 0, -1, -1);
  m_displacement.import(Iov);
  //Database::Instance().storeData(mal, Iov); // *not* IoV);

}

void BKLMDatabaseImporter::exportBklmDisplacement()
{

  //DBObjPtr<BKLMDisplacement> element("BKLMDisplacement");
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
    //B2INFO(" " << endl);
  }//end loop layer
}

void BKLMDatabaseImporter::importBklmDigitizationParams()
{

  DBImportObjPtr<BKLMScinDigitizationParams> scinDigitzationParm;
  scinDigitzationParm.construct();
  GearDir dig("/Detector/DetectorComponent[@name=\"BKLM\"]/"
              "Content/DigitizationParams");
  scinDigitzationParm->setADCRange(dig.getInt("ADCRange"));
  scinDigitzationParm->setADCSamplingTime(dig.getDouble("ADCSamplingTime"));
  scinDigitzationParm->setNDigitizations(dig.getInt("nDigitizations"));
  scinDigitzationParm->setNPEperMeV(dig.getDouble("nPEperMeV"));
  scinDigitzationParm->setMinCosTheta(cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI));
  scinDigitzationParm->setMirrorReflectiveIndex(dig.getDouble("MirrorReflectiveIndex"));
  scinDigitzationParm->setScintillatorDeExcitationTime(dig.getDouble("ScintDeExTime"));
  scinDigitzationParm->setFiberDeExcitationTime(dig.getDouble("FiberDeExTime"));
  scinDigitzationParm->setFiberLightSpeed(dig.getDouble("FiberLightSpeed"));
  scinDigitzationParm->setAttenuationLength(dig.getDouble("AttenuationLength"));
  //scinDigitzationParm->setPEAttenuationFrequency(dig.getDouble("PEAttenuationFreq"));
  scinDigitzationParm->setPEAttenuationFrequency(1.0 / 8.75); // from T2K paper by F. Retiere: PoS (PD07) 017)
  scinDigitzationParm->setMeanSiPMNoise(dig.getDouble("MeanSiPMNoise"));
  scinDigitzationParm->setEnableConstBkg(dig.getDouble("EnableConstBkg") > 0);
  scinDigitzationParm->setTimeResolution(dig.getDouble("TimeResolution"));

  IntervalOfValidity iov(0, 0, -1, -1);
  scinDigitzationParm.import(iov);

}

void BKLMDatabaseImporter::exportBklmDigitizationParams()
{

  DBObjPtr<BKLMScinDigitizationParams> element("BKLMScinDigitizationParams");
  B2INFO(" scintillator Digitization parameters: ");
  B2INFO("ADC range " << element->getADCRange());
  B2INFO("ADCSamplingTime " << element->getADCSamplingTime());
  B2INFO("nDigitizations " << element->getNDigitizations());
  B2INFO("nPEperMeV "      << element->getNPEperMeV());
  B2INFO("minCosTheta "    << element->getMinCosTheta());
  B2INFO("MirrorReflectiveIndex " << element->getMirrorReflectiveIndex());
  B2INFO("scintillatorDeExcitationTime " << element->getScintillatorDeExcitationTime());
  B2INFO("fiberDeExcitationTime " << element->getFiberDeExcitationTime());
  B2INFO("fiberLightSpeed " << element->getFiberLightSpeed());
  B2INFO("attenuationLength " << element->getAttenuationLength());
  B2INFO("PEAttenuationFreq " << element->getPEAttenuationFrequency());
  B2INFO("meanSiPMNoise " << element->getMeanSiPMNoise());
  B2INFO("enableConstBkg " << element->getEnableConstBkg());
  B2INFO("timeResolution " << element->getTimeResolution());

}

void BKLMDatabaseImporter::importBklmADCThreshold()
{

  DBImportObjPtr<BKLMADCThreshold> m_ADCParam;
  m_ADCParam.construct();
  GearDir params("/Detector/DetectorComponent[@name=\"BKLM\"]/"
                 "Content/DigitizationParams");
  m_ADCParam->setMPPCGain(params.getDouble("MPPCGain"));
  m_ADCParam->setADCOffset(params.getInt("ADCOffset"));
  m_ADCParam->setADCThreshold(params.getDouble("ADCThreshold"));

  IntervalOfValidity iov(0, 0, -1, -1);
  m_ADCParam.import(iov);
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
