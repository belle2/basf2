/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <svd/calibration/SVDCoGCalibrationsImporter.h>

#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <framework/utilities/FileSystem.h>

#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <vxd/dataobjects/VxdID.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TVectorF.h>

#include <vxd/geometry/GeoCache.h>

#include <TFile.h>
#include <TH1F.h>

#include <framework/dataobjects/EventMetaData.h>


using namespace std;
using namespace Belle2;

int SVDCoGCalibrationsImporter::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
{
  int Index;

  if (LayerNumber == 3) { //L3
    if (UVNumber) //U
      Index = 0;
    else //V
      Index = 1;
  } else { //L456
    if (SensorNumber == 1) { //FW
      if (UVNumber) //U
        Index = 2;
      else //V
        Index = 3;
    } else { //barrel
      if (UVNumber) //U
        Index = 4;
      else //V
        Index = 5;
    }
  }

  return Index;
}

TString SVDCoGCalibrationsImporter::IntExtFromIndex(int idx)
{
  TString name = "";

  if (idx < 2)
    name = "L3";
  else
    name = "L456";

  return name;
}

TString SVDCoGCalibrationsImporter::FWFromIndex(int idx)
{
  TString name = "";

  if (idx == 2 || idx == 3)
    name = "FWD";
  else
    name = "Barrel";

  return name;
}

void SVDCoGCalibrationsImporter::importSVDTimeShiftCorrections(bool m_fillAreas, TString filename)
{
  DBImportObjPtr<SVDPulseShapeCalibrations::t_time_payload> svdTimeShiftCal(SVDPulseShapeCalibrations::time_name);

  svdTimeShiftCal.construct(25);

  m_firstExperiment = 3;
  m_firstRun = 400;
  m_lastExperiment = 3;
  m_lastRun = 400;
  //I later have to get these numbers from somewhere else... store them in the root file containing the histos!

  TFile* fff = new TFile(filename.Data());

  TString folder = "CoGTimeDistribution_perSensor";
  if (m_fillAreas)
    folder = "CoGTime_areas";

  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();
  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers
    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();
    while (itSvdLadders != svdLadders.end()) { //loop on Ladders
      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      while (itSvdSensors != svdSensors.end()) { //loop on Sensors
        for (int side = 0; side < 2; side++) {
          int layer = itSvdSensors->getLayerNumber();
          int ladder =  itSvdSensors->getLadderNumber();
          int sensor = itSvdSensors->getSensorNumber();

          TString sidename = "U";
          if (side == 0)
            sidename = "V";

          int maxStripNumber = 512;
          if (layer == 3 || side == 1)
            maxStripNumber = 768;

          float meanCoG = -1000.;
          if (m_fillAreas) { //fill DB with area-based events
            int indexForHistosAndGraphs = indexFromLayerSensorSide(layer, sensor, side);

            TH1F* CoGTime_areas = (TH1F*)fff->Get(folder + "/CoG_SensorTime_" + IntExtFromIndex(indexForHistosAndGraphs) + "." + FWFromIndex(
                                                    indexForHistosAndGraphs) + "." + sidename);
            meanCoG = CoGTime_areas->GetMean();

            for (int strip = 0; strip < maxStripNumber; strip++) {
              svdTimeShiftCal->set(layer, ladder, sensor, side, strip, meanCoG);
            }
          } else { //fill DB with sensor-based events
            std::string layername = std::to_string(layer);
            std::string laddername = std::to_string(ladder);
            std::string sensorname = std::to_string(sensor);
            TH1F* CoGTime_sensors = (TH1F*)fff->Get(folder + "/CoGTime_" + layername + "." + laddername + "." + sensorname + "." + sidename);
            meanCoG = CoGTime_sensors->GetMean();

            for (int strip = 0; strip < maxStripNumber; strip++) {
              svdTimeShiftCal->set(layer, ladder, sensor, side, strip, meanCoG);
            }
          }
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  IntervalOfValidity iov(m_firstExperiment, m_firstRun, m_lastExperiment, m_lastRun);

  svdTimeShiftCal.import(iov);

  B2RESULT("SVDTimeShiftCorrections imported to database.");
}


