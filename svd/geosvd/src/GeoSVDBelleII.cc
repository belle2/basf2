/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#define B2GEOM_BASF2

#include <svd/geosvd/GeoSVDBelleII.h>
#include <svd/geosvd/B2GeomSVDLayer.h>
#include <svd/simsvd/SVDSensitiveDetector.h>

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

GeoSVDBelleII regGeoSVDBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoSVDBelleII::GeoSVDBelleII() : CreatorBase("SVDBelleII")
{
  setDescription("Creates the TGeo objects for the SVD geometry of the Belle II detector.");
  addSensitiveDetector("SD_", new SVDSensitiveDetector("SVDSensitiveDetector"));
  //The SVD subdetector uses the "SD_" prefix to flag its sensitive volumes
}


GeoSVDBelleII::~GeoSVDBelleII()
{

}


void GeoSVDBelleII::create(GearDir& content)
{
  //         Get global parameters
  double globalRotAngle = content.getParamAngle("Rotation") / deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");

  //        Add subdetector group
  TGeoRotation* geoRot = new TGeoRotation("SVDRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpSVD = addSubdetectorGroup("SVD", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));

  //           Build subdetector

  for (int iLayer = 3; iLayer <= 6; iLayer++) {
    B2GeomSVDLayer* b2gSVDLayer = new B2GeomSVDLayer(); b2gSVDLayer->setId(iLayer);
    // go to correct directory in GearBox
    GearDir layerContent(content);
    layerContent.append((format("Layers/Layer[@id=\'SVD_Layer_%1%\']/") % (iLayer)).str());
    if (!b2gSVDLayer->init(layerContent)) {
      B2FATAL("Could not initialize SVD Layer");
      return;
    } else {
      b2gSVDLayer->setId(iLayer);
    }
    if (!b2gSVDLayer->make()) {
      B2FATAL("Could not build SVD Layer");
      return;
    }
    volGrpSVD->AddNode(b2gSVDLayer->getVol(), 1, new TGeoTranslation(0.0, 0.0, 0.0));
  }
}
