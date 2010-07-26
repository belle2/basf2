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
  volPXDLayer = 0;
  iLayer = iLay;
  path = (format("PXD_Layer_%1%") % iLayer).str();

}

B2GeomPXDLayer::~B2GeomPXDLayer()
{

}

Bool_t B2GeomPXDLayer::init(GearDir& content)
{
  layerContent = GearDir(content);
  layerContent.append((format("Layers/Layer[%1%]/") % (iLayer)).str());

  fPhi0 = layerContent.getParamAngle("Phi0");
  nLadders = int(layerContent.getParamNumValue("NumberOfLadders"));
  b2gPXDLadders.resize(nLadders);
  // iHalfShell.resize(nLadders);
  fPhi.resize(nLadders);
  fLadderOffsetY = layerContent.getParamLength("OffsetY");
  fLadderOffsetZ = layerContent.getParamLength("OffsetZ");;
  fRadius = layerContent.getParamLength("Radius");
  for (int iLadder = 0; iLadder < nLadders; iLadder++) {
    fPhi[iLadder] = iLadder * (360. / nLadders) + fPhi0;
    // iHalfShell[iLadder] = 0;
  }

  // offsets
  // fHalfShellOffsetZ.resize(2);
  // fHalfShellOffsetZ[0] = 0;
  // fHalfShellOffsetZ[1] = 0;
  return true;
}

Bool_t B2GeomPXDLayer::make()
{
  volPXDLayer = new TGeoVolumeAssembly(path.c_str());
  for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
    putLadder(iLadder);
  }
  return true;
}

void B2GeomPXDLayer::putLadder(Int_t iLadder)
{

  // position the ladder at radius
  TGeoTranslation traX = TGeoTranslation("name", fRadius, 0., 0.);
  // create the wind mill structure
  TGeoTranslation traY = TGeoTranslation("name", 0., fLadderOffsetY, 0.);
  // shift the ladder in z diretion
  TGeoTranslation traZ = TGeoTranslation("name", 0., 0., fLadderOffsetZ);
  // rotate around z
  TGeoRotation rotPhi = TGeoRotation("name", fPhi[iLadder], 0, 0);

  // compose the transformation to position the ladder
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = traX * hmaHelp;
  hmaHelp = traY * hmaHelp;
  hmaHelp = traZ * hmaHelp;
  hmaHelp = rotPhi * hmaHelp;

  // this transformation is finally used for positioning of the ladder
  TGeoHMatrix* hmaLadderPosition = new TGeoHMatrix(hmaHelp);

  // create Ladder
  b2gPXDLadders[iLadder] = new B2GeomPXDLadder(iLayer, iLadder);
  b2gPXDLadders[iLadder]->init(layerContent);
  b2gPXDLadders[iLadder]->make();

  volPXDLayer->AddNode(b2gPXDLadders[iLadder]->getVol(), 1, hmaLadderPosition);

}



