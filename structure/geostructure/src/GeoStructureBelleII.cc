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
  double GlobalOffsetZ  = content.getParamAngle("OffsetZ") / Unit::rad;
  B2INFO("Structure : GlobalRotAngle (rad) =" << GlobalRotAngle);
  B2INFO("Structure : GlobalOffsetZ (cm)   =" << GlobalOffsetZ);

  TGeoRotation* geoRot = new TGeoRotation("STRRot", 90.0, 0.0, 0.0);
  TGeoVolumeAssembly* volSTR = addSubdetectorGroup("STR", new TGeoCombiTrans(0.0, 0.0, GlobalOffsetZ, geoRot));

  //Barrel KLM Vessel begins
  BarrelMgr.clear();
  BarrelMgr.read();

  //Get Material
  TGeoMedium* strMed4BKLM = gGeoManager->GetMedium(BarrelMgr.matname().c_str());

  TGeoVolume* strBarrelSTR = gGeoManager->MakePgon("BarrelSTR", strMed4BKLM,
                                                   BarrelMgr.phi() / Unit::deg, BarrelMgr.dphi() / Unit::deg,
                                                   BarrelMgr.nsides(), BarrelMgr.nBoundary());

  for (int iSet = 0; iSet < BarrelMgr.nBoundary(); iSet++) {
    ((TGeoPgon*)strBarrelSTR->GetShape())->DefineSection(iSet, BarrelMgr.z(iSet),
                                                         BarrelMgr.rmin(iSet),
                                                         BarrelMgr.rmax(iSet));
  }
  strBarrelSTR->SetAttVisibility(true);
  strBarrelSTR->SetLineColor(kBlue - 9);


  //Barrel KLM Slot begins

  //Get Material
  TGeoMedium* strMed4BKLMSlot = gGeoManager->GetMedium(BarrelMgr.matname4slot().c_str());
  TGeoMedium* strMed4BKLMMod  = gGeoManager->GetMedium(BarrelMgr.matname4module().c_str());

  TGeoVolume* strBironname[BarrelMgr.nmodule()];
  TGeoVolume* strBmodname[BarrelMgr.nmodule()];

  for (int i = 0; i < BarrelMgr.nmodule() ; i++) {
    stringstream str_set; str_set << i;
    std::string  Bironname = (i < 10) ? "I0" + str_set.str() : "I" + str_set.str();
    std::string  Bmodname = (i < 10) ? "M0" + str_set.str() : "M" + str_set.str();

    TVector3 size = BarrelMgr.barrelSlotSize(i);
    double BarrelKLM_r  = BarrelMgr.barrelSlotR(i);
    double BarrelKLM_dz = size.Z();
    double BarrelKLM_h  = size.X();
    double BarrelKLM_t  = (BarrelKLM_r + BarrelKLM_h) * tan(M_PI / float(BarrelMgr.nsector()));
    double BarrelKLM_b  = (BarrelKLM_r - BarrelKLM_h) * tan(M_PI / float(BarrelMgr.nsector()));

    /*
    B2INFO("Structure : BarrelKLM parameter r  =" << BarrelKLM_r);
    B2INFO("Structure : BarrelKLM parameter dz  =" << BarrelKLM_dz);
    B2INFO("Structure : BarrelKLM parameter h  =" << BarrelKLM_h);
    B2INFO("Structure : BarrelKLM parameter t  =" << BarrelKLM_t);
    B2INFO("Structure : BarrelKLM parameter b  =" << BarrelKLM_b);
    */

    strBironname[i] = gGeoManager->MakeTrap(Bironname.c_str(), strMed4BKLMSlot, BarrelKLM_dz, 0, 0,
                                            BarrelKLM_h, BarrelKLM_b, BarrelKLM_t, 0,
                                            BarrelKLM_h, BarrelKLM_b, BarrelKLM_t, 0);
    strBironname[i]->SetInvisible();
    strBironname[i]->SetLineColor(kBlue - 9);

    TVector3 msize = BarrelMgr.barrelModuleSize(i);
    strBmodname[i] = gGeoManager->MakeBox(Bmodname.c_str(), strMed4BKLMMod, size.Y(), size.X(), size.Z());
    strBmodname[i]->SetLineColor(kWhite);

    strBironname[i]->AddNode(strBmodname[i], 1, new TGeoTranslation(0.0, 0.0, 0.0));
  }

  int  nmod = 1;
  for (int fb = 0 ; fb < 2 ; fb++) {
    for (int sec = 0 ; sec < BarrelMgr.nsector() ; sec++) {
      for (int mod = 0 ; mod < BarrelMgr.nmodule() ; mod++) {
        double  rm = BarrelMgr.barrelSlotR(mod);
        double  x = rm * cos(M_PI * sec / 4.0);
        double  y = rm * sin(M_PI * sec / 4.0);
        double  z = (fb == 0) ? BarrelMgr.length_bslot() / 2. : -BarrelMgr.length_bslot() / 2.;

        TGeoTranslation t("shift", x, y, z);
        TGeoRotation r("rot", 0.0, 0.0, fmod(270. + 45.*sec, 360.));

        strBarrelSTR->AddNode(strBironname[mod], nmod++, new TGeoCombiTrans(t, r));
      }
    }
  }

  /*  //Endcap KLM Vessel setting is moved to "geoeklm"
  //Endcap KLM Vessel begins
  EndcapMgr.clear();
  EndcapMgr.read();

  //Get Material
  TGeoMedium* strMed4EKLM = gGeoManager->GetMedium(EndcapMgr.matname().c_str());

  TGeoVolume* strEndcapSTR[2];
  TGeoVolume* strEKLM_Layer[2];
  TGeoVolume* strEKLM_Slot[2];

  TGeoVolume* strTemp = gGeoManager->MakePgon("TempOct", strMed4EKLM,
                                              EndcapMgr.phi()/Unit::deg, EndcapMgr.dphi()/Unit::deg,
                EndcapMgr.nsides(), EndcapMgr.nBoundary());

  for (int iSet = 0; iSet < EndcapMgr.nBoundary() ; iSet++) {
    ((TGeoPgon*)strTemp->GetShape())->DefineSection(iSet, EndcapMgr.z(iSet),
                EndcapMgr.rmin(iSet), EndcapMgr.rmax(iSet));
  }

  TGeoVolume* strsubtube = gGeoManager->MakeTube("subtube", strMed4EKLM,
             EndcapMgr.rminsub(), EndcapMgr.rmaxsub(), EndcapMgr.zsub());
  TGeoCompositeShape* EndcapVessel = new TGeoCompositeShape("EKLMVessel", "TempOct-subtube");


  for (int i = 0 ; i < 2 ; i++){
    std::string EndcapSTR = (i == 0) ? "FEndcapSTR" : "BEndcapSTR";
    strEndcapSTR[i] = new TGeoVolume(EndcapSTR.c_str(), EndcapVessel, strMed4EKLM);
    strEndcapSTR[i]->SetLineColor(kBlue-9);

    std::string EKLM_Layer = (i == 0) ? "EKLM_FLayer" : "EKLM_BLayer";
    strEKLM_Layer[i] = gGeoManager->MakeTubs( EKLM_Layer.c_str(), strMed4EKLM,
                EndcapMgr.rmin_layer(), EndcapMgr.rmax_layer(),
                EndcapMgr.thick_eslot_meas()/2., 0, 90);
    strEKLM_Layer[i]->SetInvisible();
    strEKLM_Layer[i]->SetLineColor(kBlue-9);

    B2INFO("Endcap Module slot (=cavity) : Material : " << EndcapMgr.matname4module());
    TGeoMedium* strMed4EKLMSlot  = gGeoManager->GetMedium(EndcapMgr.matname4module().c_str());
    std::string EKLM_Slot = (i == 0) ? "EKLM_FSlot" : "EKLM_BSlot";
    strEKLM_Slot[i] = gGeoManager->MakeTubs(EKLM_Slot.c_str(), strMed4EKLMSlot,
              EndcapMgr.rmin_eslot()-EndcapMgr.rshift_eslot(),
              EndcapMgr.rmax_eslot()-EndcapMgr.rshift_eslot(),
              EndcapMgr.thick_eslot_meas()/2., 0, 90);
    strEKLM_Slot[i]->SetLineColor(kWhite);

    TVector3 pos = EndcapMgr.endcapModulePos( 0, 0 );
    strEKLM_Layer[i]->AddNode(strEKLM_Slot[i], 1, new TGeoTranslation(pos.X(), pos.Y(), 0.0));
  }


  for ( int fb = 0; fb < 2 ; fb++){
    nmod = 0;
    for( int sec=0;sec<4;sec++ ){
      for( int mod=0;mod<14;mod++ ){
  TVector3 pos = EndcapMgr.endcapModulePos( sec,mod );
  double posz = (fb == 0) ? pos.Z() : -pos.Z();
  TGeoTranslation t("shift", 0.0, 0.0, posz);
  TGeoRotation r("rot", 0.0, 0.0, 90.*sec);

  strEndcapSTR[fb]->AddNode(strEKLM_Layer[fb], ++nmod, new TGeoCombiTrans(t,r));
      }
    }
  }
  */

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

  volSTR->AddNode(strBarrelSTR, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  /*
  volSTR->AddNode(strEndcapSTR[0], 2, new TGeoTranslation(0.0, 0.0, 296.05));
  volSTR->AddNode(strEndcapSTR[1], 3, new TGeoTranslation(0.0, 0.0, -296.05));
  */

  TGeoTranslation poletip_t("shift", 0.0, 0.0, 0.0);
  TGeoRotation poletipf_r("rot", 0.0, 0.0, 0.0);
  //volSTR->AddNode(strPoletip, 4, new TGeoCombiTrans(poletip_t,poletipf_r));
  volSTR->AddNode(strPoletip, 2, new TGeoCombiTrans(poletip_t, poletipf_r));

  TGeoRotation poletipb_r("rot", 0.0, 180.0, 0.0);
  //volSTR->AddNode(strPoletip, 5, new TGeoCombiTrans(poletip_t,poletipb_r));
  volSTR->AddNode(strPoletip, 3, new TGeoCombiTrans(poletip_t, poletipb_r));

}

