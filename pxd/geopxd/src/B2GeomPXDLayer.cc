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
  char text[200];
  sprintf(text, "PXD_Layer_%i", iLayer);
  path = string(text);

}

B2GeomPXDLayer::~B2GeomPXDLayer()
{

}


Bool_t B2GeomPXDLayer::init(GearDir& content)
{
  printf("B2GeomPXDLayer::init start\n");
  GearDir layerContent(content);
  layerContent.append((format("Layers/Layer[@id=\'PXD_Layer_%1%\']/") % (iLayer)).str());
  initBasicParameters(layerContent);

  fPhi0 = layerContent.getParamAngle("Phi0");
  nLadders = int(layerContent.getParamNumValue("NumberOfLadders"));
  fPhi.resize(nLadders);
  b2gPXDLadders = new B2GeomPXDLadder*[nLadders];
  for (Int_t iLadder = 0; iLadder < nLadders; iLadder++) {
    b2gPXDLadders[iLadder] = new B2GeomPXDLadder(iLayer, iLadder);
    b2gPXDLadders[iLadder]->init(layerContent);
    fPhi[iLadder] = iLadder * (360. / nLadders) + fPhi0;
  }
  printf("B2GeomPXDLayer::init stop\n");
  return true;
}


Bool_t B2GeomPXDLayer::make()
{
  printf("B2GeomPXDLayer::make start (Lay:%i)\n", iLayer);
  tVolume = new TGeoVolumeAssembly(path.c_str());

  for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
    // create Ladder
    b2gPXDLadders[iLadder]->make();

    // calculate ladder position
    TGeoHMatrix pos;
    pos.CopyFrom(b2gPXDLadders[iLadder]->getPosition());
    TGeoRotation rotPhi = TGeoRotation("name", fPhi[iLadder], 0, 0);
    TGeoHMatrix hmaHelp;
    hmaHelp = pos * hmaHelp;
    hmaHelp = rotPhi * hmaHelp;
    TGeoHMatrix* hmaLadderPosition = new TGeoHMatrix(hmaHelp);
    // position ladder
    tVolume->AddNode(b2gPXDLadders[iLadder]->getVol(), 1, hmaLadderPosition);
  }
  printf("B2GeomPXDLayer::make stop\n");
  return true;
}





