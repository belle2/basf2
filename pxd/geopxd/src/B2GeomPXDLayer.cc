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

Bool_t B2GeomPXDLayer::init(GearDir& content)
{
  ////B2METHOD();
  // go to correct GearDir
  GearDir layerContent(content);
  layerContent.append((format("Layers/Layer[@id=\'PXD_Layer_%1%\']/") % (iLayer)).str());

  // read basic parameters from XML file
  if (!initBasicParameters(layerContent)) {
    B2FATAL("Could not initialize PXD layer!");
    return false;
  }

  // read starting angle of the windmill structure
  fPhi0 = layerContent.getParamAngle("Phi0");
  // read number of ladders for this layer
  nComponents = int(layerContent.getParamNumValue("NumberOfLadders"));
  fPhi.resize(nComponents);
  components = new B2GeomPXDLadder*[nComponents];
  for (int iComponent = 0; iComponent < nComponents; iComponent++) components[iComponent] = NULL;

  // initialize ladders
  for (iLadder = 0; iLadder < nComponents; iLadder++) {
    if (!initComponent<B2GeomPXDLadder>(&components[iLadder], layerContent, "Ladder")) {
      B2FATAL("Could not create PXD ladder.");
      return false;
    }
    // calculate position in phi of this ladder
    fPhi[iLadder] = iLadder * (360. / nComponents) + fPhi0;
  }
  return true;
}


Bool_t B2GeomPXDLayer::make()
{
  if (!makeGeneric()) {
    B2FATAL("Creating PXD Layer failed!\n");
    return false;
  }

  for (iLadder = 0; iLadder < nComponents; ++iLadder) {
    // create Ladder
    if (!components[iLadder]->make()) {
      B2FATAL("Could not build PXD ladder");
      return false;
    }

    // calculate ladder position (= add Phi rotation about z axis)
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
  return true;
}





