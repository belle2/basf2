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
  B2GeomVolume();
  char text[200];
  sprintf(text, "PXD_Layer_%i_Ladder_%i_Sensor_%i", iLayer, iLadder, iSensor);
  path = string(text);
  volSilicon = NULL;
}

B2GeomPXDSensor::~B2GeomPXDSensor()
{

}

Bool_t B2GeomPXDSensor::init(GearDir& content)
{
  printf("B2GeomPXDSensor::init start\n");
  GearDir sensorContent(content);
  sensorContent.append((format("Sensors/Sensor[@id=\'PXD_Layer_%1%_Ladder_Sensor_%2%\']/") % iLayer % iSensor).str());
  initBasicParameters(sensorContent);


  if (sensorContent.isParamAvailable("Silicon")) {
    volSilicon = new B2GeomPXDSensorSilicon(iLayer, iLadder, iSensor);
    if (!volSilicon->init(sensorContent)) {
      printf("ERROR! Parameter reading for SVD silicon failed!\n");
      return false;
    }
  } else {
    printf("ERROR! Definition of Silicon missing in XML file!\n");
    return false;
  }
  printf("B2GeomPXDSensor::init stop\n");
  return true;
}

Bool_t B2GeomPXDSensor::make()
{
  printf("B2GeomPXDSensor::make start\n");
  tVolume = gGeoManager->MakeBox(path.c_str(), tMedium,
                                 0.5 * fThickness,
                                 0.5 * fWidth,
                                 0.5 * fLength);
  // add silicon volume to sensor volume
  if (!volSilicon->make()) {
    printf("ERROR! Cannot create silicon of PXD sensor!\n");
    return false;
  }
  tVolume->AddNode(volSilicon->getVol(), 1, volSilicon->getPosition());
  printf("B2GeomPXDSensor::make stop\n");
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
  B2GeomVolume();
  char nameSilicon[200];
  sprintf(nameSilicon, "PXD_Layer_%i_Ladder_%i_Sensor_%i_Silicon", iLayer, iLadder, iSensor);
  path = string(nameSilicon);
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
      printf("ERROR! Parameter reading for PXD silicon thinned failed!\n");
      return false;
    }
  } else {
    printf("ERROR! Definition of PXD Silicon Thinned missing in XML file!\n");
    return false;
  }
  if (siliconContent.isParamAvailable("Active")) {
    volActive = new B2GeomPXDSensorActive(iLayer, iLadder, iSensor);
    if (!volActive->init(siliconContent)) {
      printf("ERROR! Parameter reading for SVD silicon active failed!\n");
      return false;
    }
  } else {
    printf("ERROR! Definition of PXD Silicon Active missing in XML file!\n");
    return false;
  }
  return true;
}

Bool_t B2GeomPXDSensorSilicon::make()
{

  printf("B2GeomPXDSensorSilicon::make start\n");

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

  tVolume = gGeoManager->MakeBox(path.c_str(), tMedium,
                                 0.5 * fThickness,
                                 0.5 * fWidth,
                                 0.5 * fLength
                                );
  tVolume->SetLineColor(kBlue + 3);

  // add thinned out volume with air
  if (!volThinned->make()) {
    printf("ERROR! Cannot create thinned volume of PXD sensor!");
    return false;
  }
  tVolume->AddNode(volThinned->getVol(), 1, volThinned->getPosition());
  volThinned->getPosition()->Print();

  // add active volume to the silicon
  if (!volActive->make()) {
    printf("ERROR! Cannot create active volume of PXD sensor!");
    return false;
  }
  tVolume->AddNode(volActive->getVol(), 1, volActive->getPosition());
  printf("B2GeomPXDSensorSilicon::make stop\n");
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
  char text[200];
  sprintf(text, "SD_PXD_Layer_%i_Ladder_%i_Sensor_%i_Silicon_Active", iLayer, iLadder, iSensor);
  path = string(text);
  B2GeomVolume();
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
  char nameActive[200];
  // define the active volume of the SVD sensor
  sprintf(nameActive, "%s", path.c_str());
  tVolume = gGeoManager->MakeBox(nameActive, tMedium,
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
  char shaThinnedName[200];
  sprintf(shaThinnedName, "PXD_Layer_%i_Silicon_Thinned", iLayer);
  path = string(shaThinnedName);
  B2GeomVolume();
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
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(path.c_str());
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(path.c_str(), tMedium,
                                    0.5 * fThickness,
                                    0.5 * fThickness2,
                                    0.5 * fWidth,
                                    0.5 * fWidth2,
                                    0.5 * fLength);
  }
  return true;
}
