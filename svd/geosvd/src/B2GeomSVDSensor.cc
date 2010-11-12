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
    volFoam = new B2GeomSVDSensorFoam();
    volFoam->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("Kapton")) {
    volKapton = new B2GeomSVDSensorKapton();
    volKapton->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("KaptonFlex")) {
    volKaptonFlex = new B2GeomSVDSensorKaptonFlex();
    volKaptonFlex->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("KaptonFront")) {
    volKaptonFront = new B2GeomSVDSensorKaptonFront();
    volKaptonFront->init(sensorContent);
  }

  if (sensorContent.isParamAvailable("SMDs")) {
    volSMDs = new B2GeomSVDSensorSmds;
    volSMDs->init(sensorContent);
  }

  sensorContent.setDirPath((format("//Offsets/Sensor[@id=\'SVD_Offset_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
  initOffsets(sensorContent);

  return true;

}

Bool_t B2GeomSVDSensor::make()
{
  // create container for SVD sensor components
  tVolume = new TGeoVolumeAssembly(name);

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
    volKaptonFlex->make();
    tVolume->AddNode(volKaptonFlex->getVol(), 1, volKaptonFlex->getPosition());
  }
  if (volSMDs != NULL) {
    volSMDs->make();
    tVolume->AddNode(volSMDs->getVol(), 1, volSMDs->getPosition());
  }
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
  volActive = new B2GeomSVDSensorActive(iLayer, iLadder, iSensor);
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
  if (!volActive->make()) {
    printf("ERROR! Cannot create TGeoVolume for SVD active silicon!\n");
    return false;
  };

  tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                  fThickness * 0.5,
                                  fThickness2 * 0.5,
                                  fWidth * 0.5,
                                  fWidth2 * 0.5,
                                  fLength * 0.5);
  tVolume->SetLineColor(kBlue + 3);
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

Bool_t B2GeomSVDSensorActive::init(GearDir& content)
{
  GearDir activeContent(content);
  activeContent.append("Active/");
  initBasicParameters(activeContent);
  return true;
}

Bool_t B2GeomSVDSensorActive::make()
{
  tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                  fThickness * 0.5,
                                  fThickness2 * 0.5,
                                  fWidth * 0.5,
                                  fWidth2 * 0.5,
                                  fLength * 0.5);
  tVolume->SetLineColor(kBlue - 10);
  return true;

}

// ------------------------------------------------------------------------------------------------
// Rohacell Foam Layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorFoam::B2GeomSVDSensorFoam()
{
  resetBasicParameters();
}


Bool_t B2GeomSVDSensorFoam::init(GearDir& content)
{
  GearDir foamContent(content);
  foamContent.append("Foam/");
  initBasicParameters(foamContent);
  sprintf(name, "SVD_Foam");
  return true;
}

Bool_t B2GeomSVDSensorFoam::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    fThickness * 0.5,
                                    fThickness2 * 0.5,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
                                    fLength * 0.5);
    tVolume->SetLineColor(kYellow - 9);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// upper Kapton layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorKapton::B2GeomSVDSensorKapton()
{
  resetBasicParameters();
}

Bool_t B2GeomSVDSensorKapton::init(GearDir& content)
{
  GearDir kaptonContent(content);
  kaptonContent.append("Kapton/");
  initBasicParameters(kaptonContent);
  sprintf(name, "SVD_Kapton");
  return true;
}

Bool_t B2GeomSVDSensorKapton::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    fThickness * 0.5,
                                    fThickness2 * 0.5,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
                                    fLength * 0.5);
    tVolume->SetLineColor(kOrange + 2);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// lower KaptonFlex layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorKaptonFlex::B2GeomSVDSensorKaptonFlex()
{
  resetBasicParameters();
}

Bool_t B2GeomSVDSensorKaptonFlex::init(GearDir& content)
{
  GearDir kaptonContent(content);
  kaptonContent.append("KaptonFlex/");
  initBasicParameters(kaptonContent);
  sprintf(name, "SVD_Kapton_Flex");
  return true;
}

Bool_t B2GeomSVDSensorKaptonFlex::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    fThickness * 0.5,
                                    fThickness2 * 0.5,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
                                    fLength * 0.5);
    tVolume->SetLineColor(kOrange + 2);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// lower KaptonFrontRead layer of SVD senor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorKaptonFront::B2GeomSVDSensorKaptonFront()
{
  resetBasicParameters();
}

Bool_t B2GeomSVDSensorKaptonFront::init(GearDir& content)
{
  GearDir kaptonContent(content);
  kaptonContent.append("KaptonFront/");
  initBasicParameters(kaptonContent);
  sprintf(name, "SVD_KaptonFront");
  return true;
}

Bool_t B2GeomSVDSensorKaptonFront::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    fThickness * 0.5,
                                    fThickness2 * 0.5,
                                    fWidth * 0.5,
                                    fWidth2 * 0.5,
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
  resetBasicParameters();
}
Bool_t B2GeomSVDSensorSmds::init(GearDir& content)
{
  GearDir smdsContent(content);
  smdsContent.append("SMDs/");
  initBasicParameters(smdsContent);
  sprintf(name, "SVD_SMDs");
  return true;
}

Bool_t B2GeomSVDSensorSmds::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeBox(name, tMedium,
                                   0.5 * fThickness,
                                   0.5 * fWidth,
                                   0.5 * fLength);
    tVolume->SetLineColor(kBlue + 4);
  }
  return true;
}









