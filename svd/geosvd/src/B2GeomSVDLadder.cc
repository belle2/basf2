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
#include <svd/geosvd/B2GeomSVDLadder.h>
using namespace boost;
using namespace Belle2;
#else
#include "B2GeomSVDLadder.h"
#endif

using namespace std;

B2GeomSVDLadder::B2GeomSVDLadder()
{
  B2GeomSVDLadder(-1, -1);
}

B2GeomSVDLadder::B2GeomSVDLadder(Int_t iLay, Int_t iLad)
{
  iLayer = iLay;
  iLadder = iLad;
  char text[200];
  sprintf(text, "SVD_Layer_%i_Ladder_%i" , iLayer, iLadder);
  path = string(text);
}

B2GeomSVDLadder::~B2GeomSVDLadder()
{
}

#ifdef B2GEOM_BASF2
Bool_t B2GeomSVDLadder::init(GearDir& content)
{
  ladderContent = GearDir(content);
  ladderContent.append("Ladders/");

  // get parameters for the sensors
  nSensors = int(ladderContent.getParamNumValue("NumberOfSensors"));
  isRibs = bool(ladderContent.getParamNumValue("isRibs"));

  if (iLayer > 3 && isRibs) {
    fThicknessRibs = double(ladderContent.getParamLength("ThicknessRibs"));
    fRibUPosition0 = double(ladderContent.getParamLength("RibUPosition0"));
    fRibUPosition1 = double(ladderContent.getParamLength("RibUPosition1"));
    fWidthRibs = double(ladderContent.getParamLength("WidthRibs"));
    fWidthCarbon = double(ladderContent.getParamLength("WidthCarbon"));
    fLengthRibsBarrel = double(ladderContent.getParamLength("LengthRibsBarrel"));
    fLength1RibsSlanted = double(ladderContent.getParamLength("Length1RibsSlanted"));
    fLength2RibsSlanted = double(ladderContent.getParamLength("Length2RibsSlanted"));
    fRibsDistanceFromSensor = double(ladderContent.getParamLength("RibsDistanceFromSensor"));
  }

  b2gSVDSensors.resize(nSensors);
  fThetas.resize(nSensors);
  fSensorVPositions.resize(nSensors);
  fSensorWPositions.resize(nSensors);

  for (int iSensor = 0; iSensor < nSensors; iSensor++) {
    GearDir sensorsContent(ladderContent);
    sensorsContent.append("Sensors/");
    sensorsContent.append((format("Sensor[@id=\'SVD_Layer_%1%_Ladder_Sensor_%2%\']/") % iLayer % iSensor).str());
    sensorsContent.append("Position/");
    fThetas[iSensor] = double(sensorsContent.getParamLength("Theta"));
    fSensorVPositions[iSensor] = double(sensorsContent.getParamLength("V"));
    fSensorWPositions[iSensor] =  double(sensorsContent.getParamLength("W"));
  }

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  medFoam = new TGeoMedium("medFoam", 1, matVacuum);
  medCarbon = new TGeoMedium("medCarbon", 1, matVacuum);


  b2gSensorOffsets.resize(nSensors);
  if (initOffsets()) {
    return true;
  } else {
    return false;
  }
}

