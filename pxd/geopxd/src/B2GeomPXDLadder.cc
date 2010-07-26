/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDLadder.h>

using namespace Belle2;
using namespace boost;
using namespace std;

B2GeomPXDLadder::B2GeomPXDLadder()
{
  B2GeomPXDLadder(-1, -1);
}

B2GeomPXDLadder::B2GeomPXDLadder(Int_t iLay, Int_t iLad)
{
  iLayer = iLay;
  iLadder = iLad;
  path = (format("PXD_Layer_%1%_Ladder_%2%") % iLayer % iLadder).str();
}

B2GeomPXDLadder::~B2GeomPXDLadder()
{
}

Bool_t B2GeomPXDLadder::init(GearDir& content)
{
  ladderContent = GearDir(content);
  ladderContent.append("Ladder/");
  fGapLength = int(ladderContent.getParamNumValue("Gap"));
  nSensors = int(ladderContent.getParamNumValue("NumberOfSensors"));
  b2gPXDSensors.resize(nSensors);
  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  medGlue = new TGeoMedium("medGlue", 1, matVacuum);
  return true;
}

Bool_t B2GeomPXDLadder::make()
{
  volPXDLadder = new TGeoVolumeAssembly(path.c_str());
  putSensors();
  putGlue();
  return true;
}

void B2GeomPXDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {
    b2gPXDSensors[iSensor] = new B2GeomPXDSensor(iLayer, iLadder, iSensor);
    b2gPXDSensors[iSensor]->init(ladderContent);
    b2gPXDSensors[iSensor]->make();
    volPXDLadder->AddNode(b2gPXDSensors[iSensor]->getVol(), 1, new TGeoTranslation(0.0,
                          0.0,
                          -0.5 * b2gPXDSensors[iSensor]->getLength() - 0.5 * fGapLength + iSensor *(b2gPXDSensors[iSensor]->getLength() + fGapLength)));
  }
}

void B2GeomPXDLadder::putGlue()
{

}

