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
  b2gSVDSensors.resize(nSensors);
  sensorTypes.resize(nSensors);
  fThetas.resize(nSensors);
  fSensorVPositions.resize(nSensors);
  fSensorWPositions.resize(nSensors);

  for (int iSensor = 0; iSensor < nSensors; iSensor++) {
    GearDir sensorsContent(ladderContent);
    sensorsContent.append("Sensors/");
    sensorsContent.append((format("Sensor[@id=\'SVD_Layer_%1%_Ladder_Sensor_%2%\']/") % iLayer % iSensor).str());
    sensorTypes[iSensor] = int(sensorsContent.getParamNumValue("SensorType"));
    sensorsContent.append("Position/");
    fThetas[iSensor] = double(sensorsContent.getParamLength("Theta"));
    fSensorVPositions[iSensor] = double(sensorsContent.getParamLength("V"));
    fSensorWPositions[iSensor] =  double(sensorsContent.getParamLength("W"));
  }

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);


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

}
#endif

Bool_t B2GeomSVDLadder::make()
{
  printf("make B2GeomSVDLadder %i\n", iLadder);
  volSVDLadder = new TGeoVolumeAssembly(path.c_str());
  putSensors();
  return true;
}

void B2GeomSVDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {

    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor, sensorTypes[iSensor]);
#ifdef B2GEOM_BASF2
    b2gSVDSensors[iSensor]->init(ladderContent);
#else
    b2gSVDSensors[iSensor]->init();
#endif
    b2gSVDSensors[iSensor]->make();

    // incline the sensor to z axis
    TGeoRotation rotTheta("name", -90.0, fThetas[iSensor], 90.0);
    //move the sensor to its position
    TGeoTranslation tra(fSensorWPositions[iSensor], 0.0, fSensorVPositions[iSensor]);

    TGeoHMatrix hmaHelp;
    hmaHelp = gGeoIdentity;
    hmaHelp = rotTheta * hmaHelp;
    hmaHelp = tra * hmaHelp;
    TGeoHMatrix* hmaSensorPosition = new TGeoHMatrix(hmaHelp);
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1, hmaSensorPosition);
  }
}


Double_t B2GeomSVDLadder::getLengthBarrel()
{
  Double_t fLengthBarrel = 10.0;
  return fLengthBarrel;
}

