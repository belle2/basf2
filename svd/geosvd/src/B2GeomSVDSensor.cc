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
  B2GeomSVDSensor(-1, -1, -1);
}

B2GeomSVDSensor::B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen)
{
  B2GeomVolume();
  // check for valid parameters (e.g. Layer 1 or 2 etc.....)

// define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  char text[200];
  sprintf(text, "SVD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  path = string(text);
  volSilicon = NULL;
  volFoam = NULL;
  volSMDs = NULL;
  volKapton = NULL;
}

B2GeomSVDSensor::~B2GeomSVDSensor()
{

}

Bool_t B2GeomSVDSensor::init(GearDir& content)
{

  GearDir sensorContent(content);

  sensorContent.append((format("Sensors/Sensor[@id=\'SVD_Layer_%1%_Ladder_Sensor_%2%\']/") % iLayer % iSensor).str());

  //Read sensor data
  initBasicParameters(sensorContent);

  // Read parameters for silicon
  if (sensorContent.isParamAvailable("Silicon")) {
    volSilicon = new B2GeomSVDSensorSilicon(iLayer, iLadder, iSensor);
    if (!volSilicon->init(sensorContent)) {
      printf("ERROR! Parameter reading for SVD silicon failed!\n");
      return false;
    }
  } else {
    printf("ERROR! Definition of Silicon missing in XML file!\n");
    return false;
  }

  if (sensorContent.isParamAvailable("Foam")) {
    volFoam = new B2GeomSVDSensorFoam();
    volFoam->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("Kapton")) {
    volKapton = new B2GeomSVDSensorKapton();
    volKapton->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("SMDs")) {
    volSMDs = new B2GeomSVDSensorSmds;
    volSMDs->init(sensorContent);
  }

  return true;

}

Bool_t B2GeomSVDSensor::make()
{
  printf("SVDSensor::make start (Lay: %i, Lad: %i, Sen: %i\n", iLayer, iLadder, iSensor);
  // create container for SVD sensor components
  tVolume = new TGeoVolumeAssembly(path.c_str());

  // add silicon to container
  printf("make vol silicon\n");
  volSilicon->make();
  printf("add vol silicon\n");
  tVolume->AddNode(volSilicon->getVol(), 1, volSilicon->getPosition());
  printf("vol silicon added\n");

  // add the other components to the container
  if (volFoam != NULL) {
    volFoam->make();
    tVolume->AddNode(volFoam->getVol(), 1, volFoam->getPosition());
  }
  if (volKapton != NULL) {
    volKapton->make();
    tVolume->AddNode(volKapton->getVol(), 1, volKapton->getPosition());
  }
  if (volSMDs != NULL) {
    volSMDs->make();
    tVolume->AddNode(volSMDs->getVol(), 1, volSMDs->getPosition());
  }
  printf("SVDSensor::make stop \n");
  return true;
}

// ------------------------------------------------------------------------------------------------
// Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorSilicon::B2GeomSVDSensorSilicon(Int_t iLay, Int_t iLad, Int_t iSen)
{
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  B2GeomVolume();
  volActive = new B2GeomSVDSensorActive(iLayer, iLadder, iSensor);
  char text[200];
  sprintf(text, "SVD_Layer_%i_Ladder_%i_Sensor_%i_Silicon", iLayer, iLadder, iSensor);
  path = string(text);
}

Bool_t B2GeomSVDSensorSilicon::init(GearDir& content)
{
  GearDir siliconContent(content);
  siliconContent.append("Silicon/");
  initBasicParameters(siliconContent);

  // initialize the contained active volume
  // Read parameters for active sensor
  if (siliconContent.isParamAvailable("Active")) {
    if (!volActive->init(siliconContent)) {
      printf("ERROR! Parameter reading for SVD active silicon failed!\n");
      return false;
    };
  } else {
    printf("ERROR! Definition of Active sensor missing in XML file!\n");
    return false;
  }

  return true;
}

Bool_t B2GeomSVDSensorSilicon::make()
{
  printf("SVD make sensor silicon\n");
  if (!volActive->make()) {
    printf("ERROR! Cannot create TGeoVolume for SVD active silicon!\n");
    return false;
  };

  char nameSilicon[200];
  sprintf(nameSilicon, "%s", path.c_str());
  tVolume = gGeoManager->MakeTrd1(nameSilicon, tMedium,
                                  fWidth * 0.5,
                                  fWidth2 * 0.5,
                                  fThickness * 0.5,
                                  fLength * 0.5);
  printf("box for sensor silicon made\n");
  tVolume->SetLineColor(kBlue + 3);
  printf("add now active volume!\n");
  tVolume->AddNode(volActive->getVol(), 1, volActive->getPosition());
  printf("active volume added\n");
  return true;
}

// ------------------------------------------------------------------------------------------------
// Active Silicon part of the sensor
// ------------------------------------------------------------------------------------------------


B2GeomSVDSensorActive::B2GeomSVDSensorActive(Int_t iLay, Int_t iLad, Int_t iSen)
{
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  B2GeomVolume();
  char text[200];
  sprintf(text, "SD_SVD_Layer_%i_Ladder_%i_Sensor_%i_Silicon_Active", iLayer, iLadder, iSensor);
  path = string(text);
}

Bool_t B2GeomSVDSensorActive::init(GearDir& content)
{
  GearDir activeContent(content);
  activeContent.append("Active/");
  initBasicParameters(activeContent);
  return true;
}

Bool_t B2GeomSVDSensorActive::make()
{
  char nameActive[200];
  // define the active volume of the SVD sensor
  // the prefix SD flags the volume as active
  sprintf(nameActive, "%s", path.c_str());
  tVolume = gGeoManager->MakeTrd1(nameActive, tMedium,
                                  fWidth * 0.5,
                                  fWidth2 * 0.5,
                                  fThickness * 0.5,
                                  fLength * 0.5);
  tVolume->SetLineColor(kBlue - 10);
  return true;

}

// ------------------------------------------------------------------------------------------------
// Rohacell Foam Layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorFoam::B2GeomSVDSensorFoam()
{
  B2GeomVolume();
}


Bool_t B2GeomSVDSensorFoam::init(GearDir& content)
{
  GearDir foamContent(content);
  foamContent.append("Foam/");
  initBasicParameters(foamContent);
  return true;
}

Bool_t B2GeomSVDSensorFoam::make()
{
  char nameFoam[200];
  sprintf(nameFoam, "SVD_Foam");
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(nameFoam);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd1(nameFoam, tMedium,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
                                    fThickness * 0.5,
                                    fLength * 0.5);
    tVolume->SetLineColor(kYellow - 9);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// Kapton layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorKapton::B2GeomSVDSensorKapton()
{
  B2GeomVolume();
}

Bool_t B2GeomSVDSensorKapton::init(GearDir& content)
{
  GearDir kaptonContent(content);
  kaptonContent.append("Kapton/");
  initBasicParameters(kaptonContent);
  return true;
}

Bool_t B2GeomSVDSensorKapton::make()
{
  char nameKapton[200];
  sprintf(nameKapton, "SVD_Kapton");
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(nameKapton);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd1(nameKapton, tMedium,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
                                    fThickness * 0.5,
                                    fLength * 0.5);
    tVolume->SetLineColor(kOrange + 2);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// SMDs of SVD senor
// ------------------------------------------------------------------------------------------------


B2GeomSVDSensorSmds::B2GeomSVDSensorSmds()
{
  B2GeomVolume();
}
Bool_t B2GeomSVDSensorSmds::init(GearDir& content)
{
  GearDir smdsContent(content);
  smdsContent.append("SMDs/");
  initBasicParameters(smdsContent);
  return true;
}

Bool_t B2GeomSVDSensorSmds::make()
{
  char nameSMDs[200];
  sprintf(nameSMDs, "SVD_SMDs");


  tVolume = (TGeoVolume*) gROOT->FindObjectAny(nameSMDs);
  if (!tVolume) {
    tVolume = gGeoManager->MakeBox(nameSMDs, tMedium,
                                   0.5 * fThickness,
                                   0.5 * fWidth,
                                   0.5 * fLength);
    tVolume->SetLineColor(kBlue + 4);
  }
  return true;
}









