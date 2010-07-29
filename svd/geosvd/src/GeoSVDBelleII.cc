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
}


GeoSVDBelleII::~GeoSVDBelleII()
{

}


void GeoSVDBelleII::create(GearDir& content)
{
  //Get global parameters
  double globalRotAngle = content.getParamAngle("Rotation") / deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");
  //string sensorMatName  = content.getParamString("MaterialSensor");

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* volGrpSVD = new TGeoVolumeAssembly("SVD");

  TGeoRotation* geoRot = new TGeoRotation("SVDRot", 90.0, globalRotAngle, 0.0);
  topVolume->AddNode(volGrpSVD, 1, new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot)); //rotation followed by a translation

  //Get Material
  // TGeoMedium* sensorMed = gGeoManager->GetMedium(sensorMatName.c_str());

  //Get number of layers
  int nLayer = content.getNumberNodes("Layers/Layer");

  for (int iLayer = 1; iLayer <= nLayer; ++iLayer) {
    B2GeomSVDLayer* b2gSVDLayer = new B2GeomSVDLayer(iLayer);
    b2gSVDLayer->init(content);
    b2gSVDLayer->make();
    volGrpSVD->AddNode(b2gSVDLayer->getVol(), 1);
  }
}
