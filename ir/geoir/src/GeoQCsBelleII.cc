/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors:                                                         *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

#include <ir/geoir/GeoQCsBelleII.h>

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

GeoQCsBelleII regGeoQCsBelleII;
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoQCsBelleII::GeoQCsBelleII() : CreatorBase("QCsBelleII")
{
  setDescription("Creates the TGeo objects for the Belle II QCs.");
}


GeoQCsBelleII::~GeoQCsBelleII()
{

}

void GeoQCsBelleII::create(GearDir& content)
{

  TGeoRotation* geoRot = new TGeoRotation("QCSRot", 0.0, 0.0, 0.0);
  TGeoVolumeAssembly* volQCS = addSubdetectorGroup("QCS", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));
  volQCS->SetVisibility(1);
  volQCS->SetVisContainers(1);
  //volQCS->SetVisOnly(1);
  volQCS->VisibleDaughters(1);

  //#################################
  //#   Collect global parameters
  double GlobalRotAngle = content.getParamAngle("Rotation");
  double GlobalOffsetZ  = content.getParamLength("OffsetZ");
  B2INFO("QCs : GlobalRotAngle (rad) =" << GlobalRotAngle);
  B2INFO("QCs : GlobalOffsetZ (cm)   =" << GlobalOffsetZ);

  double GlobalDistanceR  = content.getParamLength("DistanceR");
  double GlobalDistanceL  = content.getParamLength("DistanceL");
  double GlobalAngleHER = content.getParamAngle("AngleHER") / Unit::deg;
  double GlobalAngleLER = content.getParamAngle("AngleLER") / Unit::deg;
  // Unit::rad = 1, Unit::deg = 0.0174533 = pi/180
  //#
  //#################################

  //variables
  double bpthick;
  double zpos;
  int    count;
  //rotations
  TGeoRotation* rotHER = new TGeoRotation("rotHERname", 90., GlobalAngleHER, -90.);
  rotHER->RegisterYourself();
  TGeoRotation* rotLER = new TGeoRotation("rotLERname", 90., GlobalAngleLER, -90.);
  rotLER->RegisterYourself();

  //##############
  //#   CryoR

  //==============
  //=   Group A

  //--------------
  //-   A1wal1

  //get parameters from .xml file
  double A1wal1l1 = content.getParamLength("A1wal1/l1");
  double A1wal1l2 = content.getParamLength("A1wal1/l2");
  double A1wal1l3 = content.getParamLength("A1wal1/l3");
  double A1wal1l4 = content.getParamLength("A1wal1/l4");
  double A1wal1l5 = content.getParamLength("A1wal1/l5");
  double A1wal1l6 = content.getParamLength("A1wal1/l6");
  double A1wal1l7 = content.getParamLength("A1wal1/l7");
  double A1wal1r1 = content.getParamLength("A1wal1/r1");
  double A1wal1r2 = content.getParamLength("A1wal1/r2");
  double A1wal1r3 = content.getParamLength("A1wal1/r3");
  double A1wal1r4 = content.getParamLength("A1wal1/r4");
  string strMatA1wal1 = content.getParamString("A1wal1/Material");
  TGeoMedium* strMedA1wal1 = gGeoManager->GetMedium(strMatA1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoA1wal1pcon = new TGeoPcon(0, 360, 8);
  geoA1wal1pcon->DefineSection(0, zpos, 0, A1wal1r1);
  zpos = zpos + A1wal1l1;
  geoA1wal1pcon->DefineSection(1, zpos, 0, A1wal1r1);
  zpos = zpos + A1wal1l2;
  geoA1wal1pcon->DefineSection(2, zpos, 0, A1wal1r2);
  zpos = zpos + A1wal1l3;
  geoA1wal1pcon->DefineSection(3, zpos, 0, A1wal1r2);
  zpos = zpos + A1wal1l4;
  geoA1wal1pcon->DefineSection(4, zpos, 0, A1wal1r3);
  zpos = zpos + A1wal1l5;
  geoA1wal1pcon->DefineSection(5, zpos, 0, A1wal1r3);
  zpos = zpos + A1wal1l6;
  geoA1wal1pcon->DefineSection(6, zpos, 0, A1wal1r4);
  zpos = zpos + A1wal1l7;
  geoA1wal1pcon->DefineSection(7, zpos, 0, A1wal1r4);
  geoA1wal1pcon->SetName("geoA1wal1pconname");

  //-   Subtraction volume
  TGeoTube* geoA1wal1tub = new TGeoTube(0, 10, GlobalDistanceR);
  geoA1wal1tub->SetName("geoA1wal1tubname");

  TGeoCompositeShape* geoA1wal1 = new TGeoCompositeShape("geoA1wal1name", "geoA1wal1pconname:rotHERname - geoA1wal1tubname");
  TGeoVolume *volA1wal1 = new TGeoVolume("volA1wal1name", geoA1wal1, strMedA1wal1);

  //-   put volume
  volA1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volA1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   A2spc1

  //get parameters from .xml file
  bpthick = content.getParamLength("A2spc1/bpthick");
  //double A2spc1l1 = A1wal1l1;
  //double A2spc1l2 = A1wal1l2;
  //double A2spc1l3 = A1wal1l3;
  //double A2spc1l4 = A1wal1l4;
  //double A2spc1l5 = A1wal1l5;
  //double A2spc1l6 = A1wal1l6;
  //double A2spc1l7 = A1wal1l7;
  double A2spc1r1 = A1wal1r1 - bpthick;
  double A2spc1r2 = A1wal1r2 - bpthick;
  double A2spc1r3 = A1wal1r3 - bpthick;
  double A2spc1r4 = A1wal1r4 - bpthick;
  string strMatA2spc1 = content.getParamString("A2spc1/Material");
  TGeoMedium* strMedA2spc1 = gGeoManager->GetMedium(strMatA2spc1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoA2spc1pcon = new TGeoPcon(0, 360, 8);
  geoA2spc1pcon->DefineSection(0, zpos, 0, A2spc1r1);
  zpos = zpos + A1wal1l1;
  geoA2spc1pcon->DefineSection(1, zpos, 0, A2spc1r1);
  zpos = zpos + A1wal1l2;
  geoA2spc1pcon->DefineSection(2, zpos, 0, A2spc1r2);
  zpos = zpos + A1wal1l3;
  geoA2spc1pcon->DefineSection(3, zpos, 0, A2spc1r2);
  zpos = zpos + A1wal1l4;
  geoA2spc1pcon->DefineSection(4, zpos, 0, A2spc1r3);
  zpos = zpos + A1wal1l5;
  geoA2spc1pcon->DefineSection(5, zpos, 0, A2spc1r3);
  zpos = zpos + A1wal1l6;
  geoA2spc1pcon->DefineSection(6, zpos, 0, A2spc1r4);
  zpos = zpos + A1wal1l7;
  geoA2spc1pcon->DefineSection(7, zpos, 0, A2spc1r4);
  geoA2spc1pcon->SetName("geoA2spc1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoA2spc1 = new TGeoCompositeShape("geoA2spc1name", "geoA2spc1pconname:rotHERname * geoA1wal1name");
  TGeoVolume *volA2spc1 = new TGeoVolume("volA2spc1name", geoA2spc1, strMedA2spc1);

  //-   put volume
  volA2spc1->SetLineColor(kGray + 1);
  volA1wal1->AddNode(volA2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //    Group B
  //==============

  //--------------
  //-   B1wal1

  //get parameters from .xml file
  double B1wal1l1 = content.getParamLength("B1wal1/l1");
  double B1wal1l2 = content.getParamLength("B1wal1/l2");
  double B1wal1l3 = content.getParamLength("B1wal1/l3");
  double B1wal1l4 = content.getParamLength("B1wal1/l4");
  double B1wal1l5 = content.getParamLength("B1wal1/l5");
  double B1wal1l6 = content.getParamLength("B1wal1/l6");
  double B1wal1l7 = content.getParamLength("B1wal1/l7");
  double B1wal1l8 = content.getParamLength("B1wal1/l8");
  double B1wal1l9 = content.getParamLength("B1wal1/l9");
  //
  double B1wal1r1 = content.getParamLength("B1wal1/r1");
  double B1wal1r2 = content.getParamLength("B1wal1/r2");
  double B1wal1r3 = content.getParamLength("B1wal1/r3");
  double B1wal1r4 = content.getParamLength("B1wal1/r4");
  double B1wal1r5 = content.getParamLength("B1wal1/r5");
  double B1wal1r6 = content.getParamLength("B1wal1/r6");
  string strMatB1wal1 = content.getParamString("B1wal1/Material");
  TGeoMedium* strMedB1wal1 = gGeoManager->GetMedium(strMatB1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoB1wal1pcon = new TGeoPcon(0, 360, 10);
  geoB1wal1pcon->DefineSection(0, zpos, 0, B1wal1r1);
  zpos = zpos + B1wal1l1;
  geoB1wal1pcon->DefineSection(1, zpos, 0, B1wal1r1);
  zpos = zpos + B1wal1l2;
  geoB1wal1pcon->DefineSection(2, zpos, 0, B1wal1r2);
  zpos = zpos + B1wal1l3;
  geoB1wal1pcon->DefineSection(3, zpos, 0, B1wal1r3);
  zpos = zpos + B1wal1l4;
  geoB1wal1pcon->DefineSection(4, zpos, 0, B1wal1r3);
  zpos = zpos + B1wal1l5;
  geoB1wal1pcon->DefineSection(5, zpos, 0, B1wal1r4);
  zpos = zpos + B1wal1l6;
  geoB1wal1pcon->DefineSection(6, zpos, 0, B1wal1r5);
  zpos = zpos + B1wal1l7;
  geoB1wal1pcon->DefineSection(7, zpos, 0, B1wal1r5);
  zpos = zpos + B1wal1l8;
  geoB1wal1pcon->DefineSection(8, zpos, 0, B1wal1r6);
  zpos = zpos + B1wal1l9;
  geoB1wal1pcon->DefineSection(9, zpos, 0, B1wal1r6);
  geoB1wal1pcon->SetName("geoB1wal1pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoB1wal1 = new TGeoCompositeShape("geoB1wal1name", "geoB1wal1pconname:rotLERname - geoA1wal1tubname");
  TGeoVolume *volB1wal1 = new TGeoVolume("volB1wal1name", geoB1wal1, strMedB1wal1);

  //-   put volume
  volB1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volB1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   B2spc1

  //get parameters from .xml file
  bpthick = content.getParamLength("B2spc1/bpthick");
  //double B2spc1d1 = B1wal1d1;
  //double B2spc1l1 = B1wal1l1;
  //double B2spc1l2 = B1wal1l2;
  //double B2spc1l3 = B1wal1l3;
  //double B2spc1l4 = B1wal1l4;
  //double B2spc1l5 = B1wal1l5;
  //double B2spc1l6 = B1wal1l6;
  //double B2spc1l7 = B1wal1l7;
  double B2spc1r1 = B1wal1r1 - bpthick;
  double B2spc1r2 = B1wal1r2 - bpthick;
  double B2spc1r3 = B1wal1r3 - bpthick;
  double B2spc1r4 = B1wal1r4 - bpthick;
  double B2spc1r5 = B1wal1r5 - bpthick;
  double B2spc1r6 = B1wal1r6 - bpthick;
  string strMatB2spc1 = content.getParamString("B2spc1/Material");
  TGeoMedium* strMedB2spc1 = gGeoManager->GetMedium(strMatB2spc1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoB2spc1pcon = new TGeoPcon(0, 360, 10);
  geoB2spc1pcon->DefineSection(0, zpos, 0, B2spc1r1);
  zpos = zpos + B1wal1l1;
  geoB2spc1pcon->DefineSection(1, zpos, 0, B2spc1r1);
  zpos = zpos + B1wal1l2;
  geoB2spc1pcon->DefineSection(2, zpos, 0, B2spc1r2);
  zpos = zpos + B1wal1l3;
  geoB2spc1pcon->DefineSection(3, zpos, 0, B2spc1r3);
  zpos = zpos + B1wal1l4;
  geoB2spc1pcon->DefineSection(4, zpos, 0, B2spc1r3);
  zpos = zpos + B1wal1l5;
  geoB2spc1pcon->DefineSection(5, zpos, 0, B2spc1r4);
  zpos = zpos + B1wal1l6;
  geoB2spc1pcon->DefineSection(6, zpos, 0, B2spc1r5);
  zpos = zpos + B1wal1l7;
  geoB2spc1pcon->DefineSection(7, zpos, 0, B2spc1r5);
  zpos = zpos + B1wal1l8;
  geoB2spc1pcon->DefineSection(8, zpos, 0, B2spc1r6);
  zpos = zpos + B1wal1l9;
  geoB2spc1pcon->DefineSection(9, zpos, 0, B2spc1r6);
  geoB2spc1pcon->SetName("geoB2spc1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoB2spc1 = new TGeoCompositeShape("geoB2spc1name", "geoB2spc1pconname:rotLERname * geoB1wal1name");
  TGeoVolume *volB2spc1 = new TGeoVolume("volB2spc1name", geoB2spc1, strMedB2spc1);

  //-   put volume
  volB2spc1->SetLineColor(kGray + 1);
  volB1wal1->AddNode(volB2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   Group C

  //--------------
  //-   C1wal1

  //get parameters from .xml file
  double C1wal1d1 = GlobalDistanceR;
  //
  double C1wal1l1 = content.getParamLength("C1wal1/l1");
  double C1wal1l2 = content.getParamLength("C1wal1/l2");
  double C1wal1l3 = content.getParamLength("C1wal1/l3");
  double C1wal1l4 = content.getParamLength("C1wal1/l4");
  double C1wal1l5 = content.getParamLength("C1wal1/l5");
  double C1wal1l6 = content.getParamLength("C1wal1/l6");
  double C1wal1l7 = content.getParamLength("C1wal1/l7");
  //
  double C1wal1r1 = content.getParamLength("C1wal1/r1");
  double C1wal1r2 = content.getParamLength("C1wal1/r2");
  double C1wal1r3 = content.getParamLength("C1wal1/r3");
  double C1wal1r4 = content.getParamLength("C1wal1/r4");
  double C1wal1r5 = content.getParamLength("C1wal1/r5");
  double C1wal1r6 = content.getParamLength("C1wal1/r6");
  string strMatC1wal1 = content.getParamString("C1wal1/Material");
  TGeoMedium* strMedC1wal1 = gGeoManager->GetMedium(strMatC1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoC1wal1pcon = new TGeoPcon(0, 360, 11);
  zpos = zpos + C1wal1d1;
  geoC1wal1pcon->DefineSection(0, zpos, 0, C1wal1r1);
  zpos = zpos + C1wal1l1;
  geoC1wal1pcon->DefineSection(1, zpos, 0, C1wal1r2);
  zpos = zpos + C1wal1l2;
  geoC1wal1pcon->DefineSection(2, zpos, 0, C1wal1r2);
  zpos = zpos + C1wal1l3;
  geoC1wal1pcon->DefineSection(3, zpos, 0, C1wal1r3);
  zpos = zpos + C1wal1l4;
  geoC1wal1pcon->DefineSection(4, zpos, 0, C1wal1r3);
  geoC1wal1pcon->DefineSection(5, zpos, 0, C1wal1r4);
  zpos = zpos + C1wal1l5;
  geoC1wal1pcon->DefineSection(6, zpos, 0, C1wal1r4);
  geoC1wal1pcon->DefineSection(7, zpos, 0, C1wal1r5);
  zpos = zpos + C1wal1l6;
  geoC1wal1pcon->DefineSection(8, zpos, 0, C1wal1r5);
  geoC1wal1pcon->DefineSection(9, zpos, 0, C1wal1r6);
  zpos = zpos + C1wal1l7;
  geoC1wal1pcon->DefineSection(10, zpos, 0, C1wal1r6);
  geoC1wal1pcon->SetName("geoC1wal1pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC1wal1 = new TGeoCompositeShape("geoC1wal1name", "geoC1wal1pconname - geoA1wal1name - geoB1wal1name");
  TGeoVolume *volC1wal1 = new TGeoVolume("volC1wal1name", geoC1wal1, strMedC1wal1);

  //-   put volume
  volC1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volC1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C2spc1

  //get parameters from .xml file
  double C2spc1d1 = content.getParamLength("C2spc1/d1");
  //
  double C2spc1l1 = content.getParamLength("C2spc1/l1");
  double C2spc1l2 = content.getParamLength("C2spc1/l2");
  double C2spc1l3 = content.getParamLength("C2spc1/l3");
  double C2spc1l4 = content.getParamLength("C2spc1/l4");
  double C2spc1l5 = content.getParamLength("C2spc1/l5");
  double C2spc1l6 = content.getParamLength("C2spc1/l6");
  double C2spc1l7 = content.getParamLength("C2spc1/l7");
  double C2spc1l8 = content.getParamLength("C2spc1/l8");
  //
  double C2spc1r1 = content.getParamLength("C2spc1/r1");
  double C2spc1r2 = content.getParamLength("C2spc1/r2");
  double C2spc1r3 = content.getParamLength("C2spc1/r3");
  double C2spc1r4 = content.getParamLength("C2spc1/r4");
  double C2spc1r5 = content.getParamLength("C2spc1/r5");
  double C2spc1r6 = content.getParamLength("C2spc1/r6");
  string strMatC2spc1 = content.getParamString("C2spc1/Material");
  TGeoMedium* strMedC2spc1 = gGeoManager->GetMedium(strMatC2spc1.c_str());

  //define geometry
  TGeoPcon* geoC2spc1pcon = new TGeoPcon(0, 360, 9);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1;
  geoC2spc1pcon->DefineSection(0, zpos, 0, C2spc1r1);
  zpos = zpos + C2spc1l1;
  geoC2spc1pcon->DefineSection(1, zpos, 0, C2spc1r2);
  zpos = zpos + C2spc1l2;
  geoC2spc1pcon->DefineSection(2, zpos, 0, C2spc1r2);
  zpos = zpos + C2spc1l3;
  geoC2spc1pcon->DefineSection(3, zpos, 0, C2spc1r3);
  zpos = zpos + C2spc1l4 - C2spc1l5;
  geoC2spc1pcon->DefineSection(4, zpos, 0, C2spc1r3);
  geoC2spc1pcon->DefineSection(5, zpos, 0, C2spc1r5);
  zpos = zpos + C2spc1l6;
  geoC2spc1pcon->DefineSection(6, zpos, 0, C2spc1r5);
  zpos = zpos + C2spc1l7;
  geoC2spc1pcon->DefineSection(7, zpos, 0, C2spc1r6);
  zpos = zpos + C2spc1l8;
  geoC2spc1pcon->DefineSection(8, zpos, 0, C2spc1r6);
  geoC2spc1pcon->SetName("geoC2spc1pconname");

  //-   Subtraction volume
  TGeoPcon* geoC2spc1tub = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C2spc1l1 + C2spc1l2 + C2spc1l3 + C2spc1l4 - C2spc1l5;
  geoC2spc1tub->DefineSection(0, zpos, C2spc1r3, C2spc1r4);
  zpos = zpos + C2spc1l5;
  geoC2spc1tub->DefineSection(1, zpos, C2spc1r3, C2spc1r4);
  geoC2spc1tub->SetName("geoC2spc1tubname");

  //TGeoCompositeShape* geoC2spc1 = new TGeoCompositeShape("geoC2spc1name","geoC2spc1pconname - geoC2spc1tubname - geoA1wal1name - geoB1wal1name");
  TGeoCompositeShape* geoC2spc1 = new TGeoCompositeShape("geoC2spc1name", "(geoC2spc1pconname - geoC2spc1tubname) * geoC1wal1name");
  TGeoVolume *volC2spc1 = new TGeoVolume("volC2spc1name", geoC2spc1, strMedC2spc1);

  //-   put volume
  volC2spc1->SetLineColor(kGray + 1);
  volC1wal1->AddNode(volC2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C3wal2

  //get parameters from .xml file
  double C3wal2d1 = content.getParamLength("C3wal2/d1");
  double C3wal2l1 = content.getParamLength("C3wal2/l1");
  double C3wal2l2 = content.getParamLength("C3wal2/l2");
  double C3wal2l3 = content.getParamLength("C3wal2/l3");
  double C3wal2l4 = content.getParamLength("C3wal2/l4");
  double C3wal2l5 = content.getParamLength("C3wal2/l5");
  double C3wal2r1 = content.getParamLength("C3wal2/r1");
  double C3wal2r2 = content.getParamLength("C3wal2/r2");
  double C3wal2r3 = content.getParamLength("C3wal2/r3");
  double C3wal2r4 = content.getParamLength("C3wal2/r4");
  string strMatC3wal2 = content.getParamString("C3wal2/Material");
  TGeoMedium* strMedC3wal2 = gGeoManager->GetMedium(strMatC3wal2.c_str());

  //define geometry
  TGeoPcon* geoC3wal2pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1;
  geoC3wal2pcon->DefineSection(0, zpos, 0, C3wal2r1);
  zpos = zpos + C3wal2l1;
  geoC3wal2pcon->DefineSection(1, zpos, 0, C3wal2r2);
  zpos = zpos + C3wal2l2;
  geoC3wal2pcon->DefineSection(2, zpos, 0, C3wal2r2);
  zpos = zpos + C3wal2l3;
  geoC3wal2pcon->DefineSection(3, zpos, 0, C3wal2r3);
  zpos = zpos + C3wal2l4;
  geoC3wal2pcon->DefineSection(4, zpos, 0, C3wal2r3);
  geoC3wal2pcon->DefineSection(5, zpos, 0, C3wal2r4);
  zpos = zpos + C3wal2l5;
  geoC3wal2pcon->DefineSection(6, zpos, 0, C3wal2r4);
  geoC3wal2pcon->SetName("geoC3wal2pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC3wal2 = new TGeoCompositeShape("geoC3wal2name", "geoC3wal2pconname - geoA1wal1name - geoB1wal1name");
  TGeoVolume *volC3wal2 = new TGeoVolume("volC3wal2name", geoC3wal2, strMedC3wal2);

  //-   put volume
  volC3wal2->SetLineColor(kGray + 3);
  volC2spc1->AddNode(volC3wal2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C4spc2

  //get parameters from .xml file
  double C4spc2d1 = content.getParamLength("C4spc2/d1");
  double C4spc2l1 = content.getParamLength("C4spc2/l1");
  double C4spc2l2 = content.getParamLength("C4spc2/l2");
  double C4spc2l3 = content.getParamLength("C4spc2/l3");
  double C4spc2l4 = content.getParamLength("C4spc2/l4");
  double C4spc2l5 = content.getParamLength("C4spc2/l5");
  double C4spc2r1 = content.getParamLength("C4spc2/r1");
  double C4spc2r2 = content.getParamLength("C4spc2/r2");
  double C4spc2r3 = content.getParamLength("C4spc2/r3");
  double C4spc2r4 = content.getParamLength("C4spc2/r4");
  string strMatC4spc2 = content.getParamString("C4spc2/Material");
  TGeoMedium* strMedC4spc2 = gGeoManager->GetMedium(strMatC4spc2.c_str());

  //define geometry
  TGeoPcon* geoC4spc2pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1;
  geoC4spc2pcon->DefineSection(0, zpos, 0, C4spc2r1);
  zpos = zpos + C4spc2l1;
  geoC4spc2pcon->DefineSection(1, zpos, 0, C4spc2r2);
  zpos = zpos + C4spc2l2;
  geoC4spc2pcon->DefineSection(2, zpos, 0, C4spc2r2);
  zpos = zpos + C4spc2l3;
  geoC4spc2pcon->DefineSection(3, zpos, 0, C4spc2r3);
  zpos = zpos + C4spc2l4;
  geoC4spc2pcon->DefineSection(4, zpos, 0, C4spc2r3);
  geoC4spc2pcon->DefineSection(5, zpos, 0, C4spc2r4);
  zpos = zpos + C4spc2l5;
  geoC4spc2pcon->DefineSection(6, zpos, 0, C4spc2r4);
  geoC4spc2pcon->SetName("geoC4spc2pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC4spc2 = new TGeoCompositeShape("geoC4spc2name", "geoC4spc2pconname - geoA1wal1name - geoB1wal1name");
  TGeoVolume *volC4spc2 = new TGeoVolume("volC4spc2name", geoC4spc2, strMedC4spc2);

  //-   put volume
  volC4spc2->SetLineColor(kGray + 1);
  volC3wal2->AddNode(volC4spc2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C5wal3

  //get parameters from .xml file
  double C5wal3d1 = content.getParamLength("C5wal3/d1");
  double C5wal3l1 = content.getParamLength("C5wal3/l1");
  double C5wal3l2 = content.getParamLength("C5wal3/l2");
  double C5wal3l3 = content.getParamLength("C5wal3/l3");
  double C5wal3l4 = content.getParamLength("C5wal3/l4");
  double C5wal3l5 = content.getParamLength("C5wal3/l5");
  double C5wal3l6 = content.getParamLength("C5wal3/l6");
  double C5wal3r1 = content.getParamLength("C5wal3/r1");
  double C5wal3r2 = content.getParamLength("C5wal3/r2");
  double C5wal3r3 = content.getParamLength("C5wal3/r3");
  double C5wal3r4 = content.getParamLength("C5wal3/r4");
  double C5wal3r5 = content.getParamLength("C5wal3/r5");
  string strMatC5wal3 = content.getParamString("C5wal3/Material");
  TGeoMedium* strMedC5wal3 = gGeoManager->GetMedium(strMatC5wal3.c_str());

  //define geometry
  TGeoPcon* geoC5wal3pcon = new TGeoPcon(0, 360, 8);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal3d1;
  geoC5wal3pcon->DefineSection(0, zpos, 0, C5wal3r1);
  zpos = zpos + C5wal3l1;
  geoC5wal3pcon->DefineSection(1, zpos, 0, C5wal3r2);
  zpos = zpos + C5wal3l2;
  geoC5wal3pcon->DefineSection(2, zpos, 0, C5wal3r2);
  geoC5wal3pcon->DefineSection(3, zpos, 0, C5wal3r3);
  zpos = zpos + C5wal3l3;
  geoC5wal3pcon->DefineSection(4, zpos, 0, C5wal3r3);
  zpos = zpos + C5wal3l4;
  geoC5wal3pcon->DefineSection(5, zpos, 0, C5wal3r4);
  zpos = zpos + C5wal3l5;
  geoC5wal3pcon->DefineSection(6, zpos, 0, C5wal3r4);
  zpos = zpos + C5wal3l6;
  geoC5wal3pcon->DefineSection(7, zpos, 0, C5wal3r5);
  geoC5wal3pcon->SetName("geoC5wal3pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC5wal3 = new TGeoCompositeShape("geoC5wal3name", "geoC5wal3pconname - geoA1wal1name - geoB1wal1name");
  TGeoVolume *volC5wal3 = new TGeoVolume("volC5wal3name", geoC5wal3, strMedC5wal3);

  //-   put volume
  volC5wal3->SetLineColor(kGray + 3);
  volC4spc2->AddNode(volC5wal3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C5wal5

  //get parameters from .xml file
  double C5wal5d1 = content.getParamLength("C5wal5/d1");
  double C5wal5l1 = content.getParamLength("C5wal5/l1");
  double C5wal5l2 = content.getParamLength("C5wal5/l2");
  double C5wal5r1 = content.getParamLength("C5wal5/r1");
  double C5wal5r2 = content.getParamLength("C5wal5/r2");
  string strMatC5wal5 = content.getParamString("C5wal5/Material");
  TGeoMedium* strMedC5wal5 = gGeoManager->GetMedium(strMatC5wal5.c_str());

  //define geometry
  TGeoPcon* geoC5wal5pcon = new TGeoPcon(0, 360, 3);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal5d1;
  geoC5wal5pcon->DefineSection(0, zpos, 0, C5wal5r1);
  zpos = zpos + C5wal5l1;
  geoC5wal5pcon->DefineSection(1, zpos, 0, C5wal5r1);
  zpos = zpos + C5wal5l2;
  geoC5wal5pcon->DefineSection(2, zpos, 0, C5wal5r2);
  geoC5wal5pcon->SetName("geoC5wal5pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC5wal5 = new TGeoCompositeShape("geoC5wal5name", "geoC5wal5pconname - geoA1wal1name - geoB1wal1name");
  TGeoVolume *volC5wal5 = new TGeoVolume("volC5wal5name", geoC5wal5, strMedC5wal5);

  //-   put volume
  volC5wal5->SetLineColor(kGray + 3);
  volC4spc2->AddNode(volC5wal5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc4

  //get parameters from .xml file
  double C6spc4l1 = content.getParamLength("C6spc4/l1");
  double C6spc4l2 = content.getParamLength("C6spc4/l2");
  double C6spc4l3 = content.getParamLength("C6spc4/l3");
  double C6spc4l4 = content.getParamLength("C6spc4/l4");
  double C6spc4l5 = content.getParamLength("C6spc4/l5");
  //
  double C6spc4r1 = content.getParamLength("C6spc4/r1");
  double C6spc4r2 = content.getParamLength("C6spc4/r2");
  double C6spc4r3 = content.getParamLength("C6spc4/r3");
  string strMatC6spc4 = content.getParamString("C6spc4/Material");
  TGeoMedium* strMedC6spc4 = gGeoManager->GetMedium(strMatC6spc4.c_str());

  //define geometry
  TGeoPcon* geoC6spc4pcon = new TGeoPcon(0, 360, 6);
  zpos = 0.0;
  geoC6spc4pcon->DefineSection(0, zpos, 0, C6spc4r1);
  zpos = zpos + C6spc4l1;
  geoC6spc4pcon->DefineSection(1, zpos, 0, C6spc4r1);
  zpos = zpos + C6spc4l2;
  geoC6spc4pcon->DefineSection(2, zpos, 0, C6spc4r2);
  zpos = zpos + C6spc4l3;
  geoC6spc4pcon->DefineSection(3, zpos, 0, C6spc4r2);
  zpos = zpos + C6spc4l4;
  geoC6spc4pcon->DefineSection(4, zpos, 0, C6spc4r3);
  zpos = zpos + C6spc4l5;
  geoC6spc4pcon->DefineSection(5, zpos, 0, C6spc4r3);
  geoC6spc4pcon->SetName("geoC6spc4pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC6spc4 = new TGeoCompositeShape("geoC6spc4name", "(geoC6spc4pconname:rotHERname * geoC5wal3name) - geoA1wal1name");
  TGeoVolume *volC6spc4 = new TGeoVolume("volC6spc4name", geoC6spc4, strMedC6spc4);

  //-   put volume
  volC6spc4->SetLineColor(kGray + 1);
  volC5wal3->AddNode(volC6spc4, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc5

  //get parameters from .xml file
  double C6spc5l1 = content.getParamLength("C6spc5/l1");
  double C6spc5l2 = content.getParamLength("C6spc5/l2");
  double C6spc5l3 = content.getParamLength("C6spc5/l3");
  double C6spc5l4 = content.getParamLength("C6spc5/l4");
  double C6spc5l5 = content.getParamLength("C6spc5/l5");
  double C6spc5l6 = content.getParamLength("C6spc5/l6");
  double C6spc5l7 = content.getParamLength("C6spc5/l7");
  //
  double C6spc5r1 = content.getParamLength("C6spc5/r1");
  double C6spc5r2 = content.getParamLength("C6spc5/r2");
  double C6spc5r3 = content.getParamLength("C6spc5/r3");
  double C6spc5r4 = content.getParamLength("C6spc5/r4");
  double C6spc5r5 = content.getParamLength("C6spc5/r5");
  string strMatC6spc5 = content.getParamString("C6spc5/Material");
  TGeoMedium* strMedC6spc5 = gGeoManager->GetMedium(strMatC6spc5.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoC6spc5pcon = new TGeoPcon(0, 360, 8);
  geoC6spc5pcon->DefineSection(0, zpos, 0, C6spc5r1);
  zpos = zpos + C6spc5l1;
  geoC6spc5pcon->DefineSection(1, zpos, 0, C6spc5r1);
  zpos = zpos + C6spc5l2;
  geoC6spc5pcon->DefineSection(2, zpos, 0, C6spc5r2);
  zpos = zpos + C6spc5l3;
  geoC6spc5pcon->DefineSection(3, zpos, 0, C6spc5r3);
  zpos = zpos + C6spc5l4;
  geoC6spc5pcon->DefineSection(4, zpos, 0, C6spc5r3);
  zpos = zpos + C6spc5l5;
  geoC6spc5pcon->DefineSection(5, zpos, 0, C6spc5r4);
  zpos = zpos + C6spc5l6;
  geoC6spc5pcon->DefineSection(6, zpos, 0, C6spc5r5);
  zpos = zpos + C6spc5l7;
  geoC6spc5pcon->DefineSection(7, zpos, 0, C6spc5r5);
  geoC6spc5pcon->SetName("geoC6spc5pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC6spc5 = new TGeoCompositeShape("geoC6spc5name", "(geoC6spc5pconname:rotLERname * geoC5wal3name) - geoB1wal1name");
  TGeoVolume *volC6spc5 = new TGeoVolume("volC6spc5name", geoC6spc5, strMedC6spc5);

  //-   put volume
  volC6spc5->SetLineColor(kGray + 1);
  volC5wal3->AddNode(volC6spc5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc7

  //get parameters from .xml file
  double C6spc7d1 = content.getParamLength("C6spc7/d1");
  double C6spc7l1 = content.getParamLength("C6spc7/l1");
  double C6spc7r1 = content.getParamLength("C6spc7/r1");
  string strMatC6spc7 = content.getParamString("C6spc7/Material");
  TGeoMedium* strMedC6spc7 = gGeoManager->GetMedium(strMatC6spc7.c_str());

  //define geometry
  TGeoPcon* geoC6spc7pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C6spc7d1;
  geoC6spc7pcon->DefineSection(0, zpos, 0, C6spc7r1);
  zpos = zpos + C6spc7l1;
  geoC6spc7pcon->DefineSection(1, zpos, 0, C6spc7r1);
  geoC6spc7pcon->SetName("geoC6spc7pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC6spc7 = new TGeoCompositeShape("geoC6spc7name", "(geoC6spc7pconname:rotHERname * geoC5wal5name) - geoA1wal1name");
  TGeoVolume *volC6spc7 = new TGeoVolume("volC6spc7name", geoC6spc7, strMedC6spc7);

  //-   put volume
  volC6spc7->SetLineColor(kGray + 1);
  volC5wal5->AddNode(volC6spc7, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc8

  //get parameters from .xml file
  double C6spc8d1 = content.getParamLength("C6spc8/d1");
  double C6spc8l1 = content.getParamLength("C6spc8/l1");
  double C6spc8r1 = content.getParamLength("C6spc8/r1");
  string strMatC6spc8 = content.getParamString("C6spc8/Material");
  TGeoMedium* strMedC6spc8 = gGeoManager->GetMedium(strMatC6spc8.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoC6spc8pcon = new TGeoPcon(0, 360, 2);
  zpos = zpos + C6spc8d1;
  geoC6spc8pcon->DefineSection(0, zpos, 0, C6spc8r1);
  zpos = zpos + C6spc8l1;
  geoC6spc8pcon->DefineSection(1, zpos, 0, C6spc8r1);
  geoC6spc8pcon->SetName("geoC6spc8pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC6spc8 = new TGeoCompositeShape("geoC6spc8name", "(geoC6spc8pconname:rotLERname * geoC5wal5name) - geoB1wal1name");
  TGeoVolume *volC6spc8 = new TGeoVolume("volC6spc8name", geoC6spc8, strMedC6spc8);

  //-   put volume
  volC6spc8->SetLineColor(kGray + 1);
  volC5wal5->AddNode(volC6spc8, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc3

  //--- C6tnl1 ---
  //get parameters from .xml file
  double C6tnl1r1 = content.getParamLength("C6tnl1/r1");
  double C6tnl1r2 = content.getParamLength("C6tnl1/r2");
  double C6tnl1r3 = content.getParamLength("C6tnl1/r3");
  //define geometry
  TGeoPcon* geoC6tnl1pcon = new TGeoPcon(0, 360, 6);
  zpos = 0.0;
  geoC6tnl1pcon->DefineSection(0, zpos, 0, C6tnl1r1);
  zpos = zpos + C6spc4l1;
  geoC6tnl1pcon->DefineSection(1, zpos, 0, C6tnl1r1);
  zpos = zpos + C6spc4l2;
  geoC6tnl1pcon->DefineSection(2, zpos, 0, C6tnl1r2);
  zpos = zpos + C6spc4l3;
  geoC6tnl1pcon->DefineSection(3, zpos, 0, C6tnl1r2);
  zpos = zpos + C6spc4l4;
  geoC6tnl1pcon->DefineSection(4, zpos, 0, C6tnl1r3);
  zpos = zpos + C6spc4l5;
  geoC6tnl1pcon->DefineSection(5, zpos, 0, C6tnl1r3);
  geoC6tnl1pcon->SetName("geoC6tnl1pconname");

  //--- C6tnl2 ---
  //get parameters from .xml file
  double C6tnl2r1 = content.getParamLength("C6tnl2/r1");
  double C6tnl2r2 = content.getParamLength("C6tnl2/r2");
  double C6tnl2r3 = content.getParamLength("C6tnl2/r3");
  double C6tnl2r4 = content.getParamLength("C6tnl2/r4");
  double C6tnl2r5 = content.getParamLength("C6tnl2/r5");
  //define geometry
  zpos = 0.0;
  TGeoPcon* geoC6tnl2pcon = new TGeoPcon(0, 360, 8);
  geoC6tnl2pcon->DefineSection(0, zpos, 0, C6tnl2r1);
  zpos = zpos + C6spc5l1;
  geoC6tnl2pcon->DefineSection(1, zpos, 0, C6tnl2r1);
  zpos = zpos + C6spc5l2;
  geoC6tnl2pcon->DefineSection(2, zpos, 0, C6tnl2r2);
  zpos = zpos + C6spc5l3;
  geoC6tnl2pcon->DefineSection(3, zpos, 0, C6tnl2r3);
  zpos = zpos + C6spc5l4;
  geoC6tnl2pcon->DefineSection(4, zpos, 0, C6tnl2r3);
  zpos = zpos + C6spc5l5;
  geoC6tnl2pcon->DefineSection(5, zpos, 0, C6tnl2r4);
  zpos = zpos + C6spc5l6;
  geoC6tnl2pcon->DefineSection(6, zpos, 0, C6tnl2r5);
  zpos = zpos + C6spc5l7;
  geoC6tnl2pcon->DefineSection(7, zpos, 0, C6tnl2r5);
  geoC6tnl2pcon->SetName("geoC6tnl2pconname");

  //get parameters from .xml file
  double C6spc3d1 = content.getParamLength("C6spc3/d1");
  double C6spc3l1 = content.getParamLength("C6spc3/l1");
  double C6spc3l2 = content.getParamLength("C6spc3/l2");
  double C6spc3l3 = content.getParamLength("C6spc3/l3");
  double C6spc3l4 = content.getParamLength("C6spc3/l4");
  double C6spc3l5 = content.getParamLength("C6spc3/l5");
  double C6spc3r1 = content.getParamLength("C6spc3/r1");
  double C6spc3r2 = content.getParamLength("C6spc3/r2");
  double C6spc3r3 = content.getParamLength("C6spc3/r3");
  double C6spc3r4 = content.getParamLength("C6spc3/r4");
  string strMatC6spc3 = content.getParamString("C6spc3/Material");
  TGeoMedium* strMedC6spc3 = gGeoManager->GetMedium(strMatC6spc3.c_str());

  //define geometry
  TGeoPcon* geoC6spc3pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal3d1 + C6spc3d1;
  geoC6spc3pcon->DefineSection(0, zpos, 0, C6spc3r1);
  zpos = zpos + C6spc3l1;
  geoC6spc3pcon->DefineSection(1, zpos, 0, C6spc3r2);
  zpos = zpos + C6spc3l2;
  geoC6spc3pcon->DefineSection(2, zpos, 0, C6spc3r2);
  geoC6spc3pcon->DefineSection(3, zpos, 0, C6spc3r3);
  zpos = zpos + C6spc3l3;
  geoC6spc3pcon->DefineSection(4, zpos, 0, C6spc3r3);
  zpos = zpos + C6spc3l4;
  geoC6spc3pcon->DefineSection(5, zpos, 0, C6spc3r4);
  zpos = zpos + C6spc3l5;
  geoC6spc3pcon->DefineSection(6, zpos, 0, C6spc3r4);
  geoC6spc3pcon->SetName("geoC6spc3pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC6spc3 = new TGeoCompositeShape("geoC6spc3name", "geoC6spc3pconname - geoC6tnl1pconname:rotHERname - geoC6tnl2pconname:rotLERname");
  TGeoVolume *volC6spc3 = new TGeoVolume("volC6spc3name", geoC6spc3, strMedC6spc3);

  //-   put volume
  volC6spc3->SetLineColor(kGray + 1);
  volC5wal3->AddNode(volC6spc3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C6spc6

  //--- C6tnl3 ---
  //get parameters from .xml file
  double C6tnl3r1 = content.getParamLength("C6tnl3/r1");
  //define geometry
  TGeoPcon* geoC6tnl3pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C6spc7d1;
  geoC6tnl3pcon->DefineSection(0, zpos, 0, C6tnl3r1);
  zpos = zpos + C6spc7l1;
  geoC6tnl3pcon->DefineSection(1, zpos, 0, C6tnl3r1);
  geoC6tnl3pcon->SetName("geoC6tnl3pconname");

  //--- C6tnl4 ---
  //get parameters from .xml file
  double C6tnl4r1 = content.getParamLength("C6tnl4/r1");
  //define geometry
  zpos = 0.0;
  TGeoPcon* geoC6tnl4pcon = new TGeoPcon(0, 360, 2);
  zpos = zpos + C6spc8d1;
  geoC6tnl4pcon->DefineSection(0, zpos, 0, C6tnl4r1);
  zpos = zpos + C6spc8l1;
  geoC6tnl4pcon->DefineSection(1, zpos, 0, C6tnl4r1);
  geoC6tnl4pcon->SetName("geoC6tnl4pconname");

  //get parameters from .xml file
  double C6spc6d1 = content.getParamLength("C6spc6/d1");
  double C6spc6l1 = content.getParamLength("C6spc6/l1");
  double C6spc6l2 = content.getParamLength("C6spc6/l2");
  double C6spc6r1 = content.getParamLength("C6spc6/r1");
  double C6spc6r2 = content.getParamLength("C6spc6/r2");
  string strMatC6spc6 = content.getParamString("C6spc6/Material");
  TGeoMedium* strMedC6spc6 = gGeoManager->GetMedium(strMatC6spc6.c_str());

  //define geometry
  TGeoPcon* geoC6spc6pcon = new TGeoPcon(0, 360, 3);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal5d1 + C6spc6d1;
  geoC6spc6pcon->DefineSection(0, zpos, 0, C6spc6r1);
  zpos = zpos + C6spc6l1;
  geoC6spc6pcon->DefineSection(1, zpos, 0, C6spc6r1);
  zpos = zpos + C6spc6l2;
  geoC6spc6pcon->DefineSection(2, zpos, 0, C6spc6r2);
  geoC6spc6pcon->SetName("geoC6spc6pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoC6spc6 = new TGeoCompositeShape("geoC6spc6name", "geoC6spc6pconname - geoC6tnl3pconname:rotHERname - geoC6tnl4pconname:rotLERname");
  TGeoVolume *volC6spc6 = new TGeoVolume("volC6spc6name", geoC6spc6, strMedC6spc6);

  //-   put volume
  volC6spc6->SetLineColor(kGray + 1);
  volC5wal5->AddNode(volC6spc6, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag1

  //get parameters from .xml file
  double C7mag1d1 = content.getParamLength("C7mag1/d1");
  double C7mag1l1 = content.getParamLength("C7mag1/l1");
  double C7mag1o1 = content.getParamLength("C7mag1/o1");
  string strMatC7mag1 = content.getParamString("C7mag1/Material");
  TGeoMedium* strMedC7mag1 = gGeoManager->GetMedium(strMatC7mag1.c_str());

  //define geometry
  TGeoPcon* geoC7mag1pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag1d1;
  geoC7mag1pcon->DefineSection(0, zpos, 0, C7mag1o1);
  zpos = zpos + C7mag1l1;
  geoC7mag1pcon->DefineSection(1, zpos, 0, C7mag1o1);
  geoC7mag1pcon->SetName("geoC7mag1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag1 = new TGeoCompositeShape("geoC7mag1name", "geoC7mag1pconname:rotHERname * geoC6spc3name");
  TGeoVolume *volC7mag1 = new TGeoVolume("volC7mag1name", geoC7mag1, strMedC7mag1);

  //-   put volume
  volC7mag1->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag2

  //get parameters from .xml file
  double C7mag2d1 = C6spc4l1 + C6spc4l2;
  double C7mag2l1 = content.getParamLength("C7mag2/l1");
  double C7mag2o1 = content.getParamLength("C7mag2/o1");
  string strMatC7mag2 = content.getParamString("C7mag2/Material");
  TGeoMedium* strMedC7mag2 = gGeoManager->GetMedium(strMatC7mag2.c_str());

  //define geometry
  TGeoPcon* geoC7mag2pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag2d1;
  geoC7mag2pcon->DefineSection(0, zpos, 0, C7mag2o1);
  zpos = zpos + C7mag2l1;
  geoC7mag2pcon->DefineSection(1, zpos, 0, C7mag2o1);
  geoC7mag2pcon->SetName("geoC7mag2pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag2 = new TGeoCompositeShape("geoC7mag2name", "geoC7mag2pconname:rotHERname * geoC6spc3name");
  TGeoVolume *volC7mag2 = new TGeoVolume("volC7mag2name", geoC7mag2, strMedC7mag2);

  //-   put volume
  volC7mag2->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag3

  //get parameters from .xml file
  double C7mag3d1 = C6spc4l1 + C6spc4l2 + C6spc4l3 + C6spc4l4;
  double C7mag3l1 = content.getParamLength("C7mag3/l1");
  double C7mag3o1 = content.getParamLength("C7mag3/o1");
  string strMatC7mag3 = content.getParamString("C7mag3/Material");
  TGeoMedium* strMedC7mag3 = gGeoManager->GetMedium(strMatC7mag3.c_str());

  //define geometry
  TGeoPcon* geoC7mag3pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag3d1;
  geoC7mag3pcon->DefineSection(0, zpos, 0, C7mag3o1);
  zpos = zpos + C7mag3l1;
  geoC7mag3pcon->DefineSection(1, zpos, 0, C7mag3o1);
  geoC7mag3pcon->SetName("geoC7mag3pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag3 = new TGeoCompositeShape("geoC7mag3name", "geoC7mag3pconname:rotHERname * geoC6spc3name");
  TGeoVolume *volC7mag3 = new TGeoVolume("volC7mag3name", geoC7mag3, strMedC7mag3);

  //-   put volume
  volC7mag3->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag4

  //get parameters from .xml file
  double C7mag4d1 = content.getParamLength("C7mag4/d1");
  double C7mag4l1 = content.getParamLength("C7mag4/l1");
  double C7mag4o1 = content.getParamLength("C7mag4/o1");
  string strMatC7mag4 = content.getParamString("C7mag4/Material");
  TGeoMedium* strMedC7mag4 = gGeoManager->GetMedium(strMatC7mag4.c_str());

  //define geometry
  TGeoPcon* geoC7mag4pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag4d1;
  geoC7mag4pcon->DefineSection(0, zpos, 0, C7mag4o1);
  zpos = zpos + C7mag4l1;
  geoC7mag4pcon->DefineSection(1, zpos, 0, C7mag4o1);
  geoC7mag4pcon->SetName("geoC7mag4pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag4 = new TGeoCompositeShape("geoC7mag4name", "geoC7mag4pconname:rotLERname * geoC6spc3name");
  TGeoVolume *volC7mag4 = new TGeoVolume("volC7mag4name", geoC7mag4, strMedC7mag4);

  //-   put volume
  volC7mag4->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag4, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag5

  //get parameters from .xml file
  double C7mag5d1 = C6spc5l1 + C6spc5l2 + C6spc5l3;
  double C7mag5l1 = content.getParamLength("C7mag5/l1");
  double C7mag5o1 = content.getParamLength("C7mag5/o1");
  string strMatC7mag5 = content.getParamString("C7mag5/Material");
  TGeoMedium* strMedC7mag5 = gGeoManager->GetMedium(strMatC7mag5.c_str());

  //define geometry
  TGeoPcon* geoC7mag5pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag5d1;
  geoC7mag5pcon->DefineSection(0, zpos, 0, C7mag5o1);
  zpos = zpos + C7mag5l1;
  geoC7mag5pcon->DefineSection(1, zpos, 0, C7mag5o1);
  geoC7mag5pcon->SetName("geoC7mag5pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag5 = new TGeoCompositeShape("geoC7mag5name", "geoC7mag5pconname:rotLERname * geoC6spc3name");
  TGeoVolume *volC7mag5 = new TGeoVolume("volC7mag5name", geoC7mag5, strMedC7mag5);

  //-   put volume
  volC7mag5->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag6

  //get parameters from .xml file
  double C7mag6d1 = C6spc5l1 + C6spc5l2 + C6spc5l3 + C6spc5l4 + C6spc5l5 + C6spc5l6;
  double C7mag6l1 = content.getParamLength("C7mag6/l1");
  double C7mag6o1 = content.getParamLength("C7mag6/o1");
  string strMatC7mag6 = content.getParamString("C7mag6/Material");
  TGeoMedium* strMedC7mag6 = gGeoManager->GetMedium(strMatC7mag6.c_str());

  //define geometry
  TGeoPcon* geoC7mag6pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag6d1;
  geoC7mag6pcon->DefineSection(0, zpos, 0, C7mag6o1);
  zpos = zpos + C7mag6l1;
  geoC7mag6pcon->DefineSection(1, zpos, 0, C7mag6o1);
  geoC7mag6pcon->SetName("geoC7mag6pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag6 = new TGeoCompositeShape("geoC7mag6name", "geoC7mag6pconname:rotLERname * geoC6spc3name");
  TGeoVolume *volC7mag6 = new TGeoVolume("volC7mag6name", geoC7mag6, strMedC7mag6);

  //-   put volume
  volC7mag6->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7mag6, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7mag7

  //get parameters from .xml file
  double C7mag7d1 = A1wal1l1 + A1wal1l2 + A1wal1l3 + A1wal1l4 + A1wal1l5 + A1wal1l6 + 7.773;
  double C7mag7l1 = content.getParamLength("C7mag7/l1");
  double C7mag7o1 = content.getParamLength("C7mag7/o1");
  string strMatC7mag7 = content.getParamString("C7mag7/Material");
  TGeoMedium* strMedC7mag7 = gGeoManager->GetMedium(strMatC7mag7.c_str());

  //define geometry
  TGeoPcon* geoC7mag7pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + C7mag7d1;
  geoC7mag7pcon->DefineSection(0, zpos, 0, C7mag7o1);
  zpos = zpos + C7mag7l1;
  geoC7mag7pcon->DefineSection(1, zpos, 0, C7mag7o1);
  geoC7mag7pcon->SetName("geoC7mag7pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoC7mag7 = new TGeoCompositeShape("geoC7mag7name", "geoC7mag7pconname:rotHERname * geoC6spc6name");
  TGeoVolume *volC7mag7 = new TGeoVolume("volC7mag7name", geoC7mag7, strMedC7mag7);

  //-   put volume
  volC7mag7->SetLineColor(kOrange);
  volC6spc6->AddNode(volC7mag7, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7hld1

  //get parameters from .xml file
  double C7hld1t[16], C7hld1d[15], C7hld1i[15], C7hld1o[16], C7hld1r[15];
  C7hld1t[0] = content.getParamLength("C7hld1/t1");
  C7hld1t[1] = content.getParamLength("C7hld1/t2");
  C7hld1t[2] = content.getParamLength("C7hld1/t3");
  C7hld1t[3] = content.getParamLength("C7hld1/t4");
  C7hld1t[4] = content.getParamLength("C7hld1/t5");
  C7hld1t[5] = content.getParamLength("C7hld1/t6");
  C7hld1t[6] = content.getParamLength("C7hld1/t7");
  C7hld1t[7] = content.getParamLength("C7hld1/t8");
  C7hld1t[8] = content.getParamLength("C7hld1/t9");
  C7hld1t[9] = content.getParamLength("C7hld1/t10");
  C7hld1t[10] = content.getParamLength("C7hld1/t11");
  C7hld1t[11] = content.getParamLength("C7hld1/t12");
  C7hld1t[12] = content.getParamLength("C7hld1/t13");
  C7hld1t[13] = content.getParamLength("C7hld1/t14");
  C7hld1t[14] = content.getParamLength("C7hld1/t15");
  C7hld1t[15] = content.getParamLength("C7hld1/t16");
  C7hld1d[0] = content.getParamLength("C7hld1/d1");
  C7hld1d[1] = content.getParamLength("C7hld1/d2");
  C7hld1d[2] = content.getParamLength("C7hld1/d3");
  C7hld1d[3] = content.getParamLength("C7hld1/d4");
  C7hld1d[4] = content.getParamLength("C7hld1/d5");
  C7hld1d[5] = content.getParamLength("C7hld1/d6");
  C7hld1d[6] = content.getParamLength("C7hld1/d7");
  C7hld1d[7] = content.getParamLength("C7hld1/d8");
  C7hld1d[8] = content.getParamLength("C7hld1/d9");
  C7hld1d[9] = content.getParamLength("C7hld1/d10");
  C7hld1d[10] = content.getParamLength("C7hld1/d11");
  C7hld1d[11] = content.getParamLength("C7hld1/d12");
  C7hld1d[12] = content.getParamLength("C7hld1/d13");
  C7hld1d[13] = content.getParamLength("C7hld1/d14");
  C7hld1d[14] = content.getParamLength("C7hld1/d15");
  C7hld1i[0] = content.getParamLength("C7hld1/i1");
  C7hld1i[1] = content.getParamLength("C7hld1/i2");
  C7hld1i[2] = content.getParamLength("C7hld1/i3");
  C7hld1i[3] = content.getParamLength("C7hld1/i4");
  C7hld1i[4] = content.getParamLength("C7hld1/i5");
  C7hld1i[5] = content.getParamLength("C7hld1/i6");
  C7hld1i[6] = content.getParamLength("C7hld1/i7");
  C7hld1i[7] = content.getParamLength("C7hld1/i8");
  C7hld1i[8] = content.getParamLength("C7hld1/i9");
  C7hld1i[9] = content.getParamLength("C7hld1/i10");
  C7hld1i[10] = content.getParamLength("C7hld1/i11");
  C7hld1i[11] = content.getParamLength("C7hld1/i12");
  C7hld1i[12] = content.getParamLength("C7hld1/i13");
  C7hld1i[13] = content.getParamLength("C7hld1/i14");
  C7hld1i[14] = content.getParamLength("C7hld1/i15");
  C7hld1r[0] = content.getParamLength("C7hld1/r1");
  C7hld1r[1] = content.getParamLength("C7hld1/r2");
  C7hld1r[2] = content.getParamLength("C7hld1/r3");
  C7hld1r[3] = content.getParamLength("C7hld1/r4");
  C7hld1r[4] = content.getParamLength("C7hld1/r5");
  C7hld1r[5] = content.getParamLength("C7hld1/r6");
  C7hld1r[6] = content.getParamLength("C7hld1/r7");
  C7hld1r[7] = content.getParamLength("C7hld1/r8");
  C7hld1r[8] = content.getParamLength("C7hld1/r9");
  C7hld1r[9] = content.getParamLength("C7hld1/r10");
  C7hld1r[10] = content.getParamLength("C7hld1/r11");
  C7hld1r[11] = content.getParamLength("C7hld1/r12");
  C7hld1r[12] = content.getParamLength("C7hld1/r13");
  C7hld1r[13] = content.getParamLength("C7hld1/r14");
  C7hld1r[14] = content.getParamLength("C7hld1/r15");
  C7hld1o[0] = content.getParamLength("C7hld1/o1");
  C7hld1o[1] = content.getParamLength("C7hld1/o2");
  C7hld1o[2] = content.getParamLength("C7hld1/o3");
  C7hld1o[3] = content.getParamLength("C7hld1/o4");
  C7hld1o[4] = content.getParamLength("C7hld1/o5");
  C7hld1o[5] = content.getParamLength("C7hld1/o6");
  C7hld1o[6] = content.getParamLength("C7hld1/o7");
  C7hld1o[7] = content.getParamLength("C7hld1/o8");
  C7hld1o[8] = content.getParamLength("C7hld1/o9");
  C7hld1o[9] = content.getParamLength("C7hld1/o10");
  C7hld1o[10] = content.getParamLength("C7hld1/o11");
  C7hld1o[11] = content.getParamLength("C7hld1/o12");
  C7hld1o[12] = content.getParamLength("C7hld1/o13");
  C7hld1o[13] = content.getParamLength("C7hld1/o14");
  C7hld1o[14] = content.getParamLength("C7hld1/o15");
  C7hld1o[15] = content.getParamLength("C7hld1/o16");
  string strMatC7hld1 = content.getParamString("C7hld1/Material");
  TGeoMedium* strMedC7hld1 = gGeoManager->GetMedium(strMatC7hld1.c_str());

  //define geometry
  TGeoPcon* geoC7hld1pcon = new TGeoPcon(0, 360, 62);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal3d1 + C6spc3d1;
  geoC7hld1pcon->DefineSection(0, zpos, C7hld1i[0], C7hld1o[0]);
  zpos = zpos + C7hld1t[0];
  geoC7hld1pcon->DefineSection(1, zpos, C7hld1i[0], C7hld1o[0]);
  count = 2;
  for (int n = 0; n <= 14; n++) {
    geoC7hld1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1r[n]); count = count + 1;
    zpos = zpos + C7hld1d[n];
    geoC7hld1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1r[n]); count = count + 1;
    geoC7hld1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1o[n+1]); count = count + 1;
    zpos = zpos + C7hld1t[n+1];
    geoC7hld1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1o[n+1]); count = count + 1;
  }
  geoC7hld1pcon->SetName("geoC7hld1pconname");

  //-   Intersection and Subtraction
  TGeoCompositeShape* geoC7hld1 = new TGeoCompositeShape("geoC7hld1name", "(geoC7hld1pconname * geoC6spc3name) - geoC7mag6name");
  TGeoVolume *volC7hld1 = new TGeoVolume("volC7hld1name", geoC7hld1, strMedC7hld1);

  //-   put volume
  volC7hld1->SetLineColor(kGray + 3);
  volC6spc3->AddNode(volC7hld1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   C7cil1

  //get parameters from .xml file
  double C7cil1l[15];
  C7cil1l[0] = content.getParamLength("C7cil1/l1");
  C7cil1l[1] = content.getParamLength("C7cil1/l2");
  C7cil1l[2] = content.getParamLength("C7cil1/l3");
  C7cil1l[3] = content.getParamLength("C7cil1/l4");
  C7cil1l[4] = content.getParamLength("C7cil1/l5");
  C7cil1l[5] = content.getParamLength("C7cil1/l6");
  C7cil1l[6] = content.getParamLength("C7cil1/l7");
  C7cil1l[7] = content.getParamLength("C7cil1/l8");
  C7cil1l[8] = content.getParamLength("C7cil1/l9");
  C7cil1l[9] = content.getParamLength("C7cil1/l10");
  C7cil1l[10] = content.getParamLength("C7cil1/l11");
  C7cil1l[11] = content.getParamLength("C7cil1/l12");
  C7cil1l[12] = content.getParamLength("C7cil1/l13");
  C7cil1l[13] = content.getParamLength("C7cil1/l14");
  C7cil1l[14] = content.getParamLength("C7cil1/l15");
  string strMatC7cil1 = content.getParamString("C7cil1/Material");
  TGeoMedium* strMedC7cil1 = gGeoManager->GetMedium(strMatC7cil1.c_str());

  //define geometry
  TGeoPcon* geoC7cil1pcon = new TGeoPcon(0, 360, 62);
  zpos = 0.0;
  zpos = zpos + C1wal1d1 + C2spc1d1 + C3wal2d1 + C4spc2d1 + C5wal3d1 + C6spc3d1;
  geoC7cil1pcon->DefineSection(0, zpos, C7hld1i[0], C7hld1o[0]);
  zpos = zpos + C7hld1t[0];
  geoC7cil1pcon->DefineSection(1, zpos, C7hld1i[0], C7hld1o[0]);
  count = 2;
  for (int n = 0; n <= 14; n++) {
    geoC7cil1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1r[n] + C7cil1l[n]); count = count + 1;
    zpos = zpos + C7hld1d[n];
    geoC7cil1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1r[n] + C7cil1l[n]); count = count + 1;
    geoC7cil1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1o[n+1]); count = count + 1;
    zpos = zpos + C7hld1t[n+1];
    geoC7cil1pcon->DefineSection(count, zpos, C7hld1i[n], C7hld1o[n+1]); count = count + 1;
  }
  geoC7cil1pcon->SetName("geoC7cil1pconname");

  //-   Intersection and Subtraction
  TGeoCompositeShape* geoC7cil1 = new TGeoCompositeShape("geoC7cil1name", "((geoC7cil1pconname - geoC7hld1name) * geoC6spc3name) - geoC7mag6name");
  TGeoVolume *volC7cil1 = new TGeoVolume("volC7cil1name", geoC7cil1, strMedC7cil1);

  //-   put volume
  volC7cil1->SetLineColor(kOrange);
  volC6spc3->AddNode(volC7cil1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------


  //##############
  //#   CryoL

  //==============
  //=   Group D

  //--------------
  //-   D1wal1

  //get parameters from .xml file
  double D1wal1l1 = content.getParamLength("D1wal1/l1");
  double D1wal1l2 = content.getParamLength("D1wal1/l2");
  double D1wal1l3 = content.getParamLength("D1wal1/l3");
  double D1wal1l4 = content.getParamLength("D1wal1/l4");
  double D1wal1l5 = content.getParamLength("D1wal1/l5");
  double D1wal1l6 = content.getParamLength("D1wal1/l6");
  double D1wal1l7 = content.getParamLength("D1wal1/l7");
  //
  double D1wal1r1 = content.getParamLength("D1wal1/r1");
  double D1wal1r2 = content.getParamLength("D1wal1/r2");
  double D1wal1r3 = content.getParamLength("D1wal1/r3");
  double D1wal1r4 = content.getParamLength("D1wal1/r4");
  string strMatD1wal1 = content.getParamString("D1wal1/Material");
  TGeoMedium* strMedD1wal1 = gGeoManager->GetMedium(strMatD1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoD1wal1pcon = new TGeoPcon(0, 360, 8);
  geoD1wal1pcon->DefineSection(0, zpos, 0, D1wal1r1);
  zpos = zpos + D1wal1l1;
  geoD1wal1pcon->DefineSection(1, zpos, 0, D1wal1r1);
  zpos = zpos + D1wal1l2;
  geoD1wal1pcon->DefineSection(2, zpos, 0, D1wal1r2);
  zpos = zpos + D1wal1l3;
  geoD1wal1pcon->DefineSection(3, zpos, 0, D1wal1r2);
  zpos = zpos + D1wal1l4;
  geoD1wal1pcon->DefineSection(4, zpos, 0, D1wal1r3);
  zpos = zpos + D1wal1l5;
  geoD1wal1pcon->DefineSection(5, zpos, 0, D1wal1r3);
  zpos = zpos + D1wal1l6;
  geoD1wal1pcon->DefineSection(6, zpos, 0, D1wal1r4);
  zpos = zpos + D1wal1l7;
  geoD1wal1pcon->DefineSection(7, zpos, 0, D1wal1r4);
  geoD1wal1pcon->SetName("geoD1wal1pconname");

  //-   Subtraction volume
  TGeoTube* geoD1wal1tub = new TGeoTube(0, 10, -GlobalDistanceL);
  geoD1wal1tub->SetName("geoD1wal1tubname");

  TGeoCompositeShape* geoD1wal1 = new TGeoCompositeShape("geoD1wal1name", "geoD1wal1pconname:rotHERname - geoD1wal1tubname");
  TGeoVolume *volD1wal1 = new TGeoVolume("volD1wal1name", geoD1wal1, strMedD1wal1);

  //-   put volume
  volD1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volD1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   D2spc1

  //get parameters from .xml file
  bpthick = content.getParamLength("D2spc1/bpthick");
  double D2spc1r1 = D1wal1r1 - bpthick;
  double D2spc1r2 = D1wal1r2 - bpthick;
  double D2spc1r3 = D1wal1r3 - bpthick;
  double D2spc1r4 = D1wal1r4 - bpthick;
  string strMatD2spc1 = content.getParamString("D2spc1/Material");
  TGeoMedium* strMedD2spc1 = gGeoManager->GetMedium(strMatD2spc1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoD2spc1pcon = new TGeoPcon(0, 360, 8);
  geoD2spc1pcon->DefineSection(0, zpos, 0, D2spc1r1);
  zpos = zpos + D1wal1l1;
  geoD2spc1pcon->DefineSection(1, zpos, 0, D2spc1r1);
  zpos = zpos + D1wal1l2;
  geoD2spc1pcon->DefineSection(2, zpos, 0, D2spc1r2);
  zpos = zpos + D1wal1l3;
  geoD2spc1pcon->DefineSection(3, zpos, 0, D2spc1r2);
  zpos = zpos + D1wal1l4;
  geoD2spc1pcon->DefineSection(4, zpos, 0, D2spc1r3);
  zpos = zpos + D1wal1l5;
  geoD2spc1pcon->DefineSection(5, zpos, 0, D2spc1r3);
  zpos = zpos + D1wal1l6;
  geoD2spc1pcon->DefineSection(6, zpos, 0, D2spc1r4);
  zpos = zpos + D1wal1l7;
  geoD2spc1pcon->DefineSection(7, zpos, 0, D2spc1r4);
  geoD2spc1pcon->SetName("geoD2spc1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoD2spc1 = new TGeoCompositeShape("geoD2spc1name", "geoD2spc1pconname:rotHERname * geoD1wal1name");
  TGeoVolume *volD2spc1 = new TGeoVolume("volD2spc1name", geoD2spc1, strMedD2spc1);

  //-   put volume
  volD2spc1->SetLineColor(kGray + 1);
  volD1wal1->AddNode(volD2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //    Group E
  //==============

  //--------------
  //-   E1wal1

  //get parameters from .xml file
  double E1wal1l1 = content.getParamLength("E1wal1/l1");
  double E1wal1l2 = content.getParamLength("E1wal1/l2");
  double E1wal1l3 = content.getParamLength("E1wal1/l3");
  double E1wal1l4 = content.getParamLength("E1wal1/l4");
  double E1wal1l5 = content.getParamLength("E1wal1/l5");
  double E1wal1l6 = content.getParamLength("E1wal1/l6");
  double E1wal1l7 = content.getParamLength("E1wal1/l7");
  double E1wal1l8 = content.getParamLength("E1wal1/l8");
  double E1wal1l9 = content.getParamLength("E1wal1/l9");
  //
  double E1wal1r1 = content.getParamLength("E1wal1/r1");
  double E1wal1r2 = content.getParamLength("E1wal1/r2");
  double E1wal1r3 = content.getParamLength("E1wal1/r3");
  double E1wal1r4 = content.getParamLength("E1wal1/r4");
  double E1wal1r5 = content.getParamLength("E1wal1/r5");
  double E1wal1r6 = content.getParamLength("E1wal1/r6");
  string strMatE1wal1 = content.getParamString("E1wal1/Material");
  TGeoMedium* strMedE1wal1 = gGeoManager->GetMedium(strMatE1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoE1wal1pcon = new TGeoPcon(0, 360, 10);
  geoE1wal1pcon->DefineSection(0, zpos, 0, E1wal1r1);
  zpos = zpos + E1wal1l1;
  geoE1wal1pcon->DefineSection(1, zpos, 0, E1wal1r1);
  zpos = zpos + E1wal1l2;
  geoE1wal1pcon->DefineSection(2, zpos, 0, E1wal1r2);
  zpos = zpos + E1wal1l3;
  geoE1wal1pcon->DefineSection(3, zpos, 0, E1wal1r3);
  zpos = zpos + E1wal1l4;
  geoE1wal1pcon->DefineSection(4, zpos, 0, E1wal1r3);
  zpos = zpos + E1wal1l5;
  geoE1wal1pcon->DefineSection(5, zpos, 0, E1wal1r4);
  zpos = zpos + E1wal1l6;
  geoE1wal1pcon->DefineSection(6, zpos, 0, E1wal1r5);
  zpos = zpos + E1wal1l7;
  geoE1wal1pcon->DefineSection(7, zpos, 0, E1wal1r5);
  zpos = zpos + E1wal1l8;
  geoE1wal1pcon->DefineSection(8, zpos, 0, E1wal1r6);
  zpos = zpos + E1wal1l9;
  geoE1wal1pcon->DefineSection(9, zpos, 0, E1wal1r6);
  geoE1wal1pcon->SetName("geoE1wal1pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoE1wal1 = new TGeoCompositeShape("geoE1wal1name", "geoE1wal1pconname:rotLERname - geoD1wal1tubname");
  TGeoVolume *volE1wal1 = new TGeoVolume("volE1wal1name", geoE1wal1, strMedE1wal1);

  //-   put volume
  volE1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volE1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   E2spc1

  //get parameters from .xml file
  bpthick = content.getParamLength("E2spc1/bpthick");
  double E2spc1r1 = E1wal1r1 - bpthick;
  double E2spc1r2 = E1wal1r2 - bpthick;
  double E2spc1r3 = E1wal1r3 - bpthick;
  double E2spc1r4 = E1wal1r4 - bpthick;
  double E2spc1r5 = E1wal1r5 - bpthick;
  double E2spc1r6 = E1wal1r6 - bpthick;
  string strMatE2spc1 = content.getParamString("E2spc1/Material");
  TGeoMedium* strMedE2spc1 = gGeoManager->GetMedium(strMatE2spc1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoE2spc1pcon = new TGeoPcon(0, 360, 10);
  geoE2spc1pcon->DefineSection(0, zpos, 0, E2spc1r1);
  zpos = zpos + E1wal1l1;
  geoE2spc1pcon->DefineSection(1, zpos, 0, E2spc1r1);
  zpos = zpos + E1wal1l2;
  geoE2spc1pcon->DefineSection(2, zpos, 0, E2spc1r2);
  zpos = zpos + E1wal1l3;
  geoE2spc1pcon->DefineSection(3, zpos, 0, E2spc1r3);
  zpos = zpos + E1wal1l4;
  geoE2spc1pcon->DefineSection(4, zpos, 0, E2spc1r3);
  zpos = zpos + E1wal1l5;
  geoE2spc1pcon->DefineSection(5, zpos, 0, E2spc1r4);
  zpos = zpos + E1wal1l6;
  geoE2spc1pcon->DefineSection(6, zpos, 0, E2spc1r5);
  zpos = zpos + E1wal1l7;
  geoE2spc1pcon->DefineSection(7, zpos, 0, E2spc1r5);
  zpos = zpos + E1wal1l8;
  geoE2spc1pcon->DefineSection(8, zpos, 0, E2spc1r6);
  zpos = zpos + E1wal1l9;
  geoE2spc1pcon->DefineSection(9, zpos, 0, E2spc1r6);
  geoE2spc1pcon->SetName("geoE2spc1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoE2spc1 = new TGeoCompositeShape("geoE2spc1name", "geoE2spc1pconname:rotLERname * geoE1wal1name");
  TGeoVolume *volE2spc1 = new TGeoVolume("volE2spc1name", geoE2spc1, strMedE2spc1);

  //-   put volume
  volE2spc1->SetLineColor(kGray + 1);
  volE1wal1->AddNode(volE2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //==============
  //=   Group F

  //--------------
  //-   F1wal1

  //get parameters from .xml file
  double F1wal1d1 = GlobalDistanceL;
  //
  double F1wal1l1 = content.getParamLength("F1wal1/l1");
  double F1wal1l2 = content.getParamLength("F1wal1/l2");
  double F1wal1l3 = content.getParamLength("F1wal1/l3");
  double F1wal1l4 = content.getParamLength("F1wal1/l4");
  double F1wal1l5 = content.getParamLength("F1wal1/l5");
  double F1wal1l6 = content.getParamLength("F1wal1/l6");
  double F1wal1l7 = content.getParamLength("F1wal1/l7");
  double F1wal1l8 = content.getParamLength("F1wal1/l8");
  //
  double F1wal1r1 = content.getParamLength("F1wal1/r1");
  double F1wal1r2 = content.getParamLength("F1wal1/r2");
  double F1wal1r3 = content.getParamLength("F1wal1/r3");
  double F1wal1r4 = content.getParamLength("F1wal1/r4");
  double F1wal1r5 = content.getParamLength("F1wal1/r5");
  double F1wal1r6 = content.getParamLength("F1wal1/r6");
  double F1wal1r7 = content.getParamLength("F1wal1/r7");
  string strMatF1wal1 = content.getParamString("F1wal1/Material");
  TGeoMedium* strMedF1wal1 = gGeoManager->GetMedium(strMatF1wal1.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoF1wal1pcon = new TGeoPcon(0, 360, 13);
  zpos = zpos + F1wal1d1;
  geoF1wal1pcon->DefineSection(0, zpos, 0, F1wal1r1);
  zpos = zpos + F1wal1l1;
  geoF1wal1pcon->DefineSection(1, zpos, 0, F1wal1r2);
  zpos = zpos + F1wal1l2;
  geoF1wal1pcon->DefineSection(2, zpos, 0, F1wal1r2);
  geoF1wal1pcon->DefineSection(3, zpos, 0, F1wal1r3);
  zpos = zpos + F1wal1l3;
  geoF1wal1pcon->DefineSection(4, zpos, 0, F1wal1r3);
  zpos = zpos + F1wal1l4;
  geoF1wal1pcon->DefineSection(5, zpos, 0, F1wal1r4);
  zpos = zpos + F1wal1l5;
  geoF1wal1pcon->DefineSection(6, zpos, 0, F1wal1r4);
  geoF1wal1pcon->DefineSection(7, zpos, 0, F1wal1r5);
  zpos = zpos + F1wal1l6;
  geoF1wal1pcon->DefineSection(8, zpos, 0, F1wal1r5);
  geoF1wal1pcon->DefineSection(9, zpos, 0, F1wal1r6);
  zpos = zpos + F1wal1l7;
  geoF1wal1pcon->DefineSection(10, zpos, 0, F1wal1r6);
  geoF1wal1pcon->DefineSection(11, zpos, 0, F1wal1r7);
  zpos = zpos + F1wal1l8;
  geoF1wal1pcon->DefineSection(12, zpos, 0, F1wal1r7);
  geoF1wal1pcon->SetName("geoF1wal1pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoF1wal1 = new TGeoCompositeShape("geoF1wal1name", "geoF1wal1pconname - geoD1wal1name - geoE1wal1name");
  TGeoVolume *volF1wal1 = new TGeoVolume("volF1wal1name", geoF1wal1, strMedF1wal1);

  //-   put volume
  volF1wal1->SetLineColor(kGray + 3);
  volQCS->AddNode(volF1wal1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F2spc1

  //get parameters from .xml file
  double F2spc1d1 = content.getParamLength("F2spc1/d1");
  double F2spc1l1 = content.getParamLength("F2spc1/l1");
  double F2spc1l2 = content.getParamLength("F2spc1/l2");
  double F2spc1l3 = content.getParamLength("F2spc1/l3");
  double F2spc1l4 = content.getParamLength("F2spc1/l4");
  double F2spc1l5 = content.getParamLength("F2spc1/l5");
  double F2spc1l6 = content.getParamLength("F2spc1/l6");
  double F2spc1l7 = content.getParamLength("F2spc1/l7");
  double F2spc1l8 = content.getParamLength("F2spc1/l8");
  double F2spc1r1 = content.getParamLength("F2spc1/r1");
  double F2spc1r2 = content.getParamLength("F2spc1/r2");
  double F2spc1r3 = content.getParamLength("F2spc1/r3");
  double F2spc1r4 = content.getParamLength("F2spc1/r4");
  double F2spc1r5 = content.getParamLength("F2spc1/r5");
  double F2spc1r6 = content.getParamLength("F2spc1/r6");
  double F2spc1r7 = content.getParamLength("F2spc1/r7");
  string strMatF2spc1 = content.getParamString("F2spc1/Material");
  TGeoMedium* strMedF2spc1 = gGeoManager->GetMedium(strMatF2spc1.c_str());

  //define geometry
  TGeoPcon* geoF2spc1pcon = new TGeoPcon(0, 360, 11);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1;
  geoF2spc1pcon->DefineSection(0, zpos, 0, F2spc1r1);
  zpos = zpos + F2spc1l1;
  geoF2spc1pcon->DefineSection(1, zpos, 0, F2spc1r2);
  zpos = zpos + F2spc1l2;
  geoF2spc1pcon->DefineSection(2, zpos, 0, F2spc1r2);
  geoF2spc1pcon->DefineSection(3, zpos, 0, F2spc1r3);
  zpos = zpos + F2spc1l3;
  geoF2spc1pcon->DefineSection(4, zpos, 0, F2spc1r3);
  zpos = zpos + F2spc1l4;
  geoF2spc1pcon->DefineSection(5, zpos, 0, F2spc1r4);
  zpos = zpos + F2spc1l5 - F2spc1l6;
  geoF2spc1pcon->DefineSection(6, zpos, 0, F2spc1r4);
  geoF2spc1pcon->DefineSection(7, zpos, 0, F2spc1r6);
  zpos = zpos + F2spc1l7;
  geoF2spc1pcon->DefineSection(8, zpos, 0, F2spc1r6);
  geoF2spc1pcon->DefineSection(9, zpos, 0, F2spc1r7);
  zpos = zpos + F2spc1l8;
  geoF2spc1pcon->DefineSection(10, zpos, 0, F2spc1r7);
  geoF2spc1pcon->SetName("geoF2spc1pconname");

  //-   Subtraction volume
  TGeoPcon* geoF2spc1tub = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F2spc1l1 + F2spc1l2 + F2spc1l3 + F2spc1l4 + F2spc1l5 - F2spc1l6;
  geoF2spc1tub->DefineSection(0, zpos, F2spc1r4, F2spc1r5);
  zpos = zpos + F2spc1l6;
  geoF2spc1tub->DefineSection(1, zpos, F2spc1r4, F2spc1r5);
  geoF2spc1tub->SetName("geoF2spc1tubname");

  TGeoCompositeShape* geoF2spc1 = new TGeoCompositeShape("geoF2spc1name", "geoF2spc1pconname - geoF2spc1tubname - geoD1wal1name - geoE1wal1name");
  //TGeoCompositeShape* geoF2spc1 = new TGeoCompositeShape("geoF2spc1name","(geoF2spc1pconname - geoF2spc1tubname) * geoF1wal1name");
  TGeoVolume *volF2spc1 = new TGeoVolume("volF2spc1name", geoF2spc1, strMedF2spc1);

  //-   put volume
  volF2spc1->SetLineColor(kGray + 1);
  volF1wal1->AddNode(volF2spc1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F3wal2

  //get parameters from .xml file
  double F3wal2d1 = content.getParamLength("F3wal2/d1");
  double F3wal2l1 = content.getParamLength("F3wal2/l1");
  double F3wal2l2 = content.getParamLength("F3wal2/l2");
  double F3wal2l3 = content.getParamLength("F3wal2/l3");
  double F3wal2l4 = content.getParamLength("F3wal2/l4");
  double F3wal2l5 = content.getParamLength("F3wal2/l5");
  double F3wal2r1 = content.getParamLength("F3wal2/r1");
  double F3wal2r2 = content.getParamLength("F3wal2/r2");
  double F3wal2r3 = content.getParamLength("F3wal2/r3");
  double F3wal2r4 = content.getParamLength("F3wal2/r4");
  string strMatF3wal2 = content.getParamString("F3wal2/Material");
  TGeoMedium* strMedF3wal2 = gGeoManager->GetMedium(strMatF3wal2.c_str());

  //define geometry
  TGeoPcon* geoF3wal2pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1;
  geoF3wal2pcon->DefineSection(0, zpos, 0, F3wal2r1);
  zpos = zpos + F3wal2l1;
  geoF3wal2pcon->DefineSection(1, zpos, 0, F3wal2r2);
  zpos = zpos + F3wal2l2;
  geoF3wal2pcon->DefineSection(2, zpos, 0, F3wal2r2);
  geoF3wal2pcon->DefineSection(3, zpos, 0, F3wal2r3);
  zpos = zpos + F3wal2l3;
  geoF3wal2pcon->DefineSection(4, zpos, 0, F3wal2r3);
  zpos = zpos + F3wal2l4;
  geoF3wal2pcon->DefineSection(5, zpos, 0, F3wal2r4);
  zpos = zpos + F3wal2l5;
  geoF3wal2pcon->DefineSection(6, zpos, 0, F3wal2r4);
  geoF3wal2pcon->SetName("geoF3wal2pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoF3wal2 = new TGeoCompositeShape("geoF3wal2name", "geoF3wal2pconname - geoD1wal1name - geoE1wal1name");
  TGeoVolume *volF3wal2 = new TGeoVolume("volF3wal2name", geoF3wal2, strMedF3wal2);

  //-   put volume
  volF3wal2->SetLineColor(kGray + 3);
  volF2spc1->AddNode(volF3wal2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F3wal3

  //get parameters from .xml file
  double F3wal3d1 = content.getParamLength("F3wal3/d1");
  double F3wal3o1 = content.getParamLength("F3wal3/o1");
  double F3wal3l1 = content.getParamLength("F3wal3/l1");
  double F3wal3r1 = content.getParamLength("F3wal3/r1");
  double F3wal3t1 = content.getParamLength("F3wal3/t1");
  string strMatF3wal3 = content.getParamString("F3wal3/Material");
  TGeoMedium* strMedF3wal3 = gGeoManager->GetMedium(strMatF3wal3.c_str());

  //define geometry
  TGeoTube* geoF3wal3tube1 = new TGeoTube(0, F3wal3r1, F3wal3l1 / 2.0);
  geoF3wal3tube1->SetName("geoF3wal3tube1name");
  TGeoTranslation* trnsF3wal3tube1 = new TGeoTranslation("trnsF3wal3tube1name", F3wal3o1, F3wal3t1 / 2.0, F3wal3d1 - F3wal3l1 / 2.0);
  trnsF3wal3tube1->RegisterYourself();
  //
  TGeoTube* geoF3wal3tube2 = new TGeoTube(0, F3wal3r1, F3wal3l1 / 2.0);
  geoF3wal3tube2->SetName("geoF3wal3tube2name");
  TGeoTranslation* trnsF3wal3tube2 = new TGeoTranslation("trnsF3wal3tube2name", F3wal3o1, -F3wal3t1 / 2.0, F3wal3d1 - F3wal3l1 / 2.0);
  trnsF3wal3tube2->RegisterYourself();
  //
  new TGeoBBox("geoF3wal3bboxname", F3wal3r1, F3wal3t1 / 2.0, F3wal3l1 / 2.0);
  TGeoTranslation* trnsF3wal3bbox = new TGeoTranslation("trnsF3wal3bboxname", F3wal3o1, 0.0, F3wal3d1 - F3wal3l1 / 2.0);
  trnsF3wal3bbox->RegisterYourself();

  //Summation volume
  TGeoCompositeShape* geoF3wal3 = new TGeoCompositeShape("geoF3wal3name", "(geoF3wal3tube1name:trnsF3wal3tube1name + geoF3wal3bboxname:trnsF3wal3bboxname + geoF3wal3tube2name:trnsF3wal3tube2name) - geoD1wal1name");
  TGeoVolume *volF3wal3 = new TGeoVolume("volF3wal3name", geoF3wal3, strMedF3wal3);

  //put volume
  volF3wal3->SetLineColor(kGray + 3);
  volF2spc1->AddNode(volF3wal3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F4spc2

  //get parameters from .xml file
  double F4spc2d1 = content.getParamLength("F4spc2/d1");
  double F4spc2l1 = content.getParamLength("F4spc2/l1");
  double F4spc2l2 = content.getParamLength("F4spc2/l2");
  double F4spc2l3 = content.getParamLength("F4spc2/l3");
  double F4spc2l4 = content.getParamLength("F4spc2/l4");
  double F4spc2l5 = content.getParamLength("F4spc2/l5");
  double F4spc2r1 = content.getParamLength("F4spc2/r1");
  double F4spc2r2 = content.getParamLength("F4spc2/r2");
  double F4spc2r3 = content.getParamLength("F4spc2/r3");
  double F4spc2r4 = content.getParamLength("F4spc2/r4");
  string strMatF4spc2 = content.getParamString("F4spc2/Material");
  TGeoMedium* strMedF4spc2 = gGeoManager->GetMedium(strMatF4spc2.c_str());

  //define geometry
  TGeoPcon* geoF4spc2pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1 + F4spc2d1;
  geoF4spc2pcon->DefineSection(0, zpos, 0, F4spc2r1);
  zpos = zpos + F4spc2l1;
  geoF4spc2pcon->DefineSection(1, zpos, 0, F4spc2r2);
  zpos = zpos + F4spc2l2;
  geoF4spc2pcon->DefineSection(2, zpos, 0, F4spc2r2);
  geoF4spc2pcon->DefineSection(3, zpos, 0, F4spc2r3);
  zpos = zpos + F4spc2l3;
  geoF4spc2pcon->DefineSection(4, zpos, 0, F4spc2r3);
  zpos = zpos + F4spc2l4;
  geoF4spc2pcon->DefineSection(5, zpos, 0, F4spc2r4);
  zpos = zpos + F4spc2l5;
  geoF4spc2pcon->DefineSection(6, zpos, 0, F4spc2r4);
  geoF4spc2pcon->SetName("geoF4spc2pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoF4spc2 = new TGeoCompositeShape("geoF4spc2name", "geoF4spc2pconname - geoD1wal1name - geoE1wal1name");
  TGeoVolume *volF4spc2 = new TGeoVolume("volF4spc2name", geoF4spc2, strMedF4spc2);

  //-   put volume
  volF4spc2->SetLineColor(kGray + 1);
  volF3wal2->AddNode(volF4spc2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F4spc3

  //get parameters from .xml file
  double F4spc3d1 = content.getParamLength("F4spc3/d1");
  double F4spc3l1 = content.getParamLength("F4spc3/l1");
  double F4spc3r1 = content.getParamLength("F4spc3/r1");
  string strMatF4spc3 = content.getParamString("F4spc3/Material");
  TGeoMedium* strMedF4spc3 = gGeoManager->GetMedium(strMatF4spc3.c_str());

  //define geometry
  TGeoTube* geoF4spc3tube1 = new TGeoTube(0, F4spc3r1, F4spc3l1 / 2.0);
  geoF4spc3tube1->SetName("geoF4spc3tube1name");
  TGeoTranslation* trnsF4spc3tube1 = new TGeoTranslation("trnsF4spc3tube1name", F3wal3t1 / 2.0, F3wal3o1, F3wal3d1 + F4spc3d1 - F4spc3l1 / 2.0);
  trnsF4spc3tube1->RegisterYourself();
  //
  TGeoTube* geoF4spc3tube2 = new TGeoTube(0, F4spc3r1, F4spc3l1 / 2.0);
  geoF4spc3tube2->SetName("geoF4spc3tube2name");
  TGeoTranslation* trnsF4spc3tube2 = new TGeoTranslation("trnsF4spc3tube2name", -F3wal3t1 / 2.0, F3wal3o1, F3wal3d1 + F4spc3d1 - F4spc3l1 / 2.0);
  trnsF4spc3tube2->RegisterYourself();
  //
  new TGeoBBox("geoF4spc3bboxname", F4spc3r1, F3wal3t1 / 2.0, F4spc3l1 / 2.0);
  TGeoTranslation* trnsF4spc3bbox = new TGeoTranslation("trnsF4spc3bboxname", 0.0, F3wal3o1, F3wal3d1 + F4spc3d1 - F4spc3l1 / 2.0);
  trnsF4spc3bbox->RegisterYourself();

  //Summation volume
  TGeoCompositeShape* geoF4spc3 = new TGeoCompositeShape("geoF4spc3name", "(geoF4spc3tube1name:trnsF4spc3tube1name + geoF4spc3bboxname:trnsF4spc3bboxname + geoF4spc3tube2name:trnsF4spc3tube2name) - geoD1wal1name");
  TGeoVolume *volF4spc3 = new TGeoVolume("volF4spc3name", geoF4spc3, strMedF4spc3);

  //put volume
  volF4spc3->SetLineColor(kGray + 1);
  volF3wal3->AddNode(volF4spc3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F5wal4

  //get parameters from .xml file
  double F5wal4d1 = content.getParamLength("F5wal4/d1");
  double F5wal4l1 = content.getParamLength("F5wal4/l1");
  double F5wal4l2 = content.getParamLength("F5wal4/l2");
  double F5wal4l3 = content.getParamLength("F5wal4/l3");
  double F5wal4l4 = content.getParamLength("F5wal4/l4");
  double F5wal4l5 = content.getParamLength("F5wal4/l5");
  double F5wal4l6 = content.getParamLength("F5wal4/l6");
  double F5wal4r1 = content.getParamLength("F5wal4/r1");
  double F5wal4r2 = content.getParamLength("F5wal4/r2");
  double F5wal4r3 = content.getParamLength("F5wal4/r3");
  double F5wal4r4 = content.getParamLength("F5wal4/r4");
  double F5wal4r5 = content.getParamLength("F5wal4/r5");
  string strMatF5wal4 = content.getParamString("F5wal4/Material");
  TGeoMedium* strMedF5wal4 = gGeoManager->GetMedium(strMatF5wal4.c_str());

  //define geometry
  TGeoPcon* geoF5wal4pcon = new TGeoPcon(0, 360, 8);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1 + F4spc2d1 + F5wal4d1;
  geoF5wal4pcon->DefineSection(0, zpos, 0, F5wal4r1);
  zpos = zpos + F5wal4l1;
  geoF5wal4pcon->DefineSection(1, zpos, 0, F5wal4r2);
  zpos = zpos + F5wal4l2;
  geoF5wal4pcon->DefineSection(2, zpos, 0, F5wal4r2);
  geoF5wal4pcon->DefineSection(3, zpos, 0, F5wal4r3);
  zpos = zpos + F5wal4l3;
  geoF5wal4pcon->DefineSection(4, zpos, 0, F5wal4r3);
  zpos = zpos + F5wal4l4;
  geoF5wal4pcon->DefineSection(5, zpos, 0, F5wal4r4);
  zpos = zpos + F5wal4l5;
  geoF5wal4pcon->DefineSection(6, zpos, 0, F5wal4r4);
  zpos = zpos + F5wal4l6;
  geoF5wal4pcon->DefineSection(7, zpos, 0, F5wal4r5);
  geoF5wal4pcon->SetName("geoF5wal4pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoF5wal4 = new TGeoCompositeShape("geoF5wal4name", "geoF5wal4pconname - geoD1wal1name - geoE1wal1name");
  TGeoVolume *volF5wal4 = new TGeoVolume("volF5wal4name", geoF5wal4, strMedF5wal4);

  //-   put volume
  volF5wal4->SetLineColor(kGray + 3);
  volF4spc2->AddNode(volF5wal4, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F5wal5

  //get parameters from .xml file
  double F5wal5d1 = content.getParamLength("F5wal5/d1");
  double F5wal5l1 = content.getParamLength("F5wal5/l1");
  double F5wal5r1 = content.getParamLength("F5wal5/r1");
  string strMatF5wal5 = content.getParamString("F5wal5/Material");
  TGeoMedium* strMedF5wal5 = gGeoManager->GetMedium(strMatF5wal5.c_str());

  //define geometry
  TGeoPcon* geoF5wal5pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F5wal5d1;
  geoF5wal5pcon->DefineSection(0, zpos, 0, F5wal5r1);
  zpos = zpos + F5wal5l1;
  geoF5wal5pcon->DefineSection(1, zpos, 0, F5wal5r1);
  geoF5wal5pcon->SetName("geoF5wal5pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF5wal5 = new TGeoCompositeShape("geoF5wal5name", "geoF5wal5pconname:rotHERname - geoD1wal1name");
  TGeoVolume *volF5wal5 = new TGeoVolume("volF5wal5name", geoF5wal5, strMedF5wal5);

  //-   put volume
  volF5wal5->SetLineColor(kGray + 3);
  volF4spc3->AddNode(volF5wal5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F6spc5

  //get parameters from .xml file
  double F6spc5l1 = content.getParamLength("F6spc5/l1");
  double F6spc5l2 = content.getParamLength("F6spc5/l2");
  double F6spc5l3 = content.getParamLength("F6spc5/l3");
  double F6spc5l4 = content.getParamLength("F6spc5/l4");
  double F6spc5l5 = content.getParamLength("F6spc5/l5");
  //
  double F6spc5r1 = content.getParamLength("F6spc5/r1");
  double F6spc5r2 = content.getParamLength("F6spc5/r2");
  double F6spc5r3 = content.getParamLength("F6spc5/r3");
  string strMatF6spc5 = content.getParamString("F6spc5/Material");
  TGeoMedium* strMedF6spc5 = gGeoManager->GetMedium(strMatF6spc5.c_str());

  //define geometry
  TGeoPcon* geoF6spc5pcon = new TGeoPcon(0, 360, 6);
  zpos = 0.0;
  geoF6spc5pcon->DefineSection(0, zpos, 0, F6spc5r1);
  zpos = zpos + F6spc5l1;
  geoF6spc5pcon->DefineSection(1, zpos, 0, F6spc5r1);
  zpos = zpos + F6spc5l2;
  geoF6spc5pcon->DefineSection(2, zpos, 0, F6spc5r2);
  zpos = zpos + F6spc5l3;
  geoF6spc5pcon->DefineSection(3, zpos, 0, F6spc5r2);
  zpos = zpos + F6spc5l4;
  geoF6spc5pcon->DefineSection(4, zpos, 0, F6spc5r3);
  zpos = zpos + F6spc5l5;
  geoF6spc5pcon->DefineSection(5, zpos, 0, F6spc5r3);
  geoF6spc5pcon->SetName("geoF6spc5pconname");

  //Intersection Shape
  TGeoCompositeShape* geoF6spc5 = new TGeoCompositeShape("geoF6spc5name", "(geoF6spc5pconname:rotHERname * geoF5wal4name) - geoD1wal1name");
  TGeoVolume *volF6spc5 = new TGeoVolume("volF6spc5name", geoF6spc5, strMedF6spc5);

  //-   put volume
  volF6spc5->SetLineColor(kGray + 1);
  volF5wal4->AddNode(volF6spc5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F6spc6

  //get parameters from .xml file
  double F6spc6l1 = content.getParamLength("F6spc6/l1");
  double F6spc6l2 = content.getParamLength("F6spc6/l2");
  double F6spc6l3 = content.getParamLength("F6spc6/l3");
  double F6spc6l4 = content.getParamLength("F6spc6/l4");
  double F6spc6l5 = content.getParamLength("F6spc6/l5");
  double F6spc6l6 = content.getParamLength("F6spc6/l6");
  double F6spc6l7 = content.getParamLength("F6spc6/l7");
  //
  double F6spc6r1 = content.getParamLength("F6spc6/r1");
  double F6spc6r2 = content.getParamLength("F6spc6/r2");
  double F6spc6r3 = content.getParamLength("F6spc6/r3");
  double F6spc6r4 = content.getParamLength("F6spc6/r4");
  double F6spc6r5 = content.getParamLength("F6spc6/r5");
  string strMatF6spc6 = content.getParamString("F6spc6/Material");
  TGeoMedium* strMedF6spc6 = gGeoManager->GetMedium(strMatF6spc6.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoF6spc6pcon = new TGeoPcon(0, 360, 8);
  geoF6spc6pcon->DefineSection(0, zpos, 0, F6spc6r1);
  zpos = zpos + F6spc6l1;
  geoF6spc6pcon->DefineSection(1, zpos, 0, F6spc6r1);
  zpos = zpos + F6spc6l2;
  geoF6spc6pcon->DefineSection(2, zpos, 0, F6spc6r2);
  zpos = zpos + F6spc6l3;
  geoF6spc6pcon->DefineSection(3, zpos, 0, F6spc6r3);
  zpos = zpos + F6spc6l4;
  geoF6spc6pcon->DefineSection(4, zpos, 0, F6spc6r3);
  zpos = zpos + F6spc6l5;
  geoF6spc6pcon->DefineSection(5, zpos, 0, F6spc6r4);
  zpos = zpos + F6spc6l6;
  geoF6spc6pcon->DefineSection(6, zpos, 0, F6spc6r5);
  zpos = zpos + F6spc6l7;
  geoF6spc6pcon->DefineSection(7, zpos, 0, F6spc6r5);
  geoF6spc6pcon->SetName("geoF6spc6pconname");

  //Intersection volume
  TGeoCompositeShape* geoF6spc6 = new TGeoCompositeShape("geoF6spc6name", "(geoF6spc6pconname:rotLERname * geoF5wal4name) - geoE1wal1name");
  TGeoVolume *volF6spc6 = new TGeoVolume("volF6spc6name", geoF6spc6, strMedF6spc6);

  //-   put volume
  volF6spc6->SetLineColor(kGray + 1);
  volF5wal4->AddNode(volF6spc6, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F6spc4

  //--- F6tnl1 ---
  //get parameters from .xml file
  double F6tnl1r1 = content.getParamLength("F6tnl1/r1");
  double F6tnl1r2 = content.getParamLength("F6tnl1/r2");
  double F6tnl1r3 = content.getParamLength("F6tnl1/r3");
  //define geometry
  TGeoPcon* geoF6tnl1pcon = new TGeoPcon(0, 360, 6);
  zpos = 0.0;
  geoF6tnl1pcon->DefineSection(0, zpos, 0, F6tnl1r1);
  zpos = zpos + F6spc5l1;
  geoF6tnl1pcon->DefineSection(1, zpos, 0, F6tnl1r1);
  zpos = zpos + F6spc5l2;
  geoF6tnl1pcon->DefineSection(2, zpos, 0, F6tnl1r2);
  zpos = zpos + F6spc5l3;
  geoF6tnl1pcon->DefineSection(3, zpos, 0, F6tnl1r2);
  zpos = zpos + F6spc5l4;
  geoF6tnl1pcon->DefineSection(4, zpos, 0, F6tnl1r3);
  zpos = zpos + F6spc5l5;
  geoF6tnl1pcon->DefineSection(5, zpos, 0, F6tnl1r3);
  geoF6tnl1pcon->SetName("geoF6tnl1pconname");

  //--- F6tnl2 ---
  //get parameters from .xml file
  double F6tnl2r1 = content.getParamLength("F6tnl2/r1");
  double F6tnl2r2 = content.getParamLength("F6tnl2/r2");
  double F6tnl2r3 = content.getParamLength("F6tnl2/r3");
  double F6tnl2r4 = content.getParamLength("F6tnl2/r4");
  double F6tnl2r5 = content.getParamLength("F6tnl2/r5");
  //define geometry
  zpos = 0.0;
  TGeoPcon* geoF6tnl2pcon = new TGeoPcon(0, 360, 8);
  geoF6tnl2pcon->DefineSection(0, zpos, 0, F6tnl2r1);
  zpos = zpos + F6spc6l1;
  geoF6tnl2pcon->DefineSection(1, zpos, 0, F6tnl2r1);
  zpos = zpos + F6spc6l2;
  geoF6tnl2pcon->DefineSection(2, zpos, 0, F6tnl2r2);
  zpos = zpos + F6spc6l3;
  geoF6tnl2pcon->DefineSection(3, zpos, 0, F6tnl2r3);
  zpos = zpos + F6spc6l4;
  geoF6tnl2pcon->DefineSection(4, zpos, 0, F6tnl2r3);
  zpos = zpos + F6spc6l5;
  geoF6tnl2pcon->DefineSection(5, zpos, 0, F6tnl2r4);
  zpos = zpos + F6spc6l6;
  geoF6tnl2pcon->DefineSection(6, zpos, 0, F6tnl2r5);
  zpos = zpos + F6spc6l7;
  geoF6tnl2pcon->DefineSection(7, zpos, 0, F6tnl2r5);
  geoF6tnl2pcon->SetName("geoF6tnl2pconname");

  //get parameters from .xml file
  double F6spc4d1 = content.getParamLength("F6spc4/d1");
  double F6spc4l1 = content.getParamLength("F6spc4/l1");
  double F6spc4l2 = content.getParamLength("F6spc4/l2");
  double F6spc4l3 = content.getParamLength("F6spc4/l3");
  double F6spc4l4 = content.getParamLength("F6spc4/l4");
  double F6spc4l5 = content.getParamLength("F6spc4/l5");
  double F6spc4r1 = content.getParamLength("F6spc4/r1");
  double F6spc4r2 = content.getParamLength("F6spc4/r2");
  double F6spc4r3 = content.getParamLength("F6spc4/r3");
  double F6spc4r4 = content.getParamLength("F6spc4/r4");
  string strMatF6spc4 = content.getParamString("F6spc4/Material");
  TGeoMedium* strMedF6spc4 = gGeoManager->GetMedium(strMatF6spc4.c_str());

  //define geometry
  TGeoPcon* geoF6spc4pcon = new TGeoPcon(0, 360, 7);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1 + F4spc2d1 + F5wal4d1 + F6spc4d1;
  geoF6spc4pcon->DefineSection(0, zpos, 0, F6spc4r1);
  zpos = zpos + F6spc4l1;
  geoF6spc4pcon->DefineSection(1, zpos, 0, F6spc4r1);
  geoF6spc4pcon->DefineSection(2, zpos, 0, F6spc4r2);
  zpos = zpos + F6spc4l2;
  geoF6spc4pcon->DefineSection(3, zpos, 0, F6spc4r2);
  zpos = zpos + F6spc4l3;
  geoF6spc4pcon->DefineSection(4, zpos, 0, F6spc4r3);
  zpos = zpos + F6spc4l4;
  geoF6spc4pcon->DefineSection(5, zpos, 0, F6spc4r3);
  zpos = zpos + F6spc4l5;
  geoF6spc4pcon->DefineSection(6, zpos, 0, F6spc4r4);
  geoF6spc4pcon->SetName("geoF6spc4pconname");

  //-   Subtraction volume
  TGeoCompositeShape* geoF6spc4 = new TGeoCompositeShape("geoF6spc4name", "geoF6spc4pconname - geoF6tnl1pconname:rotHERname - geoF6tnl2pconname:rotLERname");
  TGeoVolume *volF6spc4 = new TGeoVolume("volF6spc4name", geoF6spc4, strMedF6spc4);

  //-   put volume
  volF6spc4->SetLineColor(kGray + 1);
  volF5wal4->AddNode(volF6spc4, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F6spc8

  //get parameters from .xml file
  double F6spc8d1 = content.getParamLength("F6spc8/d1");
  double F6spc8l1 = content.getParamLength("F6spc8/l1");
  double F6spc8r1 = content.getParamLength("F6spc8/r1");
  string strMatF6spc8 = content.getParamString("F6spc8/Material");
  TGeoMedium* strMedF6spc8 = gGeoManager->GetMedium(strMatF6spc8.c_str());

  //define geometry
  zpos = 0.0;
  TGeoPcon* geoF6spc8pcon = new TGeoPcon(0, 360, 2);
  zpos = zpos + F6spc8d1;
  geoF6spc8pcon->DefineSection(0, zpos, 0, F6spc8r1);
  zpos = zpos + F6spc8l1;
  geoF6spc8pcon->DefineSection(1, zpos, 0, F6spc8r1);
  geoF6spc8pcon->SetName("geoF6spc8pconname");

  //Intersection volume
  TGeoCompositeShape* geoF6spc8 = new TGeoCompositeShape("geoF6spc8name", "(geoF6spc8pconname:rotHERname * geoF5wal5name) - geoD1wal1name");
  TGeoVolume *volF6spc8 = new TGeoVolume("volF6spc8name", geoF6spc8, strMedF6spc8);

  //-   put volume
  volF6spc8->SetLineColor(kGray + 1);
  volF5wal5->AddNode(volF6spc8, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F6spc7

  //--- F6tnl3 ---
  //get parameters from .xml file
  double F6tnl3r1 = content.getParamLength("F6tnl3/r1");
  //define geometry
  TGeoPcon* geoF6tnl3pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F6spc8d1;
  geoF6tnl3pcon->DefineSection(0, zpos, 0, F6tnl3r1);
  zpos = zpos + F6spc8l1;
  geoF6tnl3pcon->DefineSection(1, zpos, 0, F6tnl3r1);
  geoF6tnl3pcon->SetName("geoF6tnl3pconname");

  //get parameters from .xml file
  double F6spc7d1 = content.getParamLength("F6spc7/d1");
  double F6spc7l1 = content.getParamLength("F6spc7/l1");
  double F6spc7r1 = content.getParamLength("F6spc7/r1");
  string strMatF6spc7 = content.getParamString("F6spc7/Material");
  TGeoMedium* strMedF6spc7 = gGeoManager->GetMedium(strMatF6spc7.c_str());

  //define geometry
  TGeoPcon* geoF6spc7pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F5wal5d1 + F6spc7d1;
  geoF6spc7pcon->DefineSection(0, zpos, 0, F6spc7r1);
  zpos = zpos + F6spc7l1;
  geoF6spc7pcon->DefineSection(1, zpos, 0, F6spc7r1);
  geoF6spc7pcon->SetName("geoF6spc7pconname");

  //Subtraction volume
  TGeoCompositeShape* geoF6spc7 = new TGeoCompositeShape("geoF6spc7name", "geoF6spc7pconname:rotHERname - geoF6tnl3pconname:rotHERname");
  TGeoVolume *volF6spc7 = new TGeoVolume("volF6spc7name", geoF6spc7, strMedF6spc7);

  //-   put volume
  volF6spc7->SetLineColor(kGray + 1);
  volF5wal5->AddNode(volF6spc7, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag1

  //get parameters from .xml file
  double F7mag1d1 = content.getParamLength("F7mag1/d1");
  double F7mag1l1 = content.getParamLength("F7mag1/l1");
  double F7mag1o1 = content.getParamLength("F7mag1/o1");
  string strMatF7mag1 = content.getParamString("F7mag1/Material");
  TGeoMedium* strMedF7mag1 = gGeoManager->GetMedium(strMatF7mag1.c_str());

  //define geometry
  TGeoPcon* geoF7mag1pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag1d1;
  geoF7mag1pcon->DefineSection(0, zpos, 0, F7mag1o1);
  zpos = zpos + F7mag1l1;
  geoF7mag1pcon->DefineSection(1, zpos, 0, F7mag1o1);
  geoF7mag1pcon->SetName("geoF7mag1pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag1 = new TGeoCompositeShape("geoF7mag1name", "geoF7mag1pconname:rotHERname * geoF6spc4name");
  TGeoVolume *volF7mag1 = new TGeoVolume("volF7mag1name", geoF7mag1, strMedF7mag1);

  //-   put volume
  volF7mag1->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag2

  //get parameters from .xml file
  double F7mag2d1 = F6spc5l1 + F6spc5l2;
  double F7mag2l1 = content.getParamLength("F7mag2/l1");
  double F7mag2o1 = content.getParamLength("F7mag2/o1");
  string strMatF7mag2 = content.getParamString("F7mag2/Material");
  TGeoMedium* strMedF7mag2 = gGeoManager->GetMedium(strMatF7mag2.c_str());

  //define geometry
  TGeoPcon* geoF7mag2pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag2d1;
  geoF7mag2pcon->DefineSection(0, zpos, 0, F7mag2o1);
  zpos = zpos + F7mag2l1;
  geoF7mag2pcon->DefineSection(1, zpos, 0, F7mag2o1);
  geoF7mag2pcon->SetName("geoF7mag2pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag2 = new TGeoCompositeShape("geoF7mag2name", "geoF7mag2pconname:rotHERname * geoF6spc4name");
  TGeoVolume *volF7mag2 = new TGeoVolume("volF7mag2name", geoF7mag2, strMedF7mag2);

  //-   put volume
  volF7mag2->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag2, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag3

  //get parameters from .xml file
  double F7mag3d1 = content.getParamLength("F7mag3/d1");
  double F7mag3l1 = content.getParamLength("F7mag3/l1");
  double F7mag3o1 = content.getParamLength("F7mag3/o1");
  string strMatF7mag3 = content.getParamString("F7mag3/Material");
  TGeoMedium* strMedF7mag3 = gGeoManager->GetMedium(strMatF7mag3.c_str());

  //define geometry
  TGeoPcon* geoF7mag3pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag3d1;
  geoF7mag3pcon->DefineSection(0, zpos, 0, F7mag3o1);
  zpos = zpos + F7mag3l1;
  geoF7mag3pcon->DefineSection(1, zpos, 0, F7mag3o1);
  geoF7mag3pcon->SetName("geoF7mag3pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag3 = new TGeoCompositeShape("geoF7mag3name", "geoF7mag3pconname:rotLERname * geoF6spc4name");
  TGeoVolume *volF7mag3 = new TGeoVolume("volF7mag3name", geoF7mag3, strMedF7mag3);

  //-   put volume
  volF7mag3->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag3, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag4

  //get parameters from .xml file
  double F7mag4d1 = F6spc6l1 + F6spc6l2 + F6spc6l3;
  double F7mag4l1 = content.getParamLength("F7mag4/l1");
  double F7mag4o1 = content.getParamLength("F7mag4/o1");
  string strMatF7mag4 = content.getParamString("F7mag4/Material");
  TGeoMedium* strMedF7mag4 = gGeoManager->GetMedium(strMatF7mag4.c_str());

  //define geometry
  TGeoPcon* geoF7mag4pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag4d1;
  geoF7mag4pcon->DefineSection(0, zpos, 0, F7mag4o1);
  zpos = zpos + F7mag4l1;
  geoF7mag4pcon->DefineSection(1, zpos, 0, F7mag4o1);
  geoF7mag4pcon->SetName("geoF7mag4pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag4 = new TGeoCompositeShape("geoF7mag4name", "geoF7mag4pconname:rotLERname * geoF6spc4name");
  TGeoVolume *volF7mag4 = new TGeoVolume("volF7mag4name", geoF7mag4, strMedF7mag4);

  //-   put volume
  volF7mag4->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag4, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag5

  //get parameters from .xml file
  double F7mag5d1 = content.getParamLength("F7mag5/d1");
  double F7mag5l1 = content.getParamLength("F7mag5/l1");
  double F7mag5o1 = content.getParamLength("F7mag5/o1");
  string strMatF7mag5 = content.getParamString("F7mag5/Material");
  TGeoMedium* strMedF7mag5 = gGeoManager->GetMedium(strMatF7mag5.c_str());

  //define geometry
  TGeoPcon* geoF7mag5pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag5d1;
  geoF7mag5pcon->DefineSection(0, zpos, 0, F7mag5o1);
  zpos = zpos + F7mag5l1;
  geoF7mag5pcon->DefineSection(1, zpos, 0, F7mag5o1);
  geoF7mag5pcon->SetName("geoF7mag5pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag5 = new TGeoCompositeShape("geoF7mag5name", "geoF7mag5pconname:rotLERname * geoF6spc4name");
  TGeoVolume *volF7mag5 = new TGeoVolume("volF7mag5name", geoF7mag5, strMedF7mag5);

  //-   put volume
  volF7mag5->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag5, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag6

  //get parameters from .xml file
  double F7mag6d1 = content.getParamLength("F7mag6/d1");
  double F7mag6l1 = content.getParamLength("F7mag6/l1");
  double F7mag6o1 = content.getParamLength("F7mag6/o1");
  string strMatF7mag6 = content.getParamString("F7mag6/Material");
  TGeoMedium* strMedF7mag6 = gGeoManager->GetMedium(strMatF7mag6.c_str());

  //define geometry
  TGeoPcon* geoF7mag6pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F7mag6d1;
  geoF7mag6pcon->DefineSection(0, zpos, 0, F7mag6o1);
  zpos = zpos + F7mag6l1;
  geoF7mag6pcon->DefineSection(1, zpos, 0, F7mag6o1);
  geoF7mag6pcon->SetName("geoF7mag6pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag6 = new TGeoCompositeShape("geoF7mag6name", "(geoF7mag6pconname * geoF6spc4name) - geoF7mag5name");
  TGeoVolume *volF7mag6 = new TGeoVolume("volF7mag6name", geoF7mag6, strMedF7mag6);

  //-   put volume
  volF7mag6->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7mag6, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7mag7

  //get parameters from .xml file
  double F7mag7d1 = content.getParamLength("F7mag7/d1");
  double F7mag7l1 = content.getParamLength("F7mag7/l1");
  double F7mag7o1 = content.getParamLength("F7mag7/o1");
  string strMatF7mag7 = content.getParamString("F7mag7/Material");
  TGeoMedium* strMedF7mag7 = gGeoManager->GetMedium(strMatF7mag7.c_str());

  //define geometry
  TGeoPcon* geoF7mag7pcon = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  zpos = zpos + F5wal5d1 + F6spc7d1 + F7mag7d1;
  geoF7mag7pcon->DefineSection(0, zpos, 0, F7mag7o1);
  zpos = zpos + F7mag7l1;
  geoF7mag7pcon->DefineSection(1, zpos, 0, F7mag7o1);
  geoF7mag7pcon->SetName("geoF7mag7pconname");

  //-   Intersection volume
  TGeoCompositeShape* geoF7mag7 = new TGeoCompositeShape("geoF7mag7name", "geoF7mag7pconname:rotHERname * geoF6spc7name");
  TGeoVolume *volF7mag7 = new TGeoVolume("volF7mag7name", geoF7mag7, strMedF7mag7);

  //-   put volume
  volF7mag7->SetLineColor(kOrange);
  volF6spc7->AddNode(volF7mag7, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7hld1

  //get parameters from .xml file
  double F7hld1t[7], F7hld1d[6], F7hld1i[6], F7hld1o[7], F7hld1r[6];
  F7hld1t[0] = content.getParamLength("F7hld1/t1");
  F7hld1t[1] = content.getParamLength("F7hld1/t2");
  F7hld1t[2] = content.getParamLength("F7hld1/t3");
  F7hld1t[3] = content.getParamLength("F7hld1/t4");
  F7hld1t[4] = content.getParamLength("F7hld1/t5");
  F7hld1t[5] = content.getParamLength("F7hld1/t6");
  F7hld1t[6] = content.getParamLength("F7hld1/t7");
  F7hld1d[0] = content.getParamLength("F7hld1/d1");
  F7hld1d[1] = content.getParamLength("F7hld1/d2");
  F7hld1d[2] = content.getParamLength("F7hld1/d3");
  F7hld1d[3] = content.getParamLength("F7hld1/d4");
  F7hld1d[4] = content.getParamLength("F7hld1/d5");
  F7hld1d[5] = content.getParamLength("F7hld1/d6");
  F7hld1i[0] = content.getParamLength("F7hld1/i1");
  F7hld1i[1] = content.getParamLength("F7hld1/i2");
  F7hld1i[2] = content.getParamLength("F7hld1/i3");
  F7hld1i[3] = content.getParamLength("F7hld1/i4");
  F7hld1i[4] = content.getParamLength("F7hld1/i5");
  F7hld1i[5] = content.getParamLength("F7hld1/i6");
  F7hld1o[0] = content.getParamLength("F7hld1/o1");
  F7hld1o[1] = content.getParamLength("F7hld1/o2");
  F7hld1o[2] = content.getParamLength("F7hld1/o3");
  F7hld1o[3] = content.getParamLength("F7hld1/o4");
  F7hld1o[4] = content.getParamLength("F7hld1/o5");
  F7hld1o[5] = content.getParamLength("F7hld1/o6");
  F7hld1o[6] = content.getParamLength("F7hld1/o7");
  for (int n = 0; n < 6; n++) {
    F7hld1r[n] = F7hld1i[n] + (-1 * F7hld1t[0]); // F7hld1t is negative
  }
  string strMatF7hld1 = content.getParamString("F7hld1/Material");
  TGeoMedium* strMedF7hld1 = gGeoManager->GetMedium(strMatF7hld1.c_str());

  //define geometry
  TGeoPcon* geoF7hld1pcon = new TGeoPcon(0, 360, 26);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1 + F4spc2d1 + F5wal4d1 + F6spc4d1;
  geoF7hld1pcon->DefineSection(0, zpos, F7hld1i[0], F7hld1o[0]);
  zpos = zpos + F7hld1t[0];
  geoF7hld1pcon->DefineSection(1, zpos, F7hld1i[0], F7hld1o[0]);
  count = 2;
  for (int n = 0; n <= 5; n++) {
    geoF7hld1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1r[n]); count = count + 1;
    zpos = zpos + F7hld1d[n];
    geoF7hld1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1r[n]); count = count + 1;
    geoF7hld1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1o[n+1]); count = count + 1;
    zpos = zpos + F7hld1t[n+1];
    geoF7hld1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1o[n+1]); count = count + 1;
  }
  geoF7hld1pcon->SetName("geoF7hld1pconname");

  //-   Intersection and Subtraction
  TGeoCompositeShape* geoF7hld1 = new TGeoCompositeShape("geoF7hld1name", "(geoF7hld1pconname * geoF6spc4name)");
  TGeoVolume *volF7hld1 = new TGeoVolume("volF7hld1name", geoF7hld1, strMedF7hld1);

  //-   put volume
  volF7hld1->SetLineColor(kGray + 3);
  volF6spc4->AddNode(volF7hld1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------

  //--------------
  //-   F7cil1

  //get parameters from .xml file
  double F7cil1l[6];
  F7cil1l[0] = content.getParamLength("F7cil1/l1");
  F7cil1l[1] = content.getParamLength("F7cil1/l2");
  F7cil1l[2] = content.getParamLength("F7cil1/l3");
  F7cil1l[3] = content.getParamLength("F7cil1/l4");
  F7cil1l[4] = content.getParamLength("F7cil1/l5");
  F7cil1l[5] = content.getParamLength("F7cil1/l6");
  string strMatF7cil1 = content.getParamString("F7cil1/Material");
  TGeoMedium* strMedF7cil1 = gGeoManager->GetMedium(strMatF7cil1.c_str());

  //define geometry
  TGeoPcon* geoF7cil1pcon = new TGeoPcon(0, 360, 26);
  zpos = 0.0;
  zpos = zpos + F1wal1d1 + F2spc1d1 + F3wal2d1 + F4spc2d1 + F5wal4d1 + F6spc4d1;
  geoF7cil1pcon->DefineSection(0, zpos, F7hld1i[0], F7hld1o[0]);
  zpos = zpos + F7hld1t[0];
  geoF7cil1pcon->DefineSection(1, zpos, F7hld1i[0], F7hld1o[0]);
  count = 2;
  for (int n = 0; n <= 5; n++) {
    geoF7cil1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1r[n] + F7cil1l[n]); count = count + 1;
    zpos = zpos + F7hld1d[n];
    geoF7cil1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1r[n] + F7cil1l[n]); count = count + 1;
    geoF7cil1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1o[n+1]); count = count + 1;
    zpos = zpos + F7hld1t[n+1];
    geoF7cil1pcon->DefineSection(count, zpos, F7hld1i[n], F7hld1o[n+1]); count = count + 1;
  }
  geoF7cil1pcon->SetName("geoF7cil1pconname");

  //-   Intersection and Subtraction
  TGeoCompositeShape* geoF7cil1 = new TGeoCompositeShape("geoF7cil1name", "((geoF7cil1pconname - geoF7hld1name) * geoF6spc4name)");
  TGeoVolume *volF7cil1 = new TGeoVolume("volF7cil1name", geoF7cil1, strMedF7cil1);

  //-   put volume
  volF7cil1->SetLineColor(kOrange);
  volF6spc4->AddNode(volF7cil1, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  //-
  //--------------


}

