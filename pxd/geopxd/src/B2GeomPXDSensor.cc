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
  sensorContent.append((format("Sensors/Sensor[@id=\'PXD_Layer_%1%_Ladder_Sensor_%2%\']/SensorParameters/") % iLayer % iSensor).str());

  fSensorWidth = sensorContent.getParamLength("Width");
  fSensorThick = sensorContent.getParamLength("Thickness");

  fLengthActive      = sensorContent.getParamLength("LengthActive");
  fWidthActive       = sensorContent.getParamLength("WidthActive");
  fThickActive       = sensorContent.getParamLength("ThicknessActive");

  fVDistanceActiveFromInnerEdge = sensorContent.getParamLength("VDistanceActiveFromInnerEdge");
  fUDistanceActiveFromInnerEdge = sensorContent.getParamLength("UDistanceActiveFromInnerEdge");

  fVDistanceThinnedFromOuterEdge = sensorContent.getParamLength("VDistanceThinnedFromOuterEdge");
  fVDistanceThinnedFromInnerEdge = sensorContent.getParamLength("VDistanceThinnedFromInnerEdge");
  fUDistanceThinnedFromOuterEdge = sensorContent.getParamLength("UDistanceThinnedFromOuterEdge");
  fUDistanceThinnedFromInnerEdge = sensorContent.getParamLength("UDistanceThinnedFromInnerEdge");

  fAlphaThinned =  sensorContent.getParamLength("AlphaThinned");

  fLengthSilicon = sensorContent.getParamLength("LengthSilicon");
  fWidthSilicon = sensorContent.getParamLength("WidthSilicon");
  fThickSilicon = sensorContent.getParamLength("ThicknessSilicon");

  // Get materials
  string sensorMatName  = sensorContent.getParamString("MaterialSensor");
  medDEPFET = gGeoManager->GetMedium(sensorMatName.c_str());
  medActiveSensor = gGeoManager->GetMedium(sensorMatName.c_str());
  medPXD_Silicon = gGeoManager->GetMedium(sensorMatName.c_str());

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);


  return true;

}
#else
Bool_t B2GeomPXDSensor::init()
{
  // check if this sensor has been already created??
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
  putSilicon();
  // putDEPFET();
  // putSwitchers();
  return true;
}

