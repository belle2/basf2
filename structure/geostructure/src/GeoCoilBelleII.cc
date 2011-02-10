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

#include <structure/geostructure/GeoCoilBelleII.h>

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
#include "TGeoCompositeShape.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoCoilBelleII regGeoCoilBelleII;
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoCoilBelleII::GeoCoilBelleII() : CreatorBase("CoilBelleII")
{
  setDescription("Creates the TGeo objects for the Belle II coil.");
}


GeoCoilBelleII::~GeoCoilBelleII()
{

}

void GeoCoilBelleII::create(GearDir& content)
{

  TGeoRotation* geoRot = new TGeoRotation("COILRot", 90.0, 0.0, 0.0);
  TGeoVolumeAssembly* volCOIL = addSubdetectorGroup("COIL", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));
  volCOIL->SetVisibility(1);
  volCOIL->SetVisContainers(1);
  //  volCOIL->SetVisOnly(1);
  volCOIL->VisibleDaughters(1);

  // --- Collect global parameters
  double GlobalRotAngle = content.getParamAngle("Rotation") / Unit::rad;
  double GlobalOffsetZ  = content.getParamLength("OffsetZ") / Unit::cm;
  B2INFO("Coil : GlobalRotAngle (rad) =" << GlobalRotAngle);
  B2INFO("Coil : GlobalOffsetZ (cm)   =" << GlobalOffsetZ);

  string strMatCryo = content.getParamString("Cryostat/Material");

  B2INFO("Coil : strMatCryo =" << strMatCryo.c_str());

  //Get Material
  TGeoMedium* strMedCryo = gGeoManager->GetMedium(strMatCryo.c_str());

  double CryoRmin   = content.getParamLength("Cryostat/Rmin");
  double CryoRmax   = content.getParamLength("Cryostat/Rmax");
  double CryoLength = content.getParamLength("Cryostat/HalfLength");

  TGeoVolume* strCryo = gGeoManager->MakeTube("Cryostat", strMedCryo, CryoRmin, CryoRmax, CryoLength);
  strCryo->SetLineColor(kGray + 3);
  volCOIL->AddNode(strCryo, 1, new TGeoTranslation(0.0, 0.0, GlobalOffsetZ));


  string strMatCav1 = content.getParamString("Cavity1/Material");
  //Get Material
  TGeoMedium* strMedCav1 = gGeoManager->GetMedium(strMatCav1.c_str());
  double Cav1Rmin   = content.getParamLength("Cavity1/Rmin");
  double Cav1Rmax   = content.getParamLength("Cavity1/Rmax");
  double Cav1Length = content.getParamLength("Cavity1/HalfLength");

  TGeoVolume* strCav1 = gGeoManager->MakeTube("Cavity1", strMedCav1, Cav1Rmin, Cav1Rmax, Cav1Length);
  strCav1->SetLineColor(kWhite);

  strCryo->AddNode(strCav1, 1, new TGeoTranslation(0.0, 0.0, 0.0));


  string strMatShield = content.getParamString("RadShield/Material");
  //Get Material
  TGeoMedium* strMedShield = gGeoManager->GetMedium(strMatShield.c_str());
  double ShieldRmin   = content.getParamLength("RadShield/Rmin");
  double ShieldRmax   = content.getParamLength("RadShield/Rmax");
  double ShieldLength = content.getParamLength("RadShield/HalfLength");

  TGeoVolume* strShield = gGeoManager->MakeTube("RadShield", strMedShield, ShieldRmin, ShieldRmax, ShieldLength);
  strShield->SetLineColor(kGray + 2);

  strCav1->AddNode(strShield, 1, new TGeoTranslation(0.0, 0.0, 0.0));


  string strMatCav2 = content.getParamString("Cavity2/Material");
  //Get Material
  TGeoMedium* strMedCav2 = gGeoManager->GetMedium(strMatCav2.c_str());
  double Cav2Rmin   = content.getParamLength("Cavity2/Rmin");
  double Cav2Rmax   = content.getParamLength("Cavity2/Rmax");
  double Cav2Length = content.getParamLength("Cavity2/HalfLength");

  TGeoVolume* strCav2 = gGeoManager->MakeTube("Cavity2", strMedCav2, Cav2Rmin, Cav2Rmax, Cav2Length);
  strCav2->SetLineColor(kWhite);

  strShield->AddNode(strCav2, 1, new TGeoTranslation(0.0, 0.0, 0.0));


  string strMatCoil = content.getParamString("Coil/Material");
  //Get Material
  TGeoMedium* strMedCoil = gGeoManager->GetMedium(strMatCoil.c_str());
  double CoilRmin   = content.getParamLength("Coil/Rmin");
  double CoilRmax   = content.getParamLength("Coil/Rmax");
  double CoilLength = content.getParamLength("Coil/HalfLength");

  TGeoVolume* strCoil = gGeoManager->MakeTube("Coil", strMedCoil, CoilRmin, CoilRmax, CoilLength);
  strCoil->SetLineColor(kGray + 2);

  strCav2->AddNode(strCoil, 1, new TGeoTranslation(0.0, 0.0, 0.0));
}