/*
const TVector3 GeoStructureBelleII::endcapModulePos(int sec, int mod) const
{
  return(TVector3(RSHIFT_ESLOT *((sec == 0 || sec == 3) ? 1 : -1),
                  RSHIFT_ESLOT *((sec == 0 || sec == 1) ? 1 : -1),
                  -(362.1 - 230.0) / 2. +
                  (THICK_EIRON + THICK_ESLOT)*(mod + 1) - THICK_ESLOT / 2.));
}
*/

/*
void GeoStructureBelleII::readParameters(GearDir& content)
{
  THICK_EIRON  = content.getParamLength("EndcapKLMSlot/THICK_EIRON");
  THICK_EIRON_MEAS = content.getParamLength("EndcapKLMSlot/THICK_EIRON_MEAS");
  THICK_ESLOT  = content.getParamLength("EndcapKLMSlot/THICK_ESLOT");
  THICK_ESLOT_MEAS = content.getParamLength("EndcapKLMSlot/THICK_ESLOT_MEAS");

  THICK_EMOD  = content.getParamLength("EndcapKLMSlot/THICK_EMOD");
  RMIN_EMOD   = content.getParamLength("EndcapKLMSlot/RMIN_EMOD");
  RMAX_EMOD   = content.getParamLength("EndcapKLMSlot/RMAX_EMOD");
  RMIN_ESLOT  = content.getParamLength("EndcapKLMSlot/RMIN_ESLOT");
  RMAX_ESLOT  = content.getParamLength("EndcapKLMSlot/RMAX_ESLOT");

  RSHIFT_ESLOT  = content.getParamLength("EndcapKLMSlot/RSHIFT_ESLOT");
  RMAX_GLASS    = content.getParamLength("EndcapKLMSlot/RMAX_GLASS");
}
*/
