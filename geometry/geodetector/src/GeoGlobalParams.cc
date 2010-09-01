/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/GeoGlobalParams.h>
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

GeoGlobalParams regGeoGlobalParams;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoGlobalParams::GeoGlobalParams() : CreatorBase("GeoGlobalParams")
{
  setDescription("Sets the global parameters for the Belle II detector geometry.");
}


GeoGlobalParams::~GeoGlobalParams()
{

}


void GeoGlobalParams::create(GearDir& content)
{
  //---------------------------------------------
  //             Create Top Volume
  //---------------------------------------------
  GearDir worldBoxDir(content, "WorldBox/");

  string mediumName = worldBoxDir.getParamString("Material");
  double xSize = worldBoxDir.getParamLength("X");
  double ySize = worldBoxDir.getParamLength("Y");
  double zSize = worldBoxDir.getParamLength("Z");

  TGeoMedium* vacuumMed = gGeoManager->GetMedium(mediumName.c_str());
  TGeoVolume* topVolume = gGeoManager->MakeBox("Top", vacuumMed, xSize, ySize, zSize);
  gGeoManager->SetTopVolume(topVolume);
}
