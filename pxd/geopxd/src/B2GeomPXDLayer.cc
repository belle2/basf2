/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <pxd/geopxd/B2GeomPXDLayer.h>
using namespace Belle2;
using namespace boost;

using namespace std;

B2GeomPXDLayer::B2GeomPXDLayer()
{
  B2GeomPXDLayer(-1);
}

B2GeomPXDLayer::B2GeomPXDLayer(Int_t iLay)
{
  iLayer = iLay;
  sprintf(name, "PXD_Layer_%i", iLayer);
  resetBasicParameters();
}

B2GeomPXDLayer::~B2GeomPXDLayer()
{

}


Bool_t B2GeomPXDLayer::init(GearDir& content)
{
  //printf("B2GeomPXDLayer::init start\n");
  GearDir layerContent(content);
  layerContent.append((format("Layers/Layer[@id=\'PXD_Layer_%1%\']/") % (iLayer)).str());
  initBasicParameters(layerContent);

  fPhi0 = layerContent.getParamAngle("Phi0");
  nComponents = int(layerContent.getParamNumValue("NumberOfLadders"));
  fPhi.resize(nComponents);
  components = new B2GeomVolume*[nComponents];
  for (Int_t iLadder = 0; iLadder < nComponents; iLadder++) {
    components[iLadder] = new B2GeomPXDLadder(iLayer, iLadder);
    components[iLadder]->init(layerContent);
    fPhi[iLadder] = iLadder * (360. / nComponents) + fPhi0;
  }
  //printf("B2GeomPXDLayer::init stop\n");
  return true;
}


Bool_t B2GeomPXDLayer::make()
{
  //printf("B2GeomPXDLayer::make start (Lay:%i)\n", iLayer);
  tVolume = new TGeoVolumeAssembly(name);

  for (int iLadder = 0; iLadder < nComponents; ++iLadder) {
    // create Ladder
    components[iLadder]->make();

    // calculate ladder position
    TGeoHMatrix pos;
    pos.CopyFrom(components[iLadder]->getPosition());
    TGeoRotation rotPhi = TGeoRotation("name", fPhi[iLadder], 0, 0);
    TGeoHMatrix hmaHelp;
    hmaHelp = pos * hmaHelp;
    hmaHelp = rotPhi * hmaHelp;
    TGeoHMatrix* hmaLadderPosition = new TGeoHMatrix(hmaHelp);
    // position ladder
    tVolume->AddNode(components[iLadder]->getVol(), 1, hmaLadderPosition);
  }
  //printf("B2GeomPXDLayer::make stop\n");
  return true;
}





