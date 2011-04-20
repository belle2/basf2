/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors: Hiroshi Nakano                                          *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

// ############################################################
// for your information, please see BelleII computing homepage and
// see /~twiki/bin/view/Computing/InteractionRegionPackages
// (basf2 Software Portal >> Interaction Region(IR) >> HOLDERgeom.pdf)
// ############################################################

#include <ir/geoir/GeoHolderBelleII.h>

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
#include <TGeoXtru.h>
#include <TGeoCone.h>
#include "TGeoCompositeShape.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoHolderBelleII regGeoHolderBelleII;
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoHolderBelleII::GeoHolderBelleII() : CreatorBase("HolderBelleII")
{
  setDescription("Creates the TGeo objects for the Belle II Holder.");
}


GeoHolderBelleII::~GeoHolderBelleII()
{

}

void GeoHolderBelleII::create(GearDir& content)
{

  TGeoRotation* geoRot = new TGeoRotation("HOLDERRot", 0.0, 0.0, 0.0);
  TGeoVolumeAssembly* volHOLDER = addSubdetectorGroup("HOLDER", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));
  volHOLDER->SetVisibility(1);
  volHOLDER->SetVisContainers(1);
  //volHOLDER->SetVisOnly(1);
  volHOLDER->VisibleDaughters(1);

  //#################################
  //#   Collect global parameters
  double GlobalRotAngle = content.getParamAngle("Rotation");
  double GlobalOffsetZ  = content.getParamLength("OffsetZ");
  B2INFO("Holder : GlobalRotAngle (rad) =" << GlobalRotAngle);
  B2INFO("Holder : GlobalOffsetZ (cm)   =" << GlobalOffsetZ);

  // Unit::rad = 1, Unit::deg = 0.0174533 = pi/180
  //#
  //#################################

  //variables
  double zpos;

  //##############
  //#   SVD Heavy metal mask

  //==============
  //=   R side 1/2 (R1hmm1)

  //get parameters from .xml file
  double R1hmm1d1 = content.getParamLength("R1hmm1/d1"); //from IP
  //
  double R1hmm1l1 = content.getParamLength("R1hmm1/l1");
  double R1hmm1l2 = content.getParamLength("R1hmm1/l2");
  //
  double R1hmm1i1 = content.getParamLength("R1hmm1/i1");
  double R1hmm1i2 = content.getParamLength("R1hmm1/i2");
  //
  double R1hmm1o1 = content.getParamLength("R1hmm1/o1");
  double R1hmm1o2 = content.getParamLength("R1hmm1/o2");
  string strMatR1hmm1 = content.getParamString("R1hmm1/Material");
  TGeoMedium* strMedR1hmm1 = gGeoManager->GetMedium(strMatR1hmm1.c_str());

  //define geometry
  TGeoPcon* geoR1hmm1pcon = new TGeoPcon(0, 360, 3);
  zpos = R1hmm1d1;
  geoR1hmm1pcon->DefineSection(0, zpos, R1hmm1i1, R1hmm1o1);
  zpos = zpos + R1hmm1l1;
  geoR1hmm1pcon->DefineSection(1, zpos, R1hmm1i2, R1hmm1o2);
  zpos = zpos + R1hmm1l2;
  geoR1hmm1pcon->DefineSection(2, zpos, R1hmm1i2, R1hmm1o2);
  geoR1hmm1pcon->SetName("geoR1hmm1pconname");
  //
  TGeoVolume *volR1hmm1 = new TGeoVolume("volR1hmm1name", geoR1hmm1pcon, strMedR1hmm1);

  //-   put volume
  volR1hmm1->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volR1hmm1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   R side 2/2 (R1hmm2)

  //get parameters from .xml file
  double R1hmm2d1 = content.getParamLength("R1hmm2/d1"); //from IP
  //
  double R1hmm2l1 = content.getParamLength("R1hmm2/l1");
  double R1hmm2l2 = content.getParamLength("R1hmm2/l2");
  double R1hmm2l3 = content.getParamLength("R1hmm2/l3");
  double R1hmm2l4 = content.getParamLength("R1hmm2/l4");
  double R1hmm2l5 = content.getParamLength("R1hmm2/l5");
  //
  double R1hmm2i1 = content.getParamLength("R1hmm2/i1");
  double R1hmm2i2 = content.getParamLength("R1hmm2/i2");
  //
  double R1hmm2o1 = content.getParamLength("R1hmm2/o1");
  double R1hmm2o2 = content.getParamLength("R1hmm2/o2");
  double R1hmm2o3 = content.getParamLength("R1hmm2/o3");
  double R1hmm2o4 = content.getParamLength("R1hmm2/o4");
  string strMatR1hmm2 = content.getParamString("R1hmm2/Material");
  TGeoMedium* strMedR1hmm2 = gGeoManager->GetMedium(strMatR1hmm2.c_str());

  //define geometry
  TGeoPcon* geoR1hmm2pcon = new TGeoPcon(0, 360, 8);
  zpos = R1hmm2d1;
  geoR1hmm2pcon->DefineSection(0, zpos, R1hmm2i1, R1hmm2o1);
  zpos = zpos + R1hmm2l1;
  geoR1hmm2pcon->DefineSection(1, zpos, R1hmm2i1, R1hmm2o1);
  geoR1hmm2pcon->DefineSection(2, zpos, R1hmm2i1, R1hmm2o2);
  zpos = zpos + R1hmm2l2;
  geoR1hmm2pcon->DefineSection(3, zpos, R1hmm2i1, R1hmm2o2);
  zpos = zpos + R1hmm2l3;
  geoR1hmm2pcon->DefineSection(4, zpos, R1hmm2i2, R1hmm2o3);
  zpos = zpos + R1hmm2l4;
  geoR1hmm2pcon->DefineSection(5, zpos, R1hmm2i2, R1hmm2o3);
  geoR1hmm2pcon->DefineSection(6, zpos, R1hmm2i2, R1hmm2o4);
  zpos = zpos + R1hmm2l5;
  geoR1hmm2pcon->DefineSection(7, zpos, R1hmm2i2, R1hmm2o4);
  geoR1hmm2pcon->SetName("geoR1hmm2pconname");
  //
  TGeoVolume *volR1hmm2 = new TGeoVolume("volR1hmm2name", geoR1hmm2pcon, strMedR1hmm2);

  //-   put volume
  volR1hmm2->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volR1hmm2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   L side 1/2 (L1hmm1)

  //get parameters from .xml file
  double L1hmm1d1 = content.getParamLength("L1hmm1/d1");
  double L1hmm1l1 = content.getParamLength("L1hmm1/l1");
  double L1hmm1l2 = content.getParamLength("L1hmm1/l2");
  double L1hmm1l3 = content.getParamLength("L1hmm1/l3");
  double L1hmm1l4 = content.getParamLength("L1hmm1/l4");
  double L1hmm1i1 = content.getParamLength("L1hmm1/i1");
  double L1hmm1i2 = content.getParamLength("L1hmm1/i2");
  double L1hmm1i3 = content.getParamLength("L1hmm1/i3");
  double L1hmm1o1 = content.getParamLength("L1hmm1/o1");
  double L1hmm1o2 = content.getParamLength("L1hmm1/o2");
  double L1hmm1o3 = content.getParamLength("L1hmm1/o3");
  string strMatL1hmm1 = content.getParamString("L1hmm1/Material");
  TGeoMedium* strMedL1hmm1 = gGeoManager->GetMedium(strMatL1hmm1.c_str());

  //define geometry
  TGeoPcon* geoL1hmm1pcon = new TGeoPcon(0, 360, 7);
  zpos = L1hmm1d1;
  geoL1hmm1pcon->DefineSection(0, zpos, L1hmm1i1, L1hmm1o1);
  zpos = zpos + L1hmm1l1;
  geoL1hmm1pcon->DefineSection(1, zpos, L1hmm1i2, L1hmm1o2);
  zpos = zpos + L1hmm1l2;
  geoL1hmm1pcon->DefineSection(2, zpos, L1hmm1i2, L1hmm1o2);
  geoL1hmm1pcon->DefineSection(3, zpos, L1hmm1i2, L1hmm1o3);
  zpos = zpos + L1hmm1l3;
  geoL1hmm1pcon->DefineSection(4, zpos, L1hmm1i2, L1hmm1o3);
  geoL1hmm1pcon->DefineSection(5, zpos, L1hmm1i3, L1hmm1o3);
  zpos = zpos + L1hmm1l4;
  geoL1hmm1pcon->DefineSection(6, zpos, L1hmm1i3, L1hmm1o3);
  geoL1hmm1pcon->SetName("geoL1hmm1pconname");
  //
  TGeoVolume *volL1hmm1 = new TGeoVolume("volL1hmm1name", geoL1hmm1pcon, strMedL1hmm1);

  //-   put volume
  volL1hmm1->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volL1hmm1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   L side 2/2 (L1hmm2)

  //get parameters from .xml file
  double L1hmm2d1 = content.getParamLength("L1hmm2/d1");
  double L1hmm2l1 = content.getParamLength("L1hmm2/l1");
  double L1hmm2l2 = content.getParamLength("L1hmm2/l2");
  double L1hmm2i1 = content.getParamLength("L1hmm2/i1");
  double L1hmm2o1 = content.getParamLength("L1hmm2/o1");
  double L1hmm2o2 = content.getParamLength("L1hmm2/o2");
  string strMatL1hmm2 = content.getParamString("L1hmm2/Material");
  TGeoMedium* strMedL1hmm2 = gGeoManager->GetMedium(strMatL1hmm2.c_str());

  //define geometry
  TGeoPcon* geoL1hmm2pcon = new TGeoPcon(0, 360, 4);
  zpos = L1hmm2d1;
  geoL1hmm2pcon->DefineSection(0, zpos, L1hmm2i1, L1hmm2o1);
  zpos = zpos + L1hmm2l1;
  geoL1hmm2pcon->DefineSection(1, zpos, L1hmm2i1, L1hmm2o1);
  geoL1hmm2pcon->DefineSection(2, zpos, L1hmm2i1, L1hmm2o2);
  zpos = zpos + L1hmm2l2;
  geoL1hmm2pcon->DefineSection(3, zpos, L1hmm2i1, L1hmm2o2);
  geoL1hmm2pcon->SetName("geoL1hmm2pconname");
  //
  TGeoVolume *volL1hmm2 = new TGeoVolume("volL1hmm2name", geoL1hmm2pcon, strMedL1hmm2);

  //-   put volume
  volL1hmm2->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volL1hmm2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //-
  //--------------

  //##############
  //#   End flange

  //==============
  //=   R side (R1plt1)

  //get parameters from .xml file
  double R1plt1d1 = content.getParamLength("R1plt1/d1");
  double R1plt1l1 = content.getParamLength("R1plt1/l1");
  double R1plt1i1 = content.getParamLength("R1plt1/i1");
  double R1plt1o1 = content.getParamLength("R1plt1/o1");
  string strMatR1plt1 = content.getParamString("R1plt1/Material");
  TGeoMedium* strMedR1plt1 = gGeoManager->GetMedium(strMatR1plt1.c_str());

  //define geometry
  TGeoPcon* geoR1plt1pcon = new TGeoPcon(0, 360, 2);
  zpos = R1plt1d1;
  geoR1plt1pcon->DefineSection(0, zpos, R1plt1i1, R1plt1o1);
  zpos = zpos + R1plt1l1;
  geoR1plt1pcon->DefineSection(1, zpos, R1plt1i1, R1plt1o1);
  geoR1plt1pcon->SetName("geoR1plt1pconname");
  //
  TGeoVolume *volR1plt1 = new TGeoVolume("volR1plt1name", geoR1plt1pcon, strMedR1plt1);

  //-   put volume
  volR1plt1->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volR1plt1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   L side (L1plt1)

  //get parameters from .xml file
  double L1plt1d1 = content.getParamLength("L1plt1/d1");
  double L1plt1l1 = content.getParamLength("L1plt1/l1");
  double L1plt1i1 = content.getParamLength("L1plt1/i1");
  double L1plt1o1 = content.getParamLength("L1plt1/o1");
  string strMatL1plt1 = content.getParamString("L1plt1/Material");
  TGeoMedium* strMedL1plt1 = gGeoManager->GetMedium(strMatL1plt1.c_str());

  //define geometry
  TGeoPcon* geoL1plt1pcon = new TGeoPcon(0, 360, 2);
  zpos = L1plt1d1;
  geoL1plt1pcon->DefineSection(0, zpos, L1plt1i1, L1plt1o1);
  zpos = zpos + L1plt1l1;
  geoL1plt1pcon->DefineSection(1, zpos, L1plt1i1, L1plt1o1);
  geoL1plt1pcon->SetName("geoL1plt1pconname");
  //
  TGeoVolume *volL1plt1 = new TGeoVolume("volL1plt1name", geoL1plt1pcon, strMedL1plt1);

  //-   put volume
  volL1plt1->SetLineColor(kGray + 3);
  volHOLDER->AddNode(volL1plt1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------
}

