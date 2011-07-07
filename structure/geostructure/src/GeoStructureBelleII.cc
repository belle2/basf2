/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors:                                                         *
 *                                                                        *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

//** todo:
//
//

#include <structure/geostructure/GeoStructureBelleII.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include <TGeoPgon.h>
#include <TGeoTube.h>
#include <TGeoCone.h>
#include "TGeoCompositeShape.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoStructureBelleII regGeoStructureBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoStructureBelleII::GeoStructureBelleII() : CreatorBase("StructureBelleII")
{
  setDescription("Creates the TGeo objects for the structure of the Belle II detector.");
}


GeoStructureBelleII::~GeoStructureBelleII()
{

}

// -----------------------------------------------------------
// --- TGeoShape function for BelleII Pipe segment ---
// --
// - Will create a pipe shape with either (but NOT both):
// - - different angle ends
// - - different radius ends

void GeoStructureBelleII::create(GearDir& content)
{

  // --- Collect global parameters
  double GlobalRotAngle = content.getParamAngle("Rotation") / Unit::rad;
  double GlobalOffsetZ  = content.getParamLength("OffsetZ") / Unit::cm;
  B2INFO("Structure : GlobalRotAngle (rad) =" << GlobalRotAngle);
  B2INFO("Structure : GlobalOffsetZ (cm)   =" << GlobalOffsetZ);

  TGeoRotation* geoRot = new TGeoRotation("STRRot", 90.0, 0.0, 0.0);
  TGeoVolumeAssembly* volSTR = addSubdetectorGroup("STR", new TGeoCombiTrans(0.0, 0.0, GlobalOffsetZ, geoRot));

  //Install Forward/Backward Pole tip of the Belle II Detector
  PoletipMgr.clear();
  PoletipMgr.read();

  B2INFO("Poletip : Poletip Material : " << PoletipMgr.matname().c_str());

  //Get Material
  TGeoMedium* strPoletipMed = gGeoManager->GetMedium(PoletipMgr.matname().c_str());

  TGeoVolume* strPoletip = gGeoManager->MakePcon("POLETIP", strPoletipMed,
                                                 PoletipMgr.phi() / Unit::deg, PoletipMgr.dphi() / Unit::deg,
                                                 PoletipMgr.nBoundary());
  strPoletip->SetLineColor(kBlue - 6);

  for (int iSet = 0; iSet < PoletipMgr.nBoundary(); iSet++) {
    ((TGeoPcon*)strPoletip->GetShape())->DefineSection(iSet, PoletipMgr.z(iSet),
                                                       PoletipMgr.rmin(iSet),
                                                       PoletipMgr.rmax(iSet));
  }

  TGeoTranslation poletip_t("shift", 0.0, 0.0, 0.0);
  TGeoRotation poletipf_r("rot", 0.0, 0.0, 0.0);
  volSTR->AddNode(strPoletip, 1, new TGeoCombiTrans(poletip_t, poletipf_r));

  TGeoRotation poletipb_r("rot", 0.0, 180.0, 0.0);
  volSTR->AddNode(strPoletip, 2, new TGeoCombiTrans(poletip_t, poletipb_r));

}

