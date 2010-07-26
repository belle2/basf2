/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDSensor.h>

using namespace boost;
using namespace Belle2;
using namespace std;

B2GeomSVDSensor::B2GeomSVDSensor()
{
  B2GeomSVDSensor(-1, -1, -1);
}

B2GeomSVDSensor::B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen, Bool_t isW)
{
  volSVDSensor = 0;
  // check for valid parameters (e.g. Layer 1 or 2 etc.....)

// define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  isWedge = isW;
  path = (format("SVD_Layer_%1%_Ladder_%2%_Sensor_%3%") % iLayer % iLadder % iSensor).str();

}

B2GeomSVDSensor::~B2GeomSVDSensor()
{

}

Bool_t B2GeomSVDSensor::init(GearDir& content)
{
  GearDir sensorContent(content);
  if (!isWedge) sensorContent.append("Sensor/");
  if (isWedge) sensorContent.append("SlantedSensor/");

  //Collect sensor data
  //sensorContent.append("Sensor/");
  fActiveSensorLength      = sensorContent.getParamLength("Length");
  fActiveSensorWidth       = sensorContent.getParamLength("Width");
  if (isWedge) fActiveSensorWidth2      = sensorContent.getParamLength("Width2");
  // sensorGap         = sensorContent.getParamLength("Gap");
  fActiveSensorThick       = sensorContent.getParamLength("Thickness");

  // check if this sensor has been already created??
  fSensorLength = 1.1 * sensorContent.getParamLength("Length");
  fSensorWidth = 1.1 * sensorContent.getParamLength("Width");
  fSensorThick = 1.1 * sensorContent.getParamLength("Thickness");

  // Get materials
  string sensorMatName  = sensorContent.getParamString("MaterialSensor");
  medActiveSensor = gGeoManager->GetMedium(sensorMatName.c_str());

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);

  return true;

}


Bool_t B2GeomSVDSensor::make()
{
  volSVDSensor = new TGeoVolumeAssembly(path.c_str());
  if (!isWedge) putSensor();
  if (isWedge) putSensorWedge();
  putSwitchers();
  return true;
}

void B2GeomSVDSensor::putSensor()
{
  char name[200];
  // define the active volume of the SVD sensor
  // the prefix SD flags the volume as active
  sprintf(name, "SD_%s_ActiveSensor", path.c_str());
  TGeoVolume* volActiveSensor = gGeoManager->MakeBox(name, medActiveSensor,
                                                     fActiveSensorThick * 0.5,
                                                     fActiveSensorWidth * 0.5,
                                                     fActiveSensorLength * 0.5);
  volActiveSensor->SetLineColor(kRed);
  volSVDSensor->AddNode(volActiveSensor, 1, new TGeoTranslation(0.0, 0.0, 0.0));
}

void B2GeomSVDSensor::putSensorWedge()
{
  char name[200];
  // define the active volume of the SVD sensor
  // the prefix SD flags the volume as active
  sprintf(name, "SD_%s_ActiveSensor", path.c_str());
  TGeoVolume* volActiveSensor = gGeoManager->MakeTrd1(name, medActiveSensor,
                                                      fActiveSensorWidth * 0.5,
                                                      fActiveSensorWidth2 * 0.5,
                                                      fActiveSensorThick * 0.5,
                                                      fActiveSensorLength * 0.5);
  volActiveSensor->SetLineColor(kRed);
  volSVDSensor->AddNode(volActiveSensor, 1, new TGeoRotation("name", 90.0, 0.0, 0.0));
}

void B2GeomSVDSensor::putSwitchers()
{
  TGeoVolume *box = gGeoManager->MakeBox("test", medAir, 0.25, 0.25, 0.25);
  // volSVDSensor->AddNode(box, 1, new TGeoTranslation(-.125-0.5*fSensorThick,0,0));
}

