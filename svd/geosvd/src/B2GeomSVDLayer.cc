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
}

Bool_t B2GeomSVDLayer::init(GearDir& layerContent)
{
  //B2METHOD();
  B2INFO("Init SVD Layer " << iLayer);
  // get parameters from XML file
  if (!initBasicParameters(layerContent)) {
    B2FATAL("Could not initialize SVD layer!");
    return false;
  }

  // read the starting angle of the windmill structure
  fPhi0 = layerContent.getParamAngle("Phi0");

  // read the number of ladders
  nComponents = int(layerContent.getParamNumValue("NumberOfLadders"));
  if (nComponents < 1) {
    B2FATAL("No SVD ladders!");
    return false;
  }


  // initialize ladders
  components = new B2GeomSVDLadder*[nComponents];
  for (int iComponent = 0; iComponent < nComponents; iComponent++) components[iComponent] = NULL;
  for (iLadder = 0; iLadder < nComponents; iLadder++) {
    if (!initComponent<B2GeomSVDLadder>(&components[iLadder], layerContent, "Ladder")) {
      B2FATAL("Could not initialize SVD ladder");
      return false;
    }

    // add rotation about phi to the ladder parameters
    TGeoHMatrix* componentPositionOld = components[iLadder]->getPosition();
    if (componentPositionOld == NULL) {
      B2FATAL("Could not get position of SVD ladder!");
      return false;
    }
    TGeoRotation rotPhi("rotPhi", iLadder *(360. / nComponents) + fPhi0, 0.0, 0.0);
    TGeoHMatrix componentPositionNew;
    componentPositionNew.CopyFrom(componentPositionOld);
    componentPositionNew = rotPhi * componentPositionNew;
    TGeoHMatrix* componentPositionNewPointer = new TGeoHMatrix(componentPositionNew);
    components[iLadder]->setPosition(componentPositionNewPointer);
    delete componentPositionOld;
  }
  return true;
}

Bool_t B2GeomSVDLayer::make()
{
  //B2METHOD();
  B2INFO("Make SVD Layer " << iLayer);
  if (!makeGeneric()) {
    B2FATAL("Creating SVD Layer failed!");
    return false;
  }


  for (int iLadder = 0; iLadder < nComponents; ++iLadder) {
    // create Ladder
    if (!makeAndAddComponent(components[iLadder])) {
      B2FATAL("Could not build SVD ladder");
      return false;
    }
  }
  return true;
}
