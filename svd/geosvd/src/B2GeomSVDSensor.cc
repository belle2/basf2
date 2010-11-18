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

B2GeomSVDSensor::B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen)
{
  resetBasicParameters();
// define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  sprintf(name, "SVD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  volSilicon = NULL;
  volFoam = NULL;
  volSMDs = NULL;
  volKapton = NULL;
  volKaptonFlex = NULL;
  volKaptonFront = NULL;
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
    volFoam = new B2GeomVolume();
    volFoam->init(sensorContent, "Foam/");
  }

  if (sensorContent.isParamAvailable("Kapton")) {
    volKapton = new B2GeomVolume();
    volKapton->init(sensorContent, "Kapton/");
  }

  if (sensorContent.isParamAvailable("KaptonFlex")) {
    volKaptonFlex = new B2GeomVolume();
    volKaptonFlex->init(sensorContent, "KaptonFlex/");
  }

  if (sensorContent.isParamAvailable("KaptonFront")) {
    volKaptonFront = new B2GeomVolume();
    volKaptonFront->init(sensorContent, "KaptonFront/");
  }

  if (sensorContent.isParamAvailable("SMDs")) {
    volSMDs = new B2GeomVolume();
    volSMDs->init(sensorContent, "SMDs/");
  }

  sensorContent.setDirPath((format("//Offsets/Sensor[@id=\'SVD_Offset_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
  initOffsets(sensorContent);

  return true;

}

Bool_t B2GeomSVDSensor::make()
{
  // create container for SVD sensor components
//printf("B2GeomSVDSensor::make start\n");
  makeGeneric();

  // add silicon to container
  volSilicon->make();
  tVolume->AddNode(volSilicon->getVol(), 1, volSilicon->getPosition());

  // add the other components to the container
  if (volFoam != NULL) {
    volFoam->make();
    tVolume->AddNode(volFoam->getVol(), 1, volFoam->getPosition());
  }
  if (volKapton != NULL) {
    volKapton->make();
    tVolume->AddNode(volKapton->getVol(), 1, volKapton->getPosition());
  }
  if (volKaptonFront != NULL) {
    volKaptonFront->make();
    tVolume->AddNode(volKaptonFront->getVol(), 1, volKaptonFront->getPosition());
  }

  if (volKaptonFlex != NULL) {
    if (!volKaptonFlex->make()) {
      printf("ERROR!\n");
      return false;
    }
    tVolume->AddNode(volKaptonFlex->getVol(), 1, volKaptonFlex->getPosition());
  }
  if (volSMDs != NULL) {
    volSMDs->make();
    tVolume->AddNode(volSMDs->getVol(), 1, volSMDs->getPosition());
  }
  //printf("B2GeomSVDSensor::make stop\n");
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
  resetBasicParameters();
  volActive = NULL;
  sprintf(name, "SVD_Layer_%i_Ladder_%i_Sensor_%i_Silicon", iLayer, iLadder, iSensor);
}

Bool_t B2GeomSVDSensorSilicon::init(GearDir& content)
{
  GearDir siliconContent(content);
  siliconContent.append("Silicon/");
  initBasicParameters(siliconContent);

  // initialize the contained active volume
  // Read parameters for active sensor
  if (siliconContent.isParamAvailable("Active")) {
    volActive = new B2GeomSVDSensorActive(iLayer, iLadder, iSensor);
    if (!volActive->init(siliconContent, "Active/")) {
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
  makeGeneric();

  if (!volActive->make()) {
    printf("ERROR! Cannot create TGeoVolume for SVD active silicon!\n");
    return false;
  };
  tVolume->AddNode(volActive->getVol(), 1, volActive->getPosition());
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
  resetBasicParameters();
  sprintf(name, "SD_SVD_Layer_%i_Ladder_%i_Sensor_%i_Silicon_Active", iLayer, iLadder, iSensor);
}

















