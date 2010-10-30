/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDLayer.h>
using namespace Belle2;
using namespace boost;

using namespace std;

B2GeomSVDLayer::B2GeomSVDLayer()
{
  B2GeomSVDLayer(-1);
}

B2GeomSVDLayer::B2GeomSVDLayer(Int_t iLay)
{
  B2GeomVolume();
  iLayer = iLay;
  char text[200];
  sprintf(text, "SVD_Layer_%i", iLayer);
  path = string(text);
  printf("SVD Layer %i created! \n", iLayer);
}

B2GeomSVDLayer::~B2GeomSVDLayer()
{

}

Bool_t B2GeomSVDLayer::init(GearDir& content)
{
  GearDir layerContent(content);
  layerContent.append((format("Layers/Layer[@id=\'SVD_Layer_%1%\']/") % (iLayer)).str());
  fPhi0 = layerContent.getParamAngle("Phi0");
  nLadders = int(layerContent.getParamNumValue("NumberOfLadders"));
  b2gSVDLadders = new B2GeomSVDLadder*[nLadders];
  fPhi.resize(nLadders);
  for (int iLadder = 0; iLadder < nLadders; iLadder++) {
    b2gSVDLadders[iLadder] = new B2GeomSVDLadder(iLayer, iLadder);
    b2gSVDLadders[iLadder]->init(layerContent);
    fPhi[iLadder] = iLadder * (360. / nLadders) + fPhi0;
  }
  return true;
}

Bool_t B2GeomSVDLayer::make()
{
  tVolume = new TGeoVolumeAssembly(path.c_str());
  for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
    // create Ladder

    if (!b2gSVDLadders[iLadder]->make()) return false;

    // rotate around z
    TGeoRotation rotPhi = TGeoRotation("name", fPhi[iLadder], 0, 0);
    TGeoHMatrix pos;
    pos.CopyFrom(b2gSVDLadders[iLadder]->getPosition());
    pos = rotPhi * pos;

    // position the ladder in the SVD layer
    tVolume->AddNode(b2gSVDLadders[iLadder]->getVol(), 1, new TGeoHMatrix(pos));
  }
  return true;
}
