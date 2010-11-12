/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDSensor.h>
using namespace boost;
using namespace Belle2;

using namespace std;

B2GeomPXDSensor::B2GeomPXDSensor()
{
  B2GeomPXDSensor(-1, -1, -1);
}

B2GeomPXDSensor::B2GeomPXDSensor(Int_t iLay, Int_t iLad, Int_t iSen)
{
  // define ID of this sensor
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  resetBasicParameters();
  sprintf(name, "PXD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  volSilicon = NULL;
  volSwitchers1 = NULL;
  volSwitchers2 = NULL;
}

B2GeomPXDSensor::~B2GeomPXDSensor()
{

}

Bool_t B2GeomPXDSensor::init(GearDir& content)
{
  // printf("B2GeomPXDSensor::init start\n");
  GearDir sensorContent(content);
  sensorContent.append((format("Sensors/Sensor[@id=\'PXD_Layer_%1%_Ladder_Sensor_%2%\']/") % iLayer % iSensor).str());
  initBasicParameters(sensorContent);


  if (sensorContent.isParamAvailable("Silicon")) {
    volSilicon = new B2GeomPXDSensorSilicon(iLayer, iLadder, iSensor);
    if (!volSilicon->init(sensorContent)) {
      printf("B2ERROR! Parameter reading for SVD silicon failed!\n");
      return false;
    }
  } else {
    printf("B2ERROR! Definition of Silicon missing in XML file!\n");
    return false;
  }

  if (sensorContent.isParamAvailable("Switchers1")) {
    volSwitchers1 = new B2GeomPXDSensorSwitchers1(iLayer);
    if (!volSwitchers1->init(sensorContent, "Switchers1/")) return false;
  }

  if (sensorContent.isParamAvailable("Switchers2")) {
    volSwitchers2 = new B2GeomPXDSensorSwitchers2(iLayer);
    if (!volSwitchers2->init(sensorContent, "Switchers2/")) return false;
  }

  // init offsets from XML file
  sensorContent.setDirPath((format("//Offsets/Sensor[@id=\'PXD_Offset_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
  initOffsets(sensorContent);
  // printf("B2GeomPXDSensor::init stop\n");
  return true;
}

Bool_t B2GeomPXDSensor::make()
{
  //printf("B2GeomPXDSensor::make start\n");
  tVolume = new TGeoVolumeAssembly(name);

  // add silicon volume to sensor volume
  if (!volSilicon->make()) {
    printf("B2ERROR! Cannot create silicon of PXD sensor!\n");
    return false;
  }
  tVolume->AddNode(volSilicon->getVol(), 1, volSilicon->getPosition());

  if (volSwitchers1 != NULL) {
    volSwitchers1->make();
    tVolume->AddNode(volSwitchers1->getVol(), 1, volSwitchers1->getPosition());
    volSwitchers1->getVol()->SetLineColor(kGray + 3);
  }

  if (volSwitchers2 != NULL) {
    volSwitchers2->make();
    tVolume->AddNode(volSwitchers2->getVol(), 1, volSwitchers2->getPosition());
    volSwitchers2->getVol()->SetLineColor(kGray + 3);
  }

  //printf("B2GeomPXDSensor::make stop\n");
  return true;
}

// ------------------------------------------------------------------------------------------------
// Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

B2GeomPXDSensorSilicon::B2GeomPXDSensorSilicon(Int_t iLay, Int_t iLad, Int_t iSen)
{
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  resetBasicParameters();
  sprintf(name, "PXD_Layer_%i_Ladder_%i_Sensor_%i_Silicon", iLayer, iLadder, iSensor);
  volActive = NULL;
  volThinned = NULL;
}

Bool_t B2GeomPXDSensorSilicon::init(GearDir& content)
{
  GearDir siliconContent(content);
  siliconContent.append("Silicon/");
  initBasicParameters(siliconContent);
  if (siliconContent.isParamAvailable("Thinned")) {
    volThinned = new B2GeomPXDSensorThinned(iLayer);
    if (!volThinned->init(siliconContent)) {
      printf("B2ERROR! Parameter reading for PXD silicon thinned failed!\n");
      return false;
    }
  } else {
    printf("B2ERROR! Definition of PXD Silicon Thinned missing in XML file!\n");
    return false;
  }
  if (siliconContent.isParamAvailable("Active")) {
    volActive = new B2GeomPXDSensorActive(iLayer, iLadder, iSensor);
    if (!volActive->init(siliconContent)) {
      printf("B2ERROR! Parameter reading for SVD silicon active failed!\n");
      return false;
    }
  } else {
    printf("B2ERROR! Definition of PXD Silicon Active missing in XML file!\n");
    return false;
  }
  return true;
}

Bool_t B2GeomPXDSensorSilicon::make()
{

  //printf("B2GeomPXDSensorSilicon::make start\n");

  /*
  char compShapeName[200];
  sprintf(compShapeName, "PXD_Layer_%i_ShapeSiliconMinusThinned", iLayer);
  TGeoCompositeShape* shaSiliconMinusThinned = (TGeoCompositeShape*) gROOT->FindObjectAny(compShapeName);

  if (!shaSiliconMinusThinned) {

  char shaSiliconName[200];
  sprintf(shaSiliconName, "%s_ShapeSilicon", path.c_str());
  TGeoBBox *shaSilicon = new TGeoBBox(shaSiliconName,
                                      0.5 * fThickness,
                                      0.5 * fWidth,
                                      0.5 * fLength);
  char shaThinnedName[200];
  sprintf(shaThinnedName, "%s_ShapeThinned", path.c_str());
  if (!volThinned->make()) return false;
  TGeoShape* shaThinned = volThinned->getVol()->GetShape();
  shaThinned->SetName(shaThinnedName);

  char thinnedPosName[200];
  sprintf(thinnedPosName, "hmaThinnedPosition_Layer_%i_Sensor_%i", iLayer, iSensor);
  TGeoHMatrix* hmaThinnedVolumePosition = (TGeoHMatrix*) gROOT->FindObjectAny(thinnedPosName);
  if (!hmaThinnedVolumePosition) {
  hmaThinnedVolumePosition = volThinned->getPosition();
  hmaThinnedVolumePosition->SetName(thinnedPosName);
  hmaThinnedVolumePosition->RegisterYourself();
  }

  // we subtract now the thinned volume from the silicon
  char compShapeCommand[200];
  sprintf(compShapeCommand, "%s - %s:%s", shaSiliconName, shaThinnedName, thinnedPosName);
  shaSiliconMinusThinned = new TGeoCompositeShape(compShapeName, compShapeCommand);
  shaSiliconMinusThinned->InspectShape();
  }

   tVolume = new TGeoVolume(path.c_str(), shaSiliconMinusThinned);
  */

  tVolume = gGeoManager->MakeBox(name, tMedium,
                                 0.5 * fThickness,
                                 0.5 * fWidth,
                                 0.5 * fLength
                                );
  tVolume->SetLineColor(kBlue + 3);

  // add thinned out volume with air
  if (!volThinned->make()) {
    printf("B2ERROR! Cannot create thinned volume of PXD sensor!");
    return false;
  }
  tVolume->AddNode(volThinned->getVol(), 1, volThinned->getPosition());

  // add active volume to the silicon
  if (!volActive->make()) {
    printf("B2ERROR! Cannot create active volume of PXD sensor!");
    return false;
  }
  tVolume->AddNode(volActive->getVol(), 1, volActive->getPosition());
  //printf("B2GeomPXDSensorSilicon::make stop\n");
  return true;
}

// ------------------------------------------------------------------------------------------------
// Active Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

B2GeomPXDSensorActive::B2GeomPXDSensorActive(Int_t iLay, Int_t iLad, Int_t iSen)
{
  iLayer = iLay;
  iLadder = iLad;
  iSensor = iSen;
  sprintf(name, "SD_PXD_Layer_%i_Ladder_%i_Sensor_%i_Silicon_Active", iLayer, iLadder, iSensor);
  resetBasicParameters();
}

Bool_t B2GeomPXDSensorActive::init(GearDir& content)
{
  GearDir activeContent(content);
  activeContent.append("Active/");
  initBasicParameters(activeContent);

  // check if the sensor or the ladder is mirrored!
  // then the active sensor has also to be mirrored in order to keep the right coordinates
  while (true) {
    if (!goToParentNode(activeContent)) break;
    if (content.isParamAvailable("ReflectX")) isReflectX = !isReflectX;
    if (content.isParamAvailable("ReflectY")) isReflectY = !isReflectY;
    if (content.isParamAvailable("ReflectZ")) isReflectZ = !isReflectZ;
  }
  return true;
}

Bool_t B2GeomPXDSensorActive::make()
{
  // define the active volume of the SVD sensor
  tVolume = gGeoManager->MakeBox(name, tMedium,
                                 fThickness * 0.5,
                                 fWidth * 0.5,
                                 fLength * 0.5);
  tVolume->SetLineColor(kBlue - 10);
  return true;
}

// ------------------------------------------------------------------------------------------------
// Thinned part of the sensor
// ------------------------------------------------------------------------------------------------
B2GeomPXDSensorThinned::B2GeomPXDSensorThinned(Int_t iLay)
{
  iLayer = iLay;
  sprintf(name, "PXD_Layer_%i_Silicon_Thinned", iLayer);
  resetBasicParameters();
}

Bool_t B2GeomPXDSensorThinned::init(GearDir& content)
{
  GearDir thinnedContent(content);
  thinnedContent.append("Thinned/");
  initBasicParameters(thinnedContent);
  return true;
}


Bool_t B2GeomPXDSensorThinned::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                    0.5 * fThickness,
                                    0.5 * fThickness2,
                                    0.5 * fWidth,
                                    0.5 * fWidth2,
                                    0.5 * fLength);
    tVolume->SetLineColor(kWhite);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// Electronics1 of the PXD module
// ------------------------------------------------------------------------------------------------

B2GeomPXDSensorSwitchers1::B2GeomPXDSensorSwitchers1(Int_t iLay)
{
  iLayer = iLay;
  sprintf(name, "PXD_Layer_%i_Switchers1", iLayer);
  resetBasicParameters();
}

// ------------------------------------------------------------------------------------------------
// Electronics2 of the PXD module
// ------------------------------------------------------------------------------------------------

B2GeomPXDSensorSwitchers2::B2GeomPXDSensorSwitchers2(Int_t iLay)
{
  iLayer = iLay;
  sprintf(name, "PXD_Layer_%i_Switchers2", iLayer);
  resetBasicParameters();
}



