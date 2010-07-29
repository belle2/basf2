/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#define B2GEOM_BASF2

#ifdef B2GEOM_BASF2
#include <pxd/geopxd/B2GeomPXDSensor.h>
using namespace boost;
using namespace Belle2;
#else
#include "B2GeomPXDSensor.h"
#endif

using namespace std;

B2GeomPXDSensor::B2GeomPXDSensor()
{
  B2GeomPXDSensor(-1, -1, -1);
}

B2GeomPXDSensor::B2GeomPXDSensor(Int_t iLay, Int_t iLad, Int_t iSen)
{
  volPXDSensor = 0;
  // check for valid parameters (e.g. Layer 1 or 2 etc.....)

// define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  char text[200];
  sprintf(text, "PXD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  path = string(text);

}

B2GeomPXDSensor::~B2GeomPXDSensor()
{

}

#ifdef B2GEOM_BASF2
Bool_t B2GeomPXDSensor::init(GearDir& content)
{
  GearDir sensorContent(content);
  sensorContent.append("Sensor/");

  /*
      //Collect sensor data
  double sensorLength      = layerContent.getParamLength("Length");
  double sensorGap         = layerContent.getParamLength("Gap");
  double nSensor = int(layerContent.getParamNumValue("NumberOfSensors"));;
  double sensorRimWidthZ   = ((ladderLength - (nSensor - 1) * sensorGap) / nSensor - sensorLength) / 2.0; //rim around active sensor + active sensor length
  if (sensorRimWidthZ < (0.00001)) sensorRimWidthZ = 0; // Check if sensor rim not below precision (0.01 um)
  */

  // check if this sensor has been already created??
  fSensorLength = 1.1 * sensorContent.getParamLength("Length");
  fSensorWidth = 1.1 * sensorContent.getParamLength("Width");
  fSensorThick = 1.1 * sensorContent.getParamLength("Thickness");

  fActiveSensorLength      = sensorContent.getParamLength("Length");
  fActiveSensorWidth       = sensorContent.getParamLength("Width");
  fActiveSensorThick       = sensorContent.getParamLength("Thickness");

  // Get materials
  string sensorMatName  = sensorContent.getParamString("MaterialSensor");
  medDEPFET = gGeoManager->GetMedium(sensorMatName.c_str());
  medActiveSensor = gGeoManager->GetMedium(sensorMatName.c_str());

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);

  return true;

}
#else
Bool_t B2GeomPXDSensor::init()
{
  // check if this sensor has been already created??
  fSensorLength = 10;
  fSensorThick = .05;
  fSensorWidth = 1.;

  fActiveSensorLength      = 9.;
  fActiveSensorWidth =  .7;
  fActiveSensorThick      = .0075;

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);

  // Get materials
  medDEPFET =  new TGeoMedium("medDEPFET", 1, matVacuum);
  medActiveSensor =  new TGeoMedium("medActiveSensor", 1, matVacuum);

  return true;

}
#endif

Bool_t B2GeomPXDSensor::make()
{
  volPXDSensor = new TGeoVolumeAssembly(path.c_str());
  putDEPFET();
  putSwitchers();
  return true;
}

void B2GeomPXDSensor::putDEPFET()
{
  char name[200];

  // create DEPFET
  sprintf(name, "%s_DEPFET", path.c_str());
  volDEPFET = gGeoManager->MakeBox(name, medAir,
                                   0.5 * fSensorThick,
                                   0.5 * fSensorWidth,
                                   0.5 * fSensorLength);

  volDEPFET->SetLineColor(kBlue);
  // add DEPFET to the PXDsensor
  volPXDSensor->AddNode(volDEPFET, 1,
                        new TGeoTranslation(0.,
                                            0.,
                                            0.));

  // create volume for the thinned out part of DEPFET
  sprintf(name, "%s_Air", path.c_str());
  volAir = gGeoManager->FindVolumeFast(name);
  if (!volAir) {
    volAir = gGeoManager->MakeBox(name, medAir,
                                  0.5 * (fSensorThick - fActiveSensorThick),
                                  0.5 * fActiveSensorWidth,
                                  0.5 * fActiveSensorLength);

  }
  volAir->SetLineColor(kMagenta);



  volDEPFET->AddNode(volAir, 1,
                     new TGeoTranslation((0.5 * fSensorThick) - (0.5 *(fSensorThick - fActiveSensorThick)) ,
                                         0.,
                                         0.));




  // define the active volume of the DEPFET
  // the prefix SD flags the volume as active
  sprintf(name, "SD_%s_ActiveSensor", path.c_str());
  TGeoVolume* volActiveSensor = gGeoManager->MakeBox(name, medActiveSensor,
                                                     0.5 * fActiveSensorThick,
                                                     0.5 * fActiveSensorWidth,
                                                     0.5 * fActiveSensorLength);

  volDEPFET->AddNode(volActiveSensor, 1,
                     new TGeoTranslation((-0.5 * fSensorThick) + (0.5 * fActiveSensorThick),
                                         0.,
                                         0.));

  volActiveSensor->SetLineColor(kGreen);


}

void B2GeomPXDSensor::putSwitchers()
{
  TGeoVolume *box = gGeoManager->MakeBox("test", medAir, 0.25, 0.25, 0.25);
  // volPXDSensor->AddNode(box, 1, new TGeoTranslation(-.125-0.5*fSensorThick,0,0));
}

