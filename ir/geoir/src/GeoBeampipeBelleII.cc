/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geoir/GeoBeampipeBelleII.h>

#include <gearbox/GearDir.h>
#include <datastore/Units.h>

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

GeoBeampipeBelleII regGeoBeampipeBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoBeampipeBelleII::GeoBeampipeBelleII() : CreatorBase("BeampipeBelleII")
{
  setDescription("Creates the TGeo objects for the beampipe geometry of the Belle II detector.");
}


GeoBeampipeBelleII::~GeoBeampipeBelleII()
{

}


void GeoBeampipeBelleII::create(GearDir& content)
{
  //Collect global parameters
  double rotAngle = content.getParamAngle("Rotation") / deg;
  double length   = 0.5 * content.getParamLength("Length");
  double offsetZ  = content.getParamLength("OffsetZ");

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* volGrpBP = new TGeoVolumeAssembly("Beampipe");

  TGeoRotation* geoRot = new TGeoRotation("BeampipeRot", 90.0, rotAngle, 0.0);
  topVolume->AddNode(volGrpBP, 1, new TGeoCombiTrans(0.0, 0.0, offsetZ, geoRot)); //rotation followed by a translation

  //Build Beampipe from shells
  int nShell = content.getNumberNodes("Shells/Shell");

  for (int iShell = 1; iShell <= nShell; ++iShell) {
    GearDir shellContent(content);
    shellContent.append((format("Shells/Shell[%1%]/") % (iShell)).str());

    //Collect information
    string shellName   = shellContent.getParamString("attribute::name");
    double innerRadius = shellContent.getParamLength("InnerRadius");
    double outerRadius = shellContent.getParamLength("OuterRadius");
    string matName     = shellContent.getParamString("Material");

    //Get Material
    TGeoMedium* shellMed = gGeoManager->GetMedium(matName.c_str());

    //Create Tube
    TGeoVolume* shellTube = gGeoManager->MakeTube(shellName.c_str(), shellMed, innerRadius, outerRadius, length);
    volGrpBP->AddNode(shellTube, 1);
  }
}
