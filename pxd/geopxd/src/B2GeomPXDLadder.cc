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
  sprintf(name, "PXD_Layer_%i_Ladder_%i", iLayer, iLadder);
  resetBasicParameters();
}

B2GeomPXDLadder::~B2GeomPXDLadder()
{
}

Bool_t B2GeomPXDLadder::init(GearDir& content)
{
  //printf("B2GeomPXDLadder::init start (Lay:%i, Lad:%i)\n", iLayer, iLadder);
  GearDir ladderContent(content);
  ladderContent.append("Ladder/");

  initBasicParameters(ladderContent);

  // get number of sensors
  GearDir sensorsContent(ladderContent);
  sensorsContent.append("/Sensors/Sensor");
  nComponents = int(sensorsContent.getNumberNodes());

  components = new B2GeomVolume*[nComponents];
  for (Int_t iSensor = 0; iSensor < nComponents; iSensor++) {
    components[iSensor] = new B2GeomPXDSensor(iLayer, iLadder, iSensor);
    components[iSensor]->init(ladderContent);
  }
  //printf("B2GeomPXDLadder::init stop\n");
  return true;
}


Bool_t B2GeomPXDLadder::make()
{
  //printf("B2GeomPXDLadder::make start (Lay:%i, Lad:%i)\n", iLayer, iLadder);
  tVolume = new TGeoVolumeAssembly(name);
  for (int iSensor = 0; iSensor < nComponents; ++iSensor) {
    components[iSensor]->make();
    tVolume->AddNode(components[iSensor]->getVol(), 1, components[iSensor]->getPosition());
  }
  //printf("B2GeomPXDLadder::make stop\n");
  return true;
}




