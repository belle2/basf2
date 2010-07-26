/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDLadder.h>

using namespace Belle2;
using namespace boost;
using namespace std;

B2GeomSVDLadder::B2GeomSVDLadder()
{
  B2GeomSVDLadder(-1, -1);
}

B2GeomSVDLadder::B2GeomSVDLadder(Int_t iLay, Int_t iLad)
{
  iLayer = iLay;
  iLadder = iLad;
  path = (format("SVD_Layer_%1%_Ladder_%2%") % iLayer % iLadder).str();
}

B2GeomSVDLadder::~B2GeomSVDLadder()
{
}

Bool_t B2GeomSVDLadder::init(GearDir& content)
{
  ladderContent = GearDir(content);
  ladderContent.append("Ladder/");
  fGapLength = int(ladderContent.getParamNumValue("Gap"));
  nSensors = int(ladderContent.getParamNumValue("NumberOfSensors"));
  nSlantedSensors = int(ladderContent.getParamNumValue("NumberOfSlantedSensors"));
  b2gSVDSensors.resize(nSensors + nSlantedSensors);
  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  medGlue = new TGeoMedium("medGlue", 1, matVacuum);
  return true;
}

Bool_t B2GeomSVDLadder::make()
{
  volSVDLadder = new TGeoVolumeAssembly(path.c_str());
  putSensors();
  putGlue();
  return true;
}

void B2GeomSVDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {

    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor, false);
    b2gSVDSensors[iSensor]->init(ladderContent);
    b2gSVDSensors[iSensor]->make();
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1,
                          new TGeoTranslation(0.0,
                                              0.0,
                                              -0.5 * b2gSVDSensors[iSensor]->getLength() + iSensor *(b2gSVDSensors[iSensor]->getLength() + fGapLength)));
  }

  // add slanted sensor to this layer if there is any
  for (int iSensor = nSensors; iSensor < nSensors + nSlantedSensors; iSensor++) {
    b2gSVDSensors[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor, true);
    b2gSVDSensors[iSensor]->init(ladderContent);
    b2gSVDSensors[iSensor]->make();
    volSVDLadder->AddNode(b2gSVDSensors[iSensor]->getVol(), 1,
                          new TGeoTranslation(0.0,
                                              0.0,
                                              -0.5 * b2gSVDSensors[0]->getLength() + (nSensors - 1) *(b2gSVDSensors[0]->getLength() + fGapLength)));
  }
}

void B2GeomSVDLadder::putGlue()
{

}

