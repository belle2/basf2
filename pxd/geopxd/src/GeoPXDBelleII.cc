/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/GeoPXDBelleII.h>
#include <pxd/geopxd/B2GeomPXDLayer.h>
#include <framework/logging/Logger.h>
#include <pxd/simpxd/PXDSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>




using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoPXDBelleII regGeoPXDBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoPXDBelleII::GeoPXDBelleII() : CreatorBase("PXDBelleII")
{
  setDescription("Creates the TGeo objects for the PXD geometry of the Belle II detector.");
  //The PXD subdetector uses the "SD_" prefix to flag its sensitive volumes
  addSensitiveDetector("SD_", new PXDSensitiveDetector("PXDSensitiveDetector"));
}


GeoPXDBelleII::~GeoPXDBelleII()
{

}


void GeoPXDBelleII::create(GearDir& content)
{
  // Get global parameters
  double globalRotAngle = content.getParamAngle("Rotation") / Unit::deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");

  // Add subdetector group
  TGeoRotation* geoRot = new TGeoRotation("PXDRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpPXD = addSubdetectorGroup("PXD", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));

  // Build subdetector
  //Get number of layers
  int nLayer = content.getNumberNodes("Layers/Layer");
  if (nLayer == 0) {
    B2FATAL("Could not find any PXD layers in XML file.");
    return;
  }

  for (int iLayer = 1; iLayer <= nLayer; ++iLayer) {
    // initialize new PXD Layer object
    B2GeomPXDLayer* b2gPXDLayer = new B2GeomPXDLayer(); b2gPXDLayer->setId(iLayer);
    if (!b2gPXDLayer->init(content)) {
      B2FATAL("Could not build PXD Layer " << iLayer);
      return;
    }
    // build PXD layer in geometry
    if (!b2gPXDLayer->make()) {
      B2FATAL("Could not build PXD Layer " << iLayer);
    }
    // add PXD layer to geometry
    volGrpPXD->AddNode(b2gPXDLayer->getVol(), 1, new TGeoTranslation(0, 0, 0));
  }
}
