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
#include <svd/geosvd/B2GeomSVDSensor.h>
using namespace boost;
using namespace Belle2;
#else
#include "B2GeomSVDSensor.h"
#endif

using namespace std;

B2GeomSVDSensor::B2GeomSVDSensor()
{
  B2GeomSVDSensor(-1, -1, -1, -1);
}

B2GeomSVDSensor::B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen, Int_t iST)
{
  volSVDSensor = 0;
  // check for valid parameters (e.g. Layer 1 or 2 etc.....)

// define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  iSensorType = iST;
  char text[200];
  sprintf(text, "SVD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  path = string(text);

}

B2GeomSVDSensor::~B2GeomSVDSensor()
{

}

#ifdef B2GEOM_BASF2
Bool_t B2GeomSVDSensor::init(GearDir& content)
{
  GearDir sensorContent(content);

  sensorContent.append((format("Sensors/Sensor[@id=\'SVD_Layer_%1%_Ladder_Sensor_%2%\']/SensorParameters/") % iLayer % iSensor).str());

  //Collect sensor data
  fActiveSensorLength      = sensorContent.getParamLength("LengthActive");
  fActiveSensorWidth       = sensorContent.getParamLength("WidthActive");
  if (iSensorType == 2) {
    fActiveSensorWidth2 = sensorContent.getParamLength("Width2Active");
  }
  fActiveSensorThick       = sensorContent.getParamLength("ThicknessActive");

  fSensorLength = sensorContent.getParamLength("Length");
  fSensorWidth = sensorContent.getParamLength("Width");
  if (iSensorType == 2) fSensorWidth2 = sensorContent.getParamLength("Width2");
  fSensorThick = sensorContent.getParamLength("Thickness");


  fSiliconLength = sensorContent.getParamLength("LengthSilicon");
  fSiliconWidth = sensorContent.getParamLength("WidthSilicon");
  if (iSensorType == 2) fSiliconWidth2 = sensorContent.getParamLength("Width2Silicon");
  fSiliconThick = sensorContent.getParamLength("ThicknessSilicon");

  // Get materials
  string sensorMatName  = sensorContent.getParamString("MaterialSensor");
  medSVD_Silicon = gGeoManager->GetMedium(sensorMatName.c_str());

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  return true;

}
#else
Bool_t B2GeomSVDSensor::init()
{
  fActiveSensorLength      = 11.52;
  fActiveSensorWidth       = 5.76;
  if (isWedge) fActiveSensorWidth2 = 3.84;
  fActiveSensorThick       = 0.03;

  fSensorLength = 12.4880;
  fSensorWidth = 5.9600;
  fSensorThick = 0.1;

  if (iLayer == 3) {
    fActiveSensorWidth = 3.84;
  }

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  medSVD_Silicon = new TGeoMedium("medAir", 1, matVacuum);

  return true;
}
#endif

Bool_t B2GeomSVDSensor::make()
{
  volSVDSensor = new TGeoVolumeAssembly(path.c_str());
  putSilicon();
  return true;
}

void B2GeomSVDSensor::putSilicon()
{
  char nameActive[200];
  char nameSilicon[200];
  // define the active volume of the SVD sensor
  // the prefix SD flags the volume as active
  sprintf(nameActive, "SD_%s_ActiveSensor", path.c_str());
  sprintf(nameSilicon, "%s_Silicon", path.c_str());
  if (iSensorType < 2) {
    volSilicon = gGeoManager->MakeBox(nameSilicon, medSVD_Silicon,
                                      fSiliconWidth * 0.5,
                                      fSiliconThick * 0.5,
                                      fSiliconLength * 0.5);

    volActiveSensor = gGeoManager->MakeBox(nameActive, medSVD_Silicon,
                                           fActiveSensorWidth * 0.5,
                                           fActiveSensorThick * 0.5,
                                           fActiveSensorLength * 0.5);

    volSilicon->AddNode(volActiveSensor, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  }
  if (iSensorType == 2) {
    volSilicon = gGeoManager->MakeTrd1(nameSilicon, medSVD_Silicon,
                                       fSiliconWidth * 0.5,
                                       fSiliconWidth2 * 0.5,
                                       fSiliconThick * 0.5,
                                       fSiliconLength * 0.5);

    volActiveSensor = gGeoManager->MakeTrd1(nameActive, medSVD_Silicon,
                                            fActiveSensorWidth * 0.5,
                                            fActiveSensorWidth2 * 0.5,
                                            fActiveSensorThick * 0.5,
                                            fActiveSensorLength * 0.5);

    volSilicon->AddNode(volActiveSensor, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  }

  TGeoRotation rot1("name", 90.0, 0.0, 0.0);
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = rot1 * hmaHelp;
  TGeoHMatrix* hmaActiveSensorPosition = new TGeoHMatrix(hmaHelp);
  volSVDSensor->AddNode(volSilicon, 1, hmaActiveSensorPosition);
}


void B2GeomSVDSensor::putSwitchers()
{
  // TGeoVolume *box = gGeoManager->MakeBox("test", medAir, 0.25, 0.25, 0.25);
  // volSVDSensor->AddNode(box, 1, new TGeoTranslation(-.125-0.5*fSensorThick,0,0));
}

