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
  ladderContent.append("Ladder/");
  nSensors = int(ladderContent.getParamNumValue("NumberOfSensors"));
  nSlantedSensors = int(ladderContent.getParamNumValue("NumberOfSlantedSensors"));
  b2gSVDSensors.resize(nSensors + nSlantedSensors);
  fTheta = double(ladderContent.getParamNumValue("Theta"));
  fGapLength = double(ladderContent.getParamNumValue("Gap"));
  fGapLength = 0;
  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  medGlue = new TGeoMedium("medGlue", 1, matVacuum);
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
  putGlue();
  return true;
}

void B2GeomSVDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {

    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor, false);
#ifdef B2GEOM_BASF2
    b2gSVDSensors[iSensor]->init(ladderContent);
#else
    b2gSVDSensors[iSensor]->init();
#endif
    b2gSVDSensors[iSensor]->make();
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1,
                          new TGeoTranslation(0.0,
                                              0.0,
                                              -0.5 * b2gSVDSensors[iSensor]->getLength() + iSensor *(b2gSVDSensors[iSensor]->getLength() + fGapLength)));
  }

  // add slanted sensor to this layer if there is any
  for (int iSensor = nSensors; iSensor < nSensors + nSlantedSensors; iSensor++) {
    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor, true);
#ifdef B2GEOM_BASF2
    b2gSVDSensors[iSensor]->init(ladderContent);
#else
    b2gSVDSensors[iSensor]->init();
#endif
    b2gSVDSensors[iSensor]->make();
    //move the outer edge of the sensor to the origin
    TGeoTranslation tra1(0.0, 0.0, 0.5 * b2gSVDSensors[iSensor]->getLength());
    // incline the sensor to z axis
    TGeoRotation rotTheta("name", -90.0, fTheta, 90.0);
    // go to outer end of the laddder
    TGeoTranslation tra2(0.0, 0.0, -1.0 * b2gSVDSensors[0]->getLength() + nSensors *(b2gSVDSensors[0]->getLength()) + (nSensors - 1) * fGapLength);

    TGeoHMatrix hmaHelp;
    hmaHelp = gGeoIdentity;
    hmaHelp = tra1 * hmaHelp;
    hmaHelp = rotTheta * hmaHelp;
    hmaHelp = tra2 * hmaHelp;
    TGeoHMatrix* hmaSlantedSensorPosition = new TGeoHMatrix(hmaHelp);
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1, hmaSlantedSensorPosition);
  }
}

void B2GeomSVDLadder::putGlue()
{

}

