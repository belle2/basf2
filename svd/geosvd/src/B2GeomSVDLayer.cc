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
  resetBasicParameters();
  iLayer = iLay;
  printf("Create SVD Layer %i! \n", iLayer);
  sprintf(name, "SVD_Layer_%i", iLayer);
}

B2GeomSVDLayer::~B2GeomSVDLayer()
{

}

Bool_t B2GeomSVDLayer::init(GearDir& content)
{
  GearDir layerContent(content);
  layerContent.append((format("Layers/Layer[@id=\'SVD_Layer_%1%\']/") % (iLayer)).str());
  fPhi0 = layerContent.getParamAngle("Phi0");
  nComponents = int(layerContent.getParamNumValue("NumberOfLadders"));
  components = new B2GeomVolume*[nComponents];
  for (int iLadder = 0; iLadder < nComponents; iLadder++) {
    components[iLadder] = new B2GeomSVDLadder(iLayer, iLadder);
    components[iLadder]->init(layerContent);

    // add rotation about phi to the ladder parameters
    TGeoHMatrix* componentPositionOld = components[iLadder]->getPosition();
    TGeoRotation rotPhi("rotPhi", iLadder *(360. / nComponents) + fPhi0, 0.0, 0.0);
    TGeoHMatrix componentPositionNew;
    componentPositionNew.CopyFrom(componentPositionOld);
    componentPositionNew = rotPhi * componentPositionNew;
    TGeoHMatrix* componentPositionNewPointer = new TGeoHMatrix(componentPositionNew);
    components[iLadder]->setPosition(componentPositionNewPointer);
    delete componentPositionOld;
    //delete componentPositionNewPointer;
  }
  return true;
}

Bool_t B2GeomSVDLayer::make()
{
  tVolume = new TGeoVolumeAssembly(name);
  for (int iLadder = 0; iLadder < nComponents; ++iLadder) {
    // create Ladder

    if (!components[iLadder]->make()) return false;

    // position the ladder in the SVD layer
    tVolume->AddNode(components[iLadder]->getVol(), 1, components[iLadder]->getPosition());
  }
  return true;
}
