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
  GearDir dir("/Detector/ElectronicsMapping/BKLM");

// define data array
  TClonesArray bklmMapping("Belle2::BKLMElectronicMapping");
  int index = 0;

// loop over xml files and extract the data
  for (GearDir& copper : dir.getNodes("COPPER")) {

    int copperId = copper.getInt("@id");

    for (GearDir& slot : copper.getNodes("Slot")) {
      int slotId = slot.getInt("@id");
      //B2DEBUG(1, "slotid: " << slotId);
      for (GearDir& lane : slot.getNodes("Lane")) {
        int laneId = lane.getInt("@id");
        for (GearDir& axis : lane.getNodes("Axis")) {
          int axisId = axis.getInt("@id");
          int sector = axis.getInt("Sector");
          int isForward = axis.getInt("IsForward");
          int layer = axis.getInt("Layer");
          int plane = axis.getInt("Plane");
          B2DEBUG(1, "reading xml file...");
          B2DEBUG(1, " copperId: " << copperId << " slotId: " << slotId << " laneId: " << laneId << " axisId: " << axisId);
          B2DEBUG(1, " sector: " << sector << " isforward: " << isForward << " layer: " << layer << " plane: " << plane);
          // save data as an element of the array
          new(bklmMapping[index]) BKLMElectronicMapping(0, copperId, slotId, laneId, axisId, isForward, sector, layer, plane);
          index++;
        }
      }
    }

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

// store under default name:
//Database::Instance().storeData(&bklmMapping, iov);

// store under user defined name:
  Database::Instance().storeData("BKLMElectronicMapping", &bklmMapping, iov);

}

void BKLMDatabaseImporter::exportBklmElectronicMapping()
{

  DBArray<BKLMElectronicMapping> elements("BKLMElectronicMapping");
  elements.getEntries();

  // Print mapping info
  B2INFO("DBArray<BKLMElectronicMapping> entries " << elements.getEntries());

  for (const auto& element : elements) {
    B2INFO("Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
           ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
           ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
           " plane(z/phi) = " << element.getPlane());
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