Bool_t B2GeomSVDLadder::initOffsets()
{
  // create GearDir object for parameter access
  GearDir offsetsDir;

  for (Int_t iSensor = 0; iSensor < nSensors; iSensor++) {
    // go in XML file to section where the parameters for the corresponding sensor are stored
    offsetsDir.setDirPath((format("//SensorOffset[@id=\'SVD_Offsets_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
    // init parameters
    b2gSensorOffsets[iSensor] = new B2GeomOffset();
    b2gSensorOffsets[iSensor]->init(offsetsDir);
  }
  return true;
}
#else
Bool_t B2GeomSVDLadder::init()
{
// get parameters for the sensors
  if (iLayer == 3) {
    nSensors = 2;

    b2gSVDSensors.resize(nSensors);
    fThetas.resize(nSensors);
    fSensorVPositions.resize(nSensors);
    fSensorWPositions.resize(nSensors);

    fThetas[0] = 0.0;
    fThetas[1] = 0.0;
    fSensorVPositions[0] = -3.42;
    fSensorWPositions[0] = 0.0;
    fSensorVPositions[1] = 8.8;
    fSensorWPositions[1] = 0.0;
  }

}

TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
medAir = new TGeoMedium("medAir", 1, matVacuum);
medFoam = new TGeoMedium("medFoam", 1, matVacuum);
medCarbon = new TGeoMedium("medCarbon", 1, matVacuum);


b2gSensorOffsets.resize(nSensors);
if (initOffsets())
{
  return true;
} else
{
  return false;
}
}
#endif

Bool_t B2GeomSVDLadder::make()
{
  volSVDLadder = new TGeoVolumeAssembly(path.c_str());
  putSensors();
  if (iLayer > 3 && isRibs) {
    putRibsBarrel();
    // putRibsSlanted();
  }
  return true;
}

void B2GeomSVDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {

    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor);

#ifdef B2GEOM_BASF2
    b2gSVDSensors[iSensor]->init(ladderContent);
#else
    b2gSVDSensors[iSensor]->init();
#endif

    b2gSVDSensors[iSensor]->make();

    // incline the sensor to z axis
    TGeoRotation rotTheta("name", -90.0, fThetas[iSensor], 90.0);
    // calculate sensor position
    TGeoTranslation tra(fSensorWPositions[iSensor], 0.0, fSensorVPositions[iSensor]);
    TGeoHMatrix hmaHelp;
    hmaHelp = gGeoIdentity;
    // first go to the center of the sensor surface
    hmaHelp = b2gSVDSensors[iSensor]->getSurfaceCenterPosition().Inverse() * hmaHelp;
    hmaHelp = rotTheta * hmaHelp;
    hmaHelp = tra * hmaHelp;
    TGeoHMatrix* hmaSensorPosition = new TGeoHMatrix(hmaHelp);
    // place sensor in the ladder
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1, hmaSensorPosition);
  }
}

void B2GeomSVDLadder::putRibsBarrel()
{
  char nameRibsBarrel[200];
  sprintf(nameRibsBarrel, "SVD_Layer_%i_Ladder_Ribs_Barrel", iLayer);

  volRibsBarrel = (TGeoVolume*) gROOT->FindObjectAny(nameRibsBarrel);
  if (!volRibsBarrel) {

    volRibsBarrel = gGeoManager->MakeBox(nameRibsBarrel, medAir,
                                         0.5 * fThicknessRibs,
                                         0.5 * (fRibUPosition0 - fRibUPosition1) + 0.5 * fWidthRibs,
                                         0.5 * fLengthRibsBarrel);
    volRibsBarrel->SetInvisible();

    char nameRibBarrel[200];
    sprintf(nameRibBarrel, "SVD_Layer_%i_Ladder_Ribs_Barrel_Rib", iLayer);
    volRibBarrel = gGeoManager->MakeBox(nameRibBarrel, medFoam,
                                        0.5 * fThicknessRibs,
                                        0.5 * fWidthRibs,
                                        0.5 * fLengthRibsBarrel
                                       );
    volRibBarrel->SetLineColor(kYellow - 9);

    char nameCarbonBarrel[200];
    sprintf(nameCarbonBarrel, "SVD_Layer_%i_Ladder_Ribs_Barrel_Rib_Carbon", iLayer);
    volCarbonBarrel = gGeoManager->MakeBox(nameCarbonBarrel, medCarbon,
                                           0.5 * fThicknessRibs,
                                           0.5 * fWidthCarbon, // later we make this shorter -> offsets!
                                           0.5 * fLengthRibsBarrel);
    volCarbonBarrel->SetLineColor(kBlack);
    volRibBarrel->AddNode(volCarbonBarrel, 1, new TGeoTranslation(0.0, - 0.5 * fWidthRibs + 0.5 * fWidthCarbon, 0.0));
    volRibBarrel->AddNode(volCarbonBarrel, 2, new TGeoTranslation(0.0, + 0.5 * fWidthRibs - 0.5 * fWidthCarbon, 0.0));
    volRibsBarrel->AddNode(volRibBarrel, 1, new TGeoTranslation(0.0, fRibUPosition0, 0.0));
    volRibsBarrel->AddNode(volRibBarrel, 2, new TGeoTranslation(0.0, fRibUPosition1, 0.0));

  }
  volSVDLadder->AddNode(volRibsBarrel, 1, new TGeoTranslation(-fRibsDistanceFromSensor,
                                                              0.,
                                                              fSensorVPositions[nSensors-2] + 0.5*b2gSVDSensors[0]->getLength() - 0.5 * fLengthRibsBarrel
                                                             ));
}

