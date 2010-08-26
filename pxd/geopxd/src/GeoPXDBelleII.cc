/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#define B2GEOM_BASF2

#include <pxd/geopxd/GeoPXDBelleII.h>
#include <pxd/geopxd/B2GeomPXDLayer.h>

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>

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
  activateAutoSensitiveVolumes("SD_"); //The PXD subdetector uses the "SD_" prefix to flag its sensitive volumes
}


GeoPXDBelleII::~GeoPXDBelleII()
{

}


void GeoPXDBelleII::create(GearDir& content)
{
  //----------------------------------------
  //         Get global parameters
  //----------------------------------------
  double globalRotAngle = content.getParamAngle("Rotation") / deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");

  //----------------------------------------
  //        Add subdetector group
  //----------------------------------------
  TGeoRotation* geoRot = new TGeoRotation("PXDRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpPXD = addSubdetectorGroup("PXD", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));


  //----------------------------------------
  //           Build subdetector
  //----------------------------------------

  //Get number of layers
  int nLayer = content.getNumberNodes("Layers/Layer");

  for (int iLayer = 1; iLayer <= nLayer; ++iLayer) {

    //Build geometry
    B2GeomPXDLayer* b2gPXDLayer = new B2GeomPXDLayer(iLayer);
    b2gPXDLayer->init(content);
    b2gPXDLayer->make();

    volGrpPXD->AddNode(b2gPXDLayer->getVol(), 1);

  }
}