void B2GeomPXDSensor::putSilicon()
{
// Create the silicon of the depfet sensor


  // create shape for the silicon

  // thinned volume
  Double_t fThickExtrude = 0.0001; // this is necessary, otherwise we leave a border
  Double_t fThickThinned = fThickSilicon - fThickActive;
  Double_t fLength1Thinned = fLengthSilicon - fVDistanceThinnedFromOuterEdge - fVDistanceThinnedFromInnerEdge;
  Double_t fLength2Thinned = fLength1Thinned - 2 * (fThickThinned / TMath::Tan(fAlphaThinned * TMath::DegToRad()));
  Double_t fWidth1Thinned = fWidthSilicon - fUDistanceThinnedFromOuterEdge - fUDistanceThinnedFromInnerEdge;
  Double_t fWidth2Thinned = fWidth1Thinned - 2 * (fThickThinned / TMath::Tan(fAlphaThinned * TMath::DegToRad()));
  char shaThinnedName[200];
  sprintf(shaThinnedName, "%s_ShapeThinned", path.c_str());

  TGeoTrd2 *shaThinned = new TGeoTrd2(shaThinnedName,
                                      0.5 * fLength1Thinned,
                                      0.5 * fLength2Thinned,
                                      0.5 * fWidth1Thinned,
                                      0.5 * fWidth2Thinned,
                                      0.5 * fThickThinned + fThickExtrude);
  // silicon box
  char shaSiliconName[200];
  sprintf(shaSiliconName, "%s_ShapeSilicon", path.c_str());
  TGeoBBox *shaSilicon = new TGeoBBox(shaSiliconName,
                                      0.5 * fThickSilicon,
                                      0.5 * fWidthSilicon,
                                      0.5 * fLengthSilicon);

  // we define the position of the thinned volume inside the silicon box
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  // first we rotate to the thinned volume to the right position
  // (axes are swapped because of Trd2 constructor)
  TGeoRotation rotThinned("rotThinned", 90.0, 90.0, 90.0);
  hmaHelp = rotThinned * hmaHelp;
  // thinned out volume is always on the 'bottom' of the sensor
  TGeoTranslation traThinned(- 0.5 * fThickSilicon + 0.5 * fThickThinned - 0.5*fThickExtrude, 0.0, 0.0);
  hmaHelp = traThinned * hmaHelp;

  // different Sensor number, different position of thinned volume in V
  // the thinned out volume is on the 'right' for iSensor = 0 and on the 'left' for iSensor = 1
  if (iSensor == 0) {
    TGeoTranslation tra(0.0, 0.0, - 0.5 * fLengthSilicon + fVDistanceThinnedFromOuterEdge + 0.5*fLength1Thinned);
    hmaHelp = tra * hmaHelp;
  }
  if (iSensor == 1) {
    TGeoTranslation tra(0.0, 0.0, 0.5 * fLengthSilicon - fVDistanceThinnedFromOuterEdge - 0.5 * fLength1Thinned);
    hmaHelp = tra * hmaHelp;

  }
  // different Layer number different position of thinned volume in U
  if (iLayer == 1) {
    TGeoTranslation tra(0.0, 0.5 * fWidthSilicon - fUDistanceThinnedFromOuterEdge - 0.5 * fWidth1Thinned, 0.0);
    hmaHelp = tra * hmaHelp;
  }
  if (iLayer == 2) {
    TGeoTranslation tra(0.0, -0.5 * fWidthSilicon + fUDistanceThinnedFromOuterEdge + 0.5 * fWidth1Thinned, 0.0);
    hmaHelp = tra * hmaHelp;
  }

  TGeoHMatrix* hmaThinnedVolumePosition = new TGeoHMatrix(hmaHelp);
  char thinnedPosName[200];
  sprintf(thinnedPosName, "hmaThinnedPosition_%i_%i_%i", iLayer, iLadder, iSensor);
  hmaThinnedVolumePosition->SetName(thinnedPosName);
  hmaThinnedVolumePosition->RegisterYourself();

  // we now subtract the thinned volume from the silicon
  char compShapeCommand[200];
  sprintf(compShapeCommand, "%s - %s:%s", shaSiliconName, shaThinnedName, thinnedPosName);
  char compShapeName[200];
  sprintf(compShapeName, "%s_ShapeSiliconMinusThinned", path.c_str());
  TGeoCompositeShape *shaSiliconMinusThinned = new TGeoCompositeShape(compShapeName, compShapeCommand);

  char nameSilicon[200];
  sprintf(nameSilicon, "%s_Silicon", path.c_str());
  volSilicon = new TGeoVolume(nameSilicon, shaSiliconMinusThinned);
  volSilicon->SetLineColor(kBlue + 3);

// Create the active volume to the silicon
  char nameActive[200];
  sprintf(nameActive, "SD_%s_ActiveSensor", path.c_str());
  volActiveSensor = gGeoManager->MakeBox(nameActive, medPXD_Silicon,
                                         0.5 * fThickActive,
                                         0.5 * fWidthActive,
                                         0.5 * fLengthActive);
  volActiveSensor->SetLineColor(kBlue - 10);


  // define position of active volume inside the silicon
  hmaHelp = gGeoIdentity;
  TGeoTranslation traActive(0.5 * fThickSilicon - 0.5 * fThickActive, 0.0 , 0.0);
  hmaHelp = traActive * hmaHelp;
  // different Sensor number, different position of active volume in V
  // the active volume is on the 'right' for iSensor = 0 and on the 'left' for iSensor = 1
  if (iSensor == 0) {
    TGeoTranslation tra(0.0, 0.0, 0.5 * fLengthSilicon - fVDistanceActiveFromInnerEdge - 0.5 * fLengthActive);

    hmaHelp = tra * hmaHelp;
  }
  if (iSensor == 1) {
    TGeoTranslation tra(0.0, 0.0, - 0.5 * fLengthSilicon + fVDistanceActiveFromInnerEdge + 0.5 * fLengthActive);
    hmaHelp = tra * hmaHelp;
  }

  // different Layer number different position of thinned volume in U
  if (iLayer == 1) {
    TGeoTranslation tra(0.0, - 0.5 * fWidthSilicon + fUDistanceActiveFromInnerEdge + 0.5 * fWidthActive, 0.0);
    hmaHelp = tra * hmaHelp;
  }
  if (iLayer == 2) {
    TGeoTranslation tra(0.0, 0.5 * fWidthSilicon - fUDistanceActiveFromInnerEdge - 0.5 * fWidthActive, 0.0);
    hmaHelp = tra * hmaHelp;
  }


  // add active volume to the silicon
  volSilicon->AddNode(volActiveSensor, 1, new TGeoHMatrix(hmaHelp));

  // add silicon volume to sensor volume
  volPXDSensor->AddNode(volSilicon, 1, new TGeoTranslation(0.0, 0.0, 0.0));
}



void B2GeomPXDSensor::putSwitchers()
{
  // TGeoVolume *box = gGeoManager->MakeBox("test", medAir, 0.25, 0.25, 0.25);
  // volPXDSensor->AddNode(box, 1, new TGeoTranslation(-.125-0.5*fSensorThick,0,0));
}

Double_t B2GeomPXDSensor::getLengthSilicon()
{
  return fLengthSilicon;
}

TGeoHMatrix B2GeomPXDSensor::getSurfaceCenterPosition()
{
  TGeoHMatrix hmaHelp;
  TGeoTranslation tra(0.5*fThickSilicon, 0.0, 0.0);
  hmaHelp = gGeoIdentity;
  hmaHelp = tra * hmaHelp;
  return hmaHelp;
}