void B2GeomSVDLadder::putRibsSlanted()
{
  char nameRibsSlanted[200];
  sprintf(nameRibsSlanted, "SVD_Ribs_Slanted");

  volRibsSlanted = (TGeoVolume*) gROOT->FindObjectAny(nameRibsSlanted);
  if (!volRibsSlanted) {

    volRibsSlanted = gGeoManager->MakeTrd1(nameRibsSlanted, medAir,
                                           0.5 * fLength1RibsSlanted,
                                           0.5 * fLength2RibsSlanted,
                                           0.5 * (fRibUPosition0 - fRibUPosition1) + 0.5 * fWidthRibs,
                                           0.5 * fThicknessRibs
                                          );



    char nameRibSlanted[200];
    sprintf(nameRibSlanted, "SVD_Rib_Slanted");
    volRibSlanted = gGeoManager->MakeTrd1(nameRibSlanted, medFoam,
                                          0.5 * fLength1RibsSlanted,
                                          0.5 * fLength2RibsSlanted,
                                          0.5 * fWidthRibs,
                                          0.5 * fThicknessRibs
                                         );

    char nameCarbonSlanted[200];
    sprintf(nameCarbonSlanted, "SVD_Rib_Barrel_Carbon");
    volCarbonSlanted = gGeoManager->MakeTrd1(nameCarbonSlanted, medCarbon,
                                             0.5 * fLength1RibsSlanted,
                                             0.5 * fLength2RibsSlanted,
                                             0.5 * fWidthCarbon, // later we make this shorter -> offsets!
                                             0.5 * fThicknessRibs);

    volRibSlanted->AddNode(volCarbonSlanted, 1, new TGeoTranslation(0.0, - 0.5 * fWidthRibs + 0.5 * fWidthCarbon, 0.0));
    volRibSlanted->AddNode(volCarbonSlanted, 2, new TGeoTranslation(0.0, + 0.5 * fWidthRibs - 0.5 * fWidthCarbon, 0.0));
    volRibsSlanted->AddNode(volRibSlanted, 1, new TGeoTranslation(0.0, fRibUPosition0, 0.0));
    volRibsSlanted->AddNode(volRibSlanted, 2, new TGeoTranslation(0.0, fRibUPosition1, 0.0));

  }

  // rotate the Trd1 to the right coordinates
  TGeoRotation rot1("rot1", -90.0, 90.0, 90.0);
  // take into account distance between sensor and rib
  TGeoTranslation tra2(-fRibsDistanceFromSensor, 0.0, 0.0);
  // rotate about theta angle
  TGeoRotation rot3("rot3", -90.0, fThetas[nSensors-1], 90.0);
  // position at slanted sensor position
  TGeoTranslation tra4(fSensorWPositions[nSensors-1], 0.0, fSensorVPositions[nSensors-1]);

  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = rot1 * hmaHelp;
  hmaHelp = tra2 * hmaHelp;
  hmaHelp = rot3 * hmaHelp;
  hmaHelp = tra4 * hmaHelp;

  TGeoHMatrix* hmaRibsSlantedPosition = new TGeoHMatrix(hmaHelp);

  volSVDLadder->AddNode(volRibsSlanted, 1, hmaRibsSlantedPosition);
}

TGeoHMatrix B2GeomSVDLadder::getOrigin()
{
  TGeoTranslation tra(0.0, 0.0, 0.0);
  TGeoHMatrix hmaHelp(tra);
  return hmaHelp;
}


