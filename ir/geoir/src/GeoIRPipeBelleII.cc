/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors: Clement Ng, Andreas Moll, Hiroshi Nakano                *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

#include <ir/geoir/GeoIRPipeBelleII.h>
#include <ir/simir/IRSensitiveDetector.h>
#include <ir/dataobjects/IRVolumeUserInfo.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GearboxIOAbs.h>
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
#include <TGeoTube.h>
#include <TGeoCone.h>
#include <TGeoTrd1.h>
#include <TGeoCompositeShape.h>

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoIRPipeBelleII regGeoIRPipeBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoIRPipeBelleII::GeoIRPipeBelleII() : CreatorBase("IRPipeBelleII")
{
  setDescription("Creates the TGeo objects for the IR geometry of the Belle II detector.");
}


GeoIRPipeBelleII::~GeoIRPipeBelleII()
{

}


void GeoIRPipeBelleII::create(GearDir& content)
{
  //#################################
  //#   Collect global parameters
  int    GlobalSDFlag = content.getParamIntValue("SDFlag");

  // volIRPipe is aligned and centred on BelleII IP
  TGeoRotation* geoRot = new TGeoRotation("BeamPipeRot", 0.0, 0.0, 0.0);
  TGeoVolumeAssembly* volIRPipe = addSubdetectorGroup("IR", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));
  //volIRPipe->SetVisContainers(1);

  string crown = "";
  if (GlobalSDFlag != 0) {
    //The IR subdetector uses the "SD_" prefix to flag its sensitive volumes
    addSensitiveDetector("SD_", new IRSensitiveDetector("IRSensitiveDetector"));
    crown = "SD_";
  }
  // Unit::rad = 1, Unit::deg = 0.0174533 = pi/180
  //#
  //#################################

  //######## variables ##########
  double zpos;
  double ctubradius, ctubhalflen;
  double ctubsin1, ctubcos1;
  double ctubsin2, ctubcos2;
  double coneradius1, coneradius2, conehalflen;
  double marginlength = 0.0001; //[cm] = 1 um

  /*########## Index ##########
  # "IP pipe" -9.0 cm to 13.85 cm
  #     SUS pipe
  #         Paraffin
  #         Outer Be pipe
  #         Inner Be pipe
  #         Vacuum
  # "crotch pipe Forward " 13.85 cm to 40.0 cm
  #     Ta pipe
  #         Vacuum
  # "crotch pipe Backward " -9.0 cm to -40.0 cm
  #     Ta pipe
  #         Vacuum
  # "beam pipe more Forward " 40.0 cm to 63.35 cm
  #     Ta pipe
  #         Vacuum
  # "beam pipe more Backward " -40.0 cm to -62.725 cm
  #     Ta pipe
  #         Vacuum
  ############################*/

  //==========
  //- IP pipe

  //----------
  //- Lv1. SUS pipe

  //get parameters from .xml file
  GearDir cLv1SUS(content, "Lv1SUS/");
  double Lv1SUSL[16];
  for (int tmpn = 0; tmpn < 16; tmpn++) {
    Lv1SUSL[tmpn] = cLv1SUS.getParamLength((format("L%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv1SUSR[11];
  for (int tmpn = 0; tmpn < 11; tmpn++) {
    Lv1SUSR[tmpn] = cLv1SUS.getParamLength((format("R%1%") % (tmpn + 1)).str().c_str());
  }
  string strMatLv1SUS = cLv1SUS.getParamString("Material");
  TGeoMedium* strMedLv1SUS = gGeoManager->GetMedium(strMatLv1SUS.c_str());

  //define geometry
  TGeoPcon* geoLv1SUSpcon = new TGeoPcon(0, 360, 21);//### NEW! ###
  zpos = 0.0;
  for (int tmpn = 0; tmpn < 8; tmpn++) {
    zpos -= Lv1SUSL[tmpn];
  }
  //B2INFO("@Lv1SUSL Zpos starts from " << zpos);
  geoLv1SUSpcon->DefineSection(0, zpos, 0, Lv1SUSR[0]);
  zpos += Lv1SUSL[0];
  geoLv1SUSpcon->DefineSection(1, zpos, 0, Lv1SUSR[0]);
  geoLv1SUSpcon->DefineSection(2, zpos, 0, Lv1SUSR[1]);
  zpos += Lv1SUSL[1];
  geoLv1SUSpcon->DefineSection(3, zpos, 0, Lv1SUSR[1]);
  zpos += Lv1SUSL[2];
  geoLv1SUSpcon->DefineSection(4, zpos, 0, Lv1SUSR[2]);
  geoLv1SUSpcon->DefineSection(5, zpos, 0, Lv1SUSR[3]);
  zpos += Lv1SUSL[3];
  geoLv1SUSpcon->DefineSection(6, zpos, 0, Lv1SUSR[3]);
  zpos += Lv1SUSL[4];
  geoLv1SUSpcon->DefineSection(7, zpos, 0, Lv1SUSR[4]);
  zpos += Lv1SUSL[5];
  geoLv1SUSpcon->DefineSection(8, zpos, 0, Lv1SUSR[4]);
  zpos += Lv1SUSL[6];
  geoLv1SUSpcon->DefineSection(9, zpos, 0, Lv1SUSR[5]);
  zpos += Lv1SUSL[7];
  geoLv1SUSpcon->DefineSection(10, zpos, 0, Lv1SUSR[5]);
  zpos += Lv1SUSL[8];
  geoLv1SUSpcon->DefineSection(11, zpos, 0, Lv1SUSR[5]);
  zpos += Lv1SUSL[9];
  geoLv1SUSpcon->DefineSection(12, zpos, 0, Lv1SUSR[6]);
  zpos += Lv1SUSL[10];
  geoLv1SUSpcon->DefineSection(13, zpos, 0, Lv1SUSR[6]);
  zpos += Lv1SUSL[11];
  geoLv1SUSpcon->DefineSection(14, zpos, 0, Lv1SUSR[7]);
  zpos += Lv1SUSL[12];
  geoLv1SUSpcon->DefineSection(15, zpos, 0, Lv1SUSR[7]);
  geoLv1SUSpcon->DefineSection(16, zpos, 0, Lv1SUSR[8]);
  zpos += Lv1SUSL[13];
  geoLv1SUSpcon->DefineSection(17, zpos, 0, Lv1SUSR[9]);
  zpos += Lv1SUSL[14];
  geoLv1SUSpcon->DefineSection(18, zpos, 0, Lv1SUSR[9]);
  geoLv1SUSpcon->DefineSection(19, zpos, 0, Lv1SUSR[10]);
  zpos += Lv1SUSL[15];
  geoLv1SUSpcon->DefineSection(20, zpos, 0, Lv1SUSR[10]);
  //B2INFO("@Lv1SUSL Zpos ends at " << zpos);
  geoLv1SUSpcon->SetName("geoLv1SUSpconname");         //### SET! ###

  //-   define volume
  TGeoVolume *volLv1SUS = new TGeoVolume((format("%1%volLv1SUSname") % crown).str().c_str(), geoLv1SUSpcon, strMedLv1SUS);

  //-   put volume
  volLv1SUS->SetLineColor(kGray + 2);
  volIRPipe->AddNode(volLv1SUS, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Outer Be pipe

  //get parameters from .xml file
  GearDir cLv2OutBe(content, "Lv2OutBe/");
  double Lv2OutBeL1 = cLv2OutBe.getParamLength("L1");
  double Lv2OutBeL2 = cLv2OutBe.getParamLength("L2");
  double Lv2OutBeR1 = cLv2OutBe.getParamLength("R1");
  double Lv2OutBeR2 = cLv2OutBe.getParamLength("R2");
  string strMatLv2OutBe = cLv2OutBe.getParamString("Material");
  TGeoMedium* strMedLv2OutBe = gGeoManager->GetMedium(strMatLv2OutBe.c_str());

  //define geometry
  TGeoPcon* geoLv2OutBepcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -Lv2OutBeL1;
  geoLv2OutBepcon->DefineSection(0, zpos, Lv2OutBeR1, Lv2OutBeR2);
  zpos = Lv2OutBeL2;
  geoLv2OutBepcon->DefineSection(1, zpos, Lv2OutBeR1, Lv2OutBeR2);
  geoLv2OutBepcon->SetName("geoLv2OutBepconname");         //### SET! ###

  //-   define volume
  TGeoVolume *volLv2OutBe = new TGeoVolume("volLv2OutBename", geoLv2OutBepcon, strMedLv2OutBe);

  //-   put volume
  volLv2OutBe->SetLineColor(kOrange);
  volLv1SUS->AddNode(volLv2OutBe, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Inner Be pipe

  //get parameters from .xml file
  GearDir cLv2InBe(content, "Lv2InBe/");
  double Lv2InBeL1 = cLv2InBe.getParamLength("L1");
  double Lv2InBeL2 = cLv2InBe.getParamLength("L2");
  double Lv2InBeR1 = cLv2InBe.getParamLength("R1");
  double Lv2InBeR2 = cLv2InBe.getParamLength("R2");
  string strMatLv2InBe = cLv2InBe.getParamString("Material");
  TGeoMedium* strMedLv2InBe = gGeoManager->GetMedium(strMatLv2InBe.c_str());

  //define geometry
  TGeoPcon* geoLv2InBepcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -Lv2InBeL1;
  geoLv2InBepcon->DefineSection(0, zpos, Lv2InBeR1, Lv2InBeR2);
  zpos = Lv2InBeL2;
  geoLv2InBepcon->DefineSection(1, zpos, Lv2InBeR1, Lv2InBeR2);
  geoLv2InBepcon->SetName("geoLv2InBepconname");         //### SET! ###

  //-   define volume
  TGeoVolume *volLv2InBe = new TGeoVolume((format("%1%volLv2InBename") % crown).str().c_str(), geoLv2InBepcon, strMedLv2InBe);

  //-   put volume
  volLv2InBe->SetLineColor(kOrange);
  volLv1SUS->AddNode(volLv2InBe, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Paraffin

  //get parameters from .xml file
  GearDir cLv2Paraf(content, "Lv2Paraf/");
  double Lv2ParafL[20];
  for (int tmpn = 0; tmpn < 20; tmpn++) {
    Lv2ParafL[tmpn] = cLv2Paraf.getParamLength((format("L%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2ParafR[13];
  for (int tmpn = 0; tmpn < 13; tmpn++) {
    Lv2ParafR[tmpn] = cLv2Paraf.getParamLength((boost::format("R%1%") % (tmpn + 1)).str().c_str());
  }
  string strMatLv2Paraf = cLv2Paraf.getParamString("Material");
  TGeoMedium* strMedLv2Paraf = gGeoManager->GetMedium(strMatLv2Paraf.c_str());

  //define geometry
  TGeoPcon* geoLv2Parafpcon1 = new TGeoPcon(0, 360, 20);//### NEW! ###
  TGeoPcon* geoLv2Parafpcon2 = new TGeoPcon(0, 360, 3);//### NEW! ###
  zpos = 0.0;
  for (int tmpn = 0; tmpn < 9; tmpn++) {
    zpos -= Lv2ParafL[tmpn];
  }
  geoLv2Parafpcon1->DefineSection(0, zpos, Lv2ParafR[0], Lv2ParafR[1]);
  zpos += Lv2ParafL[0];
  geoLv2Parafpcon1->DefineSection(1, zpos, Lv2ParafR[0], Lv2ParafR[1]);
  geoLv2Parafpcon1->DefineSection(2, zpos, Lv2ParafR[0], Lv2ParafR[2]);
  zpos += Lv2ParafL[1];
  geoLv2Parafpcon1->DefineSection(3, zpos, Lv2ParafR[0], Lv2ParafR[3]);
  zpos += Lv2ParafL[2];
  geoLv2Parafpcon1->DefineSection(4, zpos, Lv2ParafR[0], Lv2ParafR[3]);
  zpos += Lv2ParafL[3];
  geoLv2Parafpcon1->DefineSection(5, zpos, Lv2ParafR[0], Lv2ParafR[4]);
  zpos += Lv2ParafL[4];
  geoLv2Parafpcon1->DefineSection(6, zpos, Lv2ParafR[0], Lv2ParafR[4]);
  zpos += Lv2ParafL[5];
  geoLv2Parafpcon1->DefineSection(7, zpos, Lv2ParafR[5], Lv2ParafR[4]);
  zpos += Lv2ParafL[6];
  geoLv2Parafpcon1->DefineSection(8, zpos, Lv2ParafR[5], Lv2ParafR[4]);
  zpos += Lv2ParafL[7];
  geoLv2Parafpcon1->DefineSection(9, zpos, Lv2ParafR[5], Lv2ParafR[6]);
  zpos += Lv2ParafL[8] + Lv2ParafL[9];
  geoLv2Parafpcon1->DefineSection(10, zpos, Lv2ParafR[5], Lv2ParafR[6]);
  zpos += Lv2ParafL[10];
  geoLv2Parafpcon1->DefineSection(11, zpos, Lv2ParafR[5], Lv2ParafR[7]);
  zpos += Lv2ParafL[11];
  geoLv2Parafpcon1->DefineSection(12, zpos, Lv2ParafR[5], Lv2ParafR[7]);
  zpos += Lv2ParafL[12];
  geoLv2Parafpcon1->DefineSection(13, zpos, Lv2ParafR[8], Lv2ParafR[7]);
  zpos += Lv2ParafL[13];
  geoLv2Parafpcon1->DefineSection(14, zpos, Lv2ParafR[8], Lv2ParafR[7]);
  zpos += Lv2ParafL[14] + Lv2ParafL[15];
  geoLv2Parafpcon1->DefineSection(15, zpos, Lv2ParafR[8], Lv2ParafR[9]);
  zpos += Lv2ParafL[16] + Lv2ParafL[17];
  geoLv2Parafpcon1->DefineSection(16, zpos, Lv2ParafR[8], Lv2ParafR[9]);
  zpos += Lv2ParafL[18];
  geoLv2Parafpcon1->DefineSection(17, zpos, Lv2ParafR[8], Lv2ParafR[11]);
  geoLv2Parafpcon1->DefineSection(18, zpos, Lv2ParafR[8], Lv2ParafR[12]);
  zpos += Lv2ParafL[19];
  geoLv2Parafpcon1->DefineSection(19, zpos, Lv2ParafR[8], Lv2ParafR[12]);
  geoLv2Parafpcon1->SetName("geoLv2Parafpcon1name");         //### SET! ###
  //
  zpos = 0.0;
  for (int tmpn = 9; tmpn <= 14; tmpn++) {
    zpos += Lv2ParafL[tmpn];
  }

  geoLv2Parafpcon2->DefineSection(0, zpos, 0, Lv2ParafR[8]);
  zpos += Lv2ParafL[15] + Lv2ParafL[16];
  geoLv2Parafpcon2->DefineSection(1, zpos, 0, Lv2ParafR[10]);
  zpos += Lv2ParafL[17] + Lv2ParafL[18] + Lv2ParafL[19] + 1.0;
  geoLv2Parafpcon2->DefineSection(2, zpos, 0, Lv2ParafR[10]);
  geoLv2Parafpcon2->SetName("geoLv2Parafpcon2name");         //### SET! ###

  //-   define volume
  TGeoCompositeShape* geoLv2Paraf = new TGeoCompositeShape("geoLv2Parafname", "geoLv2Parafpcon1name - geoLv2Parafpcon2name");
  TGeoVolume *volLv2Paraf = new TGeoVolume("volLv2Parafname", geoLv2Paraf, strMedLv2Paraf);

  //-   put volume
  volLv2Paraf->SetLineColor(kCyan);
  volLv1SUS->AddNode(volLv2Paraf, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Vacuum

  //get parameters from .xml file
  GearDir cLv2Vacuum(content, "Lv2Vacuum/");
  double Lv2VacuumL1 = cLv2Vacuum.getParamLength("L1");
  double Lv2VacuumL2 = cLv2Vacuum.getParamLength("L2");
  double Lv2VacuumL3 = cLv2Vacuum.getParamLength("L3");
  double Lv2VacuumL4 = cLv2Vacuum.getParamLength("L4");
  double Lv2VacuumR1 = cLv2Vacuum.getParamLength("R1");
  double Lv2VacuumR2 = cLv2Vacuum.getParamLength("R2");
  double Lv2VacuumR3 = cLv2Vacuum.getParamLength("R3");
  double Lv2VacuumA1 = cLv2Vacuum.getParamAngle("A1");
  double Lv2VacuumA2 = cLv2Vacuum.getParamAngle("A2");
  string strMatLv2Vacuum = cLv2Vacuum.getParamString("Material");
  TGeoMedium* strMedLv2Vacuum = gGeoManager->GetMedium(strMatLv2Vacuum.c_str());

  //define geometry
  // ##### part1 start #####
  TGeoPcon* geoLv2Vacuumpart1 = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -Lv2VacuumL1;
  geoLv2Vacuumpart1->DefineSection(0, zpos, 0.0, Lv2VacuumR1);
  zpos = Lv2VacuumL2 + marginlength;
  geoLv2Vacuumpart1->DefineSection(1, zpos, 0.0, Lv2VacuumR1);
  geoLv2Vacuumpart1->SetName("geoLv2Vacuumpart1name");  //### SET! ###
  // ##### part1 end #####
  // ##### part2 start #####
  ctubradius = Lv2VacuumR2;
  ctubhalflen = Lv2VacuumL3 / 2.0;
  ctubcos1 = cos(Lv2VacuumA2);
  ctubsin1 = sin(Lv2VacuumA2);
  ctubcos2 = 1;
  ctubsin2 = 0;
  TGeoCtub* geoLv2Vacuumpart2 = new TGeoCtub(0, ctubradius, ctubhalflen,
                                             0, 360,
                                             ctubsin1, 0, -ctubcos1,
                                             ctubsin2, 0, ctubcos2);//### NEW! ###
  geoLv2Vacuumpart2->SetName("geoLv2Vacuumpart2name");              //### SET! ###
  TVector3 posLv2Vacuumpart2(ctubhalflen*sin(Lv2VacuumA2), 0., Lv2VacuumL2 + ctubhalflen*cos(Lv2VacuumA2));
  TGeoRotation* rotLv2Vacuumpart2 = new TGeoRotation("rotLv2Vacuumpart2name", 90., Lv2VacuumA2 / Unit::deg , -90.);
  TGeoCombiTrans* transLv2Vacuumpart2 = new TGeoCombiTrans("transLv2Vacuumpart2name", posLv2Vacuumpart2.x(), 0.0, posLv2Vacuumpart2.z(), rotLv2Vacuumpart2);
  transLv2Vacuumpart2->RegisterYourself();
  // ##### part2 end #####
  // ##### part3 start #####
  TGeoPcon* geoLv2Vacuumpart3 = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = 0.0;
  geoLv2Vacuumpart3->DefineSection(0, zpos, 0.0, Lv2VacuumR3);
  zpos += Lv2VacuumL4;
  geoLv2Vacuumpart3->DefineSection(1, zpos, 0.0, Lv2VacuumR3);
  geoLv2Vacuumpart3->SetName("geoLv2Vacuumpart3name");
  TGeoRotation* rotLv2Vacuumpart3 = new TGeoRotation("rotLv2Vacuumpart3name", 90., -Lv2VacuumA1 / Unit::deg , -90.);
  rotLv2Vacuumpart3->RegisterYourself();
  // ##### part3 end #####

  //-   define volume
  TGeoCompositeShape* geoLv2Vacuum = new TGeoCompositeShape("geoLv2Vacuumname",
                                                            "( geoLv2Vacuumpart1name + geoLv2Vacuumpart2name:transLv2Vacuumpart2name + geoLv2Vacuumpart3name:rotLv2Vacuumpart3name ) * geoLv1SUSpconname");
  TGeoVolume *volLv2Vacuum = new TGeoVolume("volLv2Vacuumname", geoLv2Vacuum, strMedLv2Vacuum);
  volLv2Vacuum->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2Vacuum->SetLineColor(kGray);
  volLv1SUS->AddNode(volLv2Vacuum, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv3. AuCoat

  //get parameters from .xml file
  GearDir cLv3AuCoat(content, "Lv3AuCoat/");
  double Lv3AuCoatL1 = cLv3AuCoat.getParamLength("L1");
  double Lv3AuCoatL2 = cLv3AuCoat.getParamLength("L2");
  double Lv3AuCoatR1 = cLv3AuCoat.getParamLength("R1");
  double Lv3AuCoatR2 = cLv3AuCoat.getParamLength("R2");
  string strMatLv3AuCoat = cLv3AuCoat.getParamString("Material");
  TGeoMedium* strMedLv3AuCoat = gGeoManager->GetMedium(strMatLv3AuCoat.c_str());

  //define geometry
  TGeoPcon* geoLv3AuCoatpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -Lv3AuCoatL1;
  geoLv3AuCoatpcon->DefineSection(0, zpos, Lv3AuCoatR1, Lv3AuCoatR2);
  zpos = Lv3AuCoatL2;
  geoLv3AuCoatpcon->DefineSection(1, zpos, Lv3AuCoatR1, Lv3AuCoatR2);
  geoLv3AuCoatpcon->SetName("geoLv3AuCoatpconname");         //### SET! ###

  //-   define volume
  TGeoVolume *volLv3AuCoat = new TGeoVolume("volLv3AuCoatname", geoLv3AuCoatpcon, strMedLv3AuCoat);

  //-   put volume
  volLv3AuCoat->SetLineColor(kOrange);
  volLv2Vacuum->AddNode(volLv3AuCoat, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------
  if (GlobalSDFlag >= 10) {
    TGeoTube* geoIPchecker = new TGeoTube(0, 1.0 - (Lv3AuCoatR2 - Lv3AuCoatR1), 0.001 / 10.);
    string strMatIPchecker = "Vacuum";
    TGeoMedium* strMedIPchecker = gGeoManager->GetMedium(strMatIPchecker.c_str());
    TGeoVolume *volIPchecker = new TGeoVolume("SD_volIPcheckername", geoIPchecker, strMedIPchecker);
    volIPchecker->SetLineColor(kRed);
    volLv2Vacuum->AddNode(volIPchecker, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  }

  //=
  //==========

  //==========
  //= Ta pipe Forward

  //----------
  //- Lv1. Ta pipe Forward

  //get parameters from .xml file
  GearDir cLv1TaFwd(content, "Lv1TaFwd/");
  double Lv1TaFwdD1 = cLv1TaFwd.getParamLength("D1");
  double Lv1TaFwdL1 = cLv1TaFwd.getParamLength("L1");
  double Lv1TaFwdL2 = cLv1TaFwd.getParamLength("L2");
  double Lv1TaFwdL3 = cLv1TaFwd.getParamLength("L3");
  double Lv1TaFwdL4 = cLv1TaFwd.getParamLength("L4");
  double Lv1TaFwdT1 = cLv1TaFwd.getParamLength("T1");
  double Lv1TaFwdR1 = cLv1TaFwd.getParamLength("R1");
  string strMatLv1TaFwd = cLv1TaFwd.getParamString("Material");
  TGeoMedium* strMedLv1TaFwd = gGeoManager->GetMedium(strMatLv1TaFwd.c_str());

  //define geometry
  // ##### Trapezoidal shape #####
  TGeoTrd1* geoLv1TaFwdtrd = new TGeoTrd1(Lv1TaFwdL2, Lv1TaFwdL3, Lv1TaFwdT1, Lv1TaFwdL1 / 2.0);//### NEW! ###
  geoLv1TaFwdtrd->SetName("geoLv1TaFwdtrdname");
  // ##### Ring shape for subtraction #####
  TGeoPcon* geoLv1TaFwdpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaFwdD1 - 1.0;
  geoLv1TaFwdpcon->DefineSection(0, zpos, Lv1TaFwdR1, 1.5*Lv1TaFwdR1);
  zpos += Lv1TaFwdL4 + 1.0;
  geoLv1TaFwdpcon->DefineSection(1, zpos, Lv1TaFwdR1, 1.5*Lv1TaFwdR1);
  geoLv1TaFwdpcon->SetName("geoLv1TaFwdpconname");

  //-   define volume
  TGeoTranslation* transLv1TaFwd = new TGeoTranslation("transLv1TaFwdname", 0, 0, Lv1TaFwdL1 / 2.0 + Lv1TaFwdD1);
  transLv1TaFwd->RegisterYourself();
  TGeoCompositeShape* geoLv1TaFwd = new TGeoCompositeShape("geoLv1TaFwdname", "geoLv1TaFwdtrdname:transLv1TaFwdname - geoLv1TaFwdpconname");
  TGeoVolume *volLv1TaFwd = new TGeoVolume((format("%1%volLv1TaFwdname") % crown).str().c_str(), geoLv1TaFwd, strMedLv1TaFwd);

  //-   put volume
  volLv1TaFwd->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaFwd, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Vacuum Forward

  //get parameters from .xml file
  GearDir cLv2VacFwd(content, "Lv2VacFwd/");
  double Lv2VacFwdD[3];
  for (int tmpn = 0; tmpn < 3; tmpn++) {
    Lv2VacFwdD[tmpn] = cLv2VacFwd.getParamLength((format("D%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacFwdL[3];
  for (int tmpn = 0; tmpn < 3; tmpn++) {
    Lv2VacFwdL[tmpn] = cLv2VacFwd.getParamLength((format("L%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacFwdR[4];
  for (int tmpn = 0; tmpn < 4; tmpn++) {
    Lv2VacFwdR[tmpn] = cLv2VacFwd.getParamLength((format("R%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacFwdA[2];
  for (int tmpn = 0; tmpn < 2; tmpn++) {
    Lv2VacFwdA[tmpn] = cLv2VacFwd.getParamAngle((format("A%1%") % (tmpn + 1)).str().c_str());
  }
  string strMatLv2VacFwd = cLv2VacFwd.getParamString("Material");
  TGeoMedium* strMedLv2VacFwd = gGeoManager->GetMedium(strMatLv2VacFwd.c_str());

  //define geometry
  // ##### part1 start #####
  ctubradius = Lv2VacFwdR[0];
  ctubhalflen = sqrt(Lv2VacFwdD[0] * Lv2VacFwdD[0] + Lv2VacFwdD[1] * Lv2VacFwdD[1] - 2.0 * Lv2VacFwdD[0] * Lv2VacFwdD[1] * cos(Lv2VacFwdA[0])) / 2.0;
  //B2INFO("@Lv2VacFwd ctubhalflen " << ctubhalflen);
  ctubcos1 = cos(0.5 * Lv2VacFwdA[1]);
  ctubsin1 = sin(0.5 * Lv2VacFwdA[1]);
  ctubcos2 = cos(0.5 * (Lv2VacFwdA[0] - Lv2VacFwdA[1]));
  ctubsin2 = sin(0.5 * (Lv2VacFwdA[0] - Lv2VacFwdA[1]));
  TGeoCtub* geoLv2VacFwdpart1 = new TGeoCtub(0, ctubradius, ctubhalflen,
                                             0, 360,
                                             ctubsin1, 0, -ctubcos1,
                                             ctubsin2, 0, ctubcos2);//### NEW! ###
  geoLv2VacFwdpart1->SetName("geoLv2VacFwdpart1name");         //### SET! ###
  TVector3 posLv2VacFwdpart1(ctubhalflen*sin(Lv2VacFwdA[1]), 0., Lv2VacFwdD[0] + ctubhalflen*cos(Lv2VacFwdA[1]));
  //B2INFO("@Lv2VacFwdpart1 Position is " << posLv2VacFwdpart1.x() <<" "<< posLv2VacFwdpart1.y() <<" "<< posLv2VacFwdpart1.z() );
  TGeoRotation* rotLv2VacFwdpart1 = new TGeoRotation("rotLv2VacFwdpart1name", 90., Lv2VacFwdA[1] / Unit::deg, -90.);
  TGeoCombiTrans* transLv2VacFwdpart1 = new TGeoCombiTrans("transLv2VacFwdpart1name", posLv2VacFwdpart1.x(), 0.0, posLv2VacFwdpart1.z(), rotLv2VacFwdpart1);
  transLv2VacFwdpart1->RegisterYourself();
  // ##### part1 end #####
  // ##### part2 start #####
  ctubradius = Lv2VacFwdR[1];
  ctubhalflen = Lv2VacFwdL[0] / 2.0;
  ctubcos1 = cos(0.5 * (Lv2VacFwdA[0] - Lv2VacFwdA[1]));
  ctubsin1 = sin(0.5 * (Lv2VacFwdA[0] - Lv2VacFwdA[1]));
  ctubcos2 = 1.0;
  ctubsin2 = 0.0;
  TGeoCtub* geoLv2VacFwdpart2 = new TGeoCtub(0, ctubradius, ctubhalflen,
                                             0, 360,
                                             ctubsin1, 0, -ctubcos1,
                                             ctubsin2, 0, ctubcos2);//### NEW! ###
  geoLv2VacFwdpart2->SetName("geoLv2VacFwdpart2name");         //### SET! ###
  TVector3 posLv2VacFwdpart2((Lv2VacFwdD[1] + ctubhalflen)*sin(Lv2VacFwdA[0]), 0., (Lv2VacFwdD[1] + ctubhalflen)*cos(Lv2VacFwdA[0]));
  //B2INFO("@Lv2VacFwdpart2 Position is " << posLv2VacFwdpart2.x() <<" "<< posLv2VacFwdpart2.y() <<" "<< posLv2VacFwdpart2.z() );
  TGeoRotation* rotLv2VacFwdpart2 = new TGeoRotation("rotLv2VacFwdpart2name", 90., Lv2VacFwdA[0] / Unit::deg, -90.);
  TGeoCombiTrans* transLv2VacFwdpart2 = new TGeoCombiTrans("transLv2VacFwdpart2name", posLv2VacFwdpart2.x(), 0.0, posLv2VacFwdpart2.z(), rotLv2VacFwdpart2);
  transLv2VacFwdpart2->RegisterYourself();
  // ##### part2 end #####
  // ##### part3 start #####
  TGeoPcon* geoLv2VacFwdpart3 = new TGeoPcon(0, 360, 2);
  zpos = 0.0;
  geoLv2VacFwdpart3->DefineSection(0, zpos, 0.0, Lv2VacFwdR[2]);
  zpos = Lv2VacFwdD[2] - Lv2VacFwdL[1] + marginlength;
  geoLv2VacFwdpart3->DefineSection(1, zpos, 0.0, Lv2VacFwdR[2]);
  geoLv2VacFwdpart3->SetName("geoLv2VacFwdpart3name");
  TGeoRotation* rotLv2VacFwdpart3 = new TGeoRotation("rotLv2VacFwdpart3name", 90., -Lv2VacFwdA[0] / Unit::deg , -90.);
  TGeoCombiTrans* transLv2VacFwdpart3 = new TGeoCombiTrans("transLv2VacFwdpart3name", 0.0, 0.0, 0.0, rotLv2VacFwdpart3);
  transLv2VacFwdpart3->RegisterYourself();
  // ##### part3 end #####
  // ##### part4 start #####
  coneradius1 = Lv2VacFwdR[2];
  coneradius2 = Lv2VacFwdR[3];
  conehalflen = Lv2VacFwdL[1] / 2.0;
  TGeoCone* geoLv2VacFwdpart4 = new TGeoCone(conehalflen, 0, coneradius1, 0, coneradius2);
  geoLv2VacFwdpart4->SetName("geoLv2VacFwdpart4name");         //### SET! ###
  TVector3 posLv2VacFwdpart4((Lv2VacFwdD[2] - conehalflen)*sin(-Lv2VacFwdA[0]), 0., (Lv2VacFwdD[2] - conehalflen)*cos(-Lv2VacFwdA[0]));
  //B2INFO("@Lv2VacFwdpart4 Position is " << posLv2VacFwdpart4.x() <<" "<< posLv2VacFwdpart4.y() <<" "<< posLv2VacFwdpart4.z() );
  TGeoRotation* rotLv2VacFwdpart4 = new TGeoRotation("rotLv2VacFwdpart4name", 90., -Lv2VacFwdA[0] / Unit::deg, -90.);
  TGeoCombiTrans* transLv2VacFwdpart4 = new TGeoCombiTrans("transLv2VacFwdpart4name", posLv2VacFwdpart4.x(), 0.0, posLv2VacFwdpart4.z(), rotLv2VacFwdpart4);
  transLv2VacFwdpart4->RegisterYourself();
  // ##### part4 end #####
  // ##### part5 start #####
  coneradius1 = Lv2VacFwdR[3];
  coneradius2 = Lv2VacFwdR[3];
  conehalflen = Lv2VacFwdL[2] / 2.0;
  TGeoCone* geoLv2VacFwdpart5 = new TGeoCone(conehalflen + marginlength, 0, coneradius1, 0, coneradius2);
  geoLv2VacFwdpart5->SetName("geoLv2VacFwdpart5name");         //### SET! ###
  TVector3 posLv2VacFwdpart5((Lv2VacFwdD[2] + conehalflen)*sin(-Lv2VacFwdA[0]), 0., (Lv2VacFwdD[2] + conehalflen)*cos(-Lv2VacFwdA[0]));
  //B2INFO("@Lv2VacFwdpart5 Position is " << posLv2VacFwdpart5.x() <<" "<< posLv2VacFwdpart5.y() <<" "<< posLv2VacFwdpart5.z() );
  TGeoRotation* rotLv2VacFwdpart5 = new TGeoRotation("rotLv2VacFwdpart5name", 90., -Lv2VacFwdA[0] / Unit::deg, -90.);
  TGeoCombiTrans* transLv2VacFwdpart5 = new TGeoCombiTrans("transLv2VacFwdpart5name", posLv2VacFwdpart5.x(), 0.0, posLv2VacFwdpart5.z(), rotLv2VacFwdpart5);
  transLv2VacFwdpart5->RegisterYourself();
  // ##### part5 end #####

  //-   define volume
  TGeoCompositeShape* geoLv2VacFwd = new TGeoCompositeShape("geoLv2VacFwdname",
                                                            "( geoLv2VacFwdpart1name:transLv2VacFwdpart1name + geoLv2VacFwdpart2name:transLv2VacFwdpart2name + geoLv2VacFwdpart3name:transLv2VacFwdpart3name + geoLv2VacFwdpart4name:transLv2VacFwdpart4name + geoLv2VacFwdpart5name:transLv2VacFwdpart5name ) * geoLv1TaFwdname");
  TGeoVolume *volLv2VacFwd = new TGeoVolume("volLv2VacFwdname", geoLv2VacFwd, strMedLv2VacFwd);
  volLv2VacFwd->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacFwd->SetLineColor(kGray);
  volLv1TaFwd->AddNode(volLv2VacFwd, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //=
  //==========

  //==========
  //= Ta pipe Backward

  //----------
  //- Lv1. Ta pipe Backward

  //get parameters from .xml file
  GearDir cLv1TaBwd(content, "Lv1TaBwd/");
  double Lv1TaBwdD1 = cLv1TaBwd.getParamLength("D1");
  double Lv1TaBwdL1 = cLv1TaBwd.getParamLength("L1");
  double Lv1TaBwdL2 = cLv1TaBwd.getParamLength("L2");
  double Lv1TaBwdL3 = cLv1TaBwd.getParamLength("L3");
  double Lv1TaBwdL4 = cLv1TaBwd.getParamLength("L4");
  double Lv1TaBwdT1 = cLv1TaBwd.getParamLength("T1");
  double Lv1TaBwdR1 = cLv1TaBwd.getParamLength("R1");
  string strMatLv1TaBwd = cLv1TaBwd.getParamString("Material");
  TGeoMedium* strMedLv1TaBwd = gGeoManager->GetMedium(strMatLv1TaBwd.c_str());

  //define geometry
  // ##### Trapezoidal shape #####
  TGeoTrd1* geoLv1TaBwdtrd = new TGeoTrd1(Lv1TaBwdL2, Lv1TaBwdL3, Lv1TaBwdT1, Lv1TaBwdL1 / 2.0);//### NEW! ###
  geoLv1TaBwdtrd->SetName("geoLv1TaBwdtrdname");
  // ##### conical shape for subtraction #####
  TGeoPcon* geoLv1TaBwdpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -Lv1TaBwdD1 - Lv1TaBwdL4;
  geoLv1TaBwdpcon->DefineSection(0, zpos, Lv1TaBwdT1, 1.5*Lv1TaBwdT1);
  zpos += Lv1TaBwdL4 + 1.0;
  geoLv1TaBwdpcon->DefineSection(1, zpos, Lv1TaBwdR1, 1.5*Lv1TaBwdT1);
  geoLv1TaBwdpcon->SetName("geoLv1TaBwdpconname");

  //-   define volume
  TGeoTranslation* transLv1TaBwd = new TGeoTranslation("transLv1TaBwdname", 0, 0, -Lv1TaBwdL1 / 2.0 - Lv1TaBwdD1);
  transLv1TaBwd->RegisterYourself();
  TGeoCompositeShape* geoLv1TaBwd = new TGeoCompositeShape("geoLv1TaBwdname", "geoLv1TaBwdtrdname:transLv1TaBwdname - geoLv1TaBwdpconname");
  TGeoVolume *volLv1TaBwd = new TGeoVolume((format("%1%volLv1TaBwdname") % crown).str().c_str(), geoLv1TaBwd, strMedLv1TaBwd);

  //-   put volume
  volLv1TaBwd->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaBwd, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. Vacuum Backward

  //get parameters from .xml file
  GearDir cLv2VacBwd(content, "Lv2VacBwd/");
  double Lv2VacBwdD[3];
  for (int tmpn = 0; tmpn < 3; tmpn++) {
    Lv2VacBwdD[tmpn] = cLv2VacBwd.getParamLength((format("D%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacBwdL[3];
  for (int tmpn = 0; tmpn < 3; tmpn++) {
    Lv2VacBwdL[tmpn] = cLv2VacBwd.getParamLength((format("L%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacBwdR[4];
  for (int tmpn = 0; tmpn < 4; tmpn++) {
    Lv2VacBwdR[tmpn] = cLv2VacBwd.getParamLength((format("R%1%") % (tmpn + 1)).str().c_str());
  }
  double Lv2VacBwdA1 = cLv2VacBwd.getParamAngle("A1");
  string strMatLv2VacBwd = cLv2VacBwd.getParamString("Material");
  TGeoMedium* strMedLv2VacBwd = gGeoManager->GetMedium(strMatLv2VacBwd.c_str());

  //define geometry
  // ##### part1 start #####
  ctubradius = Lv2VacBwdR[0];
  ctubhalflen = sqrt(Lv2VacBwdD[0] * Lv2VacBwdD[0] + Lv2VacBwdD[1] * Lv2VacBwdD[1] - 2.0 * Lv2VacBwdD[0] * Lv2VacBwdD[1] * cos(Lv2VacBwdA1)) / 2.0;
  double Lv2VacBwdA2 = asin(Lv2VacBwdD[1] * sin(Lv2VacBwdA1) / (ctubhalflen * 2.0));
  ctubcos1 = cos(-Lv2VacBwdA2);
  ctubsin1 = sin(-Lv2VacBwdA2);
  ctubcos2 = cos(-0.5 * (Lv2VacBwdA1 - Lv2VacBwdA2));
  ctubsin2 = sin(-0.5 * (Lv2VacBwdA1 - Lv2VacBwdA2));
  TGeoCtub* geoLv2VacBwdpart1 = new TGeoCtub(0, ctubradius, ctubhalflen,
                                             0, 360,
                                             ctubsin1, 0, -ctubcos1,
                                             ctubsin2, 0, ctubcos2);//### NEW! ###
  geoLv2VacBwdpart1->SetName("geoLv2VacBwdpart1name");         //### SET! ###
  TVector3 posLv2VacBwdpart1(Lv2VacBwdD[1]*sin(Lv2VacBwdA1) / 2.0, 0., -(Lv2VacBwdD[1]*cos(Lv2VacBwdA1) + Lv2VacBwdD[0]) / 2.0);
  TGeoRotation* rotLv2VacBwdpart1 = new TGeoRotation("rotLv2VacBwdpart1name", 90., -Lv2VacBwdA2 / Unit::deg + 180.0, -90.);
  TGeoCombiTrans* transLv2VacBwdpart1 = new TGeoCombiTrans("transLv2VacBwdpart1name", posLv2VacBwdpart1.x(), 0.0, posLv2VacBwdpart1.z(), rotLv2VacBwdpart1);
  transLv2VacBwdpart1->RegisterYourself();
  // ##### part1 end #####
  // ##### part2 start #####
  ctubradius = Lv2VacBwdR[1];
  ctubhalflen = Lv2VacBwdL[0] / 2.0;
  ctubcos1 = cos(-0.5 * (Lv2VacBwdA1 - Lv2VacBwdA2));
  ctubsin1 = sin(-0.5 * (Lv2VacBwdA1 - Lv2VacBwdA2));
  ctubcos2 = 1.0;
  ctubsin2 = 0.0;
  TGeoCtub* geoLv2VacBwdpart2 = new TGeoCtub(0, ctubradius, ctubhalflen + marginlength,
                                             0, 360,
                                             ctubsin1, 0, -ctubcos1,
                                             ctubsin2, 0, ctubcos2);//### NEW! ###
  geoLv2VacBwdpart2->SetName("geoLv2VacBwdpart2name");         //### SET! ###
  TVector3 posLv2VacBwdpart2((Lv2VacBwdD[1] + ctubhalflen)*sin(Lv2VacBwdA1), 0., -(Lv2VacBwdD[1] + ctubhalflen)*cos(Lv2VacBwdA1));
  TGeoRotation* rotLv2VacBwdpart2 = new TGeoRotation("rotLv2VacBwdpart2name", 90., -Lv2VacBwdA1 / Unit::deg + 180.0, -90.);
  TGeoCombiTrans* transLv2VacBwdpart2 = new TGeoCombiTrans("transLv2VacBwdpart2name", posLv2VacBwdpart2.x(), 0.0, posLv2VacBwdpart2.z(), rotLv2VacBwdpart2);
  transLv2VacBwdpart2->RegisterYourself();
  // ##### part2 end #####
  // ##### part3 start #####
  TGeoPcon* geoLv2VacBwdpart3 = new TGeoPcon(0, 360, 2);
  zpos = -Lv2VacBwdD[2] + Lv2VacBwdL[1] - marginlength;
  geoLv2VacBwdpart3->DefineSection(0, zpos, 0.0, Lv2VacBwdR[2]);
  zpos = 0.0;
  geoLv2VacBwdpart3->DefineSection(1, zpos, 0.0, Lv2VacBwdR[2]);
  geoLv2VacBwdpart3->SetName("geoLv2VacBwdpart3name");
  TGeoRotation* rotLv2VacBwdpart3 = new TGeoRotation("rotLv2VacBwdpart3name", 90., Lv2VacBwdA1 / Unit::deg , -90.);
  TGeoCombiTrans* transLv2VacBwdpart3 = new TGeoCombiTrans("transLv2VacBwdpart3name", 0.0, 0.0, 0.0, rotLv2VacBwdpart3);
  transLv2VacBwdpart3->RegisterYourself();
  // ##### part3 end #####
  // ##### part4 start #####
  coneradius1 = Lv2VacBwdR[2];
  coneradius2 = Lv2VacBwdR[3];
  conehalflen = Lv2VacBwdL[1] / 2.0;
  TGeoCone* geoLv2VacBwdpart4 = new TGeoCone(conehalflen, 0, coneradius1, 0, coneradius2);
  geoLv2VacBwdpart4->SetName("geoLv2VacBwdpart4name");         //### SET! ###
  TVector3 posLv2VacBwdpart4((Lv2VacBwdD[2] - conehalflen)*sin(-Lv2VacBwdA1), 0., -(Lv2VacBwdD[2] - conehalflen)*cos(-Lv2VacBwdA1));
  TGeoRotation* rotLv2VacBwdpart4 = new TGeoRotation("rotLv2VacBwdpart4name", 90., Lv2VacBwdA1 / Unit::deg + 180.0, -90.);
  TGeoCombiTrans* transLv2VacBwdpart4 = new TGeoCombiTrans("transLv2VacBwdpart4name", posLv2VacBwdpart4.x(), 0.0, posLv2VacBwdpart4.z(), rotLv2VacBwdpart4);
  transLv2VacBwdpart4->RegisterYourself();
  // ##### part4 end #####
  // ##### test part5 start #####
  TGeoPcon* geoLv2VacBwdpart5 = new TGeoPcon(0, 360, 2);
  zpos = -Lv2VacBwdD[2] - Lv2VacBwdL[2];
  geoLv2VacBwdpart5->DefineSection(0, zpos, 0.0, Lv2VacBwdR[3]);
  zpos = -Lv2VacBwdD[2] + marginlength;
  geoLv2VacBwdpart5->DefineSection(1, zpos, 0.0, Lv2VacBwdR[3]);
  geoLv2VacBwdpart5->SetName("geoLv2VacBwdpart5name");
  TGeoRotation* rotLv2VacBwdpart5 = new TGeoRotation("rotLv2VacBwdpart5name", 90., Lv2VacBwdA1 / Unit::deg , -90.);
  TGeoCombiTrans* transLv2VacBwdpart5 = new TGeoCombiTrans("transLv2VacBwdpart5name", 0.0, 0.0, 0.0, rotLv2VacBwdpart5);
  transLv2VacBwdpart5->RegisterYourself();
  // ##### test part5 end #####

  //-   define volume
  TGeoCompositeShape* geoLv2VacBwd = new TGeoCompositeShape("geoLv2VacBwdname",
                                                            "( geoLv2VacBwdpart1name:transLv2VacBwdpart1name + geoLv2VacBwdpart2name:transLv2VacBwdpart2name + geoLv2VacBwdpart3name:transLv2VacBwdpart3name + geoLv2VacBwdpart4name:transLv2VacBwdpart4name + geoLv2VacBwdpart5name:transLv2VacBwdpart5name ) * geoLv1TaBwdname");
  TGeoVolume *volLv2VacBwd = new TGeoVolume("volLv2VacBwdname", geoLv2VacBwd, strMedLv2VacBwd);
  volLv2VacBwd->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacBwd->SetLineColor(kGray);
  volLv1TaBwd->AddNode(volLv2VacBwd, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //=
  //==========

  //==========
  //= beam pipe Forward Forward

  //get parameters from .xml file
  GearDir cAreaTubeFwd(content, "AreaTubeFwd/");
  double AreaTubeFwdR1 = cAreaTubeFwd.getParamLength("R1");
  double AreaTubeFwdD1 = cAreaTubeFwd.getParamLength("D1");
  double AreaTubeFwdD2 = cAreaTubeFwd.getParamLength("D2");

  //define geometry
  TGeoPcon* geoAreaTubeFwdpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = AreaTubeFwdD1;
  geoAreaTubeFwdpcon->DefineSection(0, zpos, 0.0, AreaTubeFwdR1);
  zpos = AreaTubeFwdD2;
  geoAreaTubeFwdpcon->DefineSection(1, zpos, 0.0, AreaTubeFwdR1);
  geoAreaTubeFwdpcon->SetName("geoAreaTubeFwdpconname");         //### SET! ###

  //----------
  //- Lv1. LER pipe Forward Forward

  //get parameters from .xml file
  GearDir cLv1TaLERUp(content, "Lv1TaLERUp/");
  double Lv1TaLERUpR1 = cLv1TaLERUp.getParamLength("R1");
  double Lv1TaLERUpL1 = cLv1TaLERUp.getParamLength("L1");
  double Lv1TaLERUpL2 = cLv1TaLERUp.getParamLength("L2");
  double Lv1TaLERUpA1 = cLv1TaLERUp.getParamAngle("A1");
  string strMatLv1TaLERUp = cLv1TaLERUp.getParamString("Material");
  TGeoMedium* strMedLv1TaLERUp = gGeoManager->GetMedium(strMatLv1TaLERUp.c_str());

  //define geometry
  TGeoPcon* geoLv1TaLERUppcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaLERUpL1;
  geoLv1TaLERUppcon->DefineSection(0, zpos, 0.0, Lv1TaLERUpR1);
  zpos = Lv1TaLERUpL2;
  geoLv1TaLERUppcon->DefineSection(1, zpos, 0.0, Lv1TaLERUpR1);
  geoLv1TaLERUppcon->SetName("geoLv1TaLERUppconname");         //### SET! ###
  TGeoRotation* rotLv1TaLERUp = new TGeoRotation("rotLv1TaLERUpname", 90., Lv1TaLERUpA1 / Unit::deg , -90.);
  rotLv1TaLERUp->RegisterYourself();

  //-   define volume
  TGeoCompositeShape* geoLv1TaLERUp = new TGeoCompositeShape("geoLv1TaLERUpname",
                                                             "geoLv1TaLERUppconname:rotLv1TaLERUpname * geoAreaTubeFwdpconname");
  TGeoVolume *volLv1TaLERUp = new TGeoVolume((format("%1%volLv1TaLERUpname") % crown).str().c_str(), geoLv1TaLERUp, strMedLv1TaLERUp);

  //-   put volume
  volLv1TaLERUp->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaLERUp, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. LER pipe Forward Forward

  //get parameters from .xml file
  GearDir cLv2VacLERUp(content, "Lv2VacLERUp/");
  double Lv2VacLERUpR1 = cLv2VacLERUp.getParamLength("R1");
  string strMatLv2VacLERUp = cLv2VacLERUp.getParamString("Material");
  TGeoMedium* strMedLv2VacLERUp = gGeoManager->GetMedium(strMatLv2VacLERUp.c_str());

  //define geometry
  TGeoPcon* geoLv2VacLERUppcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaLERUpL1;
  geoLv2VacLERUppcon->DefineSection(0, zpos, 0.0, Lv2VacLERUpR1);
  zpos = Lv1TaLERUpL2;
  geoLv2VacLERUppcon->DefineSection(1, zpos, 0.0, Lv2VacLERUpR1);
  geoLv2VacLERUppcon->SetName("geoLv2VacLERUppconname");         //### SET! ###

  //-   define volume
  TGeoCompositeShape* geoLv2VacLERUp = new TGeoCompositeShape("geoLv2VacLERUpname",
                                                              "geoLv2VacLERUppconname:rotLv1TaLERUpname * geoAreaTubeFwdpconname");
  TGeoVolume *volLv2VacLERUp = new TGeoVolume("volLv2VacLERUpname", geoLv2VacLERUp, strMedLv2VacLERUp);
  volLv2VacLERUp->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacLERUp->SetLineColor(kGray);
  volLv1TaLERUp->AddNode(volLv2VacLERUp, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv1. HER pipe Forward Forward

  //get parameters from .xml file
  GearDir cLv1TaHERDwn(content, "Lv1TaHERDwn/");
  double Lv1TaHERDwnR1 = cLv1TaHERDwn.getParamLength("R1");
  double Lv1TaHERDwnL1 = cLv1TaHERDwn.getParamLength("L1");
  double Lv1TaHERDwnL2 = cLv1TaHERDwn.getParamLength("L2");
  double Lv1TaHERDwnA1 = cLv1TaHERDwn.getParamAngle("A1");
  string strMatLv1TaHERDwn = cLv1TaHERDwn.getParamString("Material");
  TGeoMedium* strMedLv1TaHERDwn = gGeoManager->GetMedium(strMatLv1TaHERDwn.c_str());

  //define geometry
  TGeoPcon* geoLv1TaHERDwnpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaHERDwnL1;
  geoLv1TaHERDwnpcon->DefineSection(0, zpos, 0.0, Lv1TaHERDwnR1);
  zpos = Lv1TaHERDwnL2;
  geoLv1TaHERDwnpcon->DefineSection(1, zpos, 0.0, Lv1TaHERDwnR1);
  geoLv1TaHERDwnpcon->SetName("geoLv1TaHERDwnpconname");         //### SET! ###
  TGeoRotation* rotLv1TaHERDwn = new TGeoRotation("rotLv1TaHERDwnname", 90., Lv1TaHERDwnA1 / Unit::deg , -90.);
  rotLv1TaHERDwn->RegisterYourself();

  //-   define volume
  TGeoCompositeShape* geoLv1TaHERDwn = new TGeoCompositeShape("geoLv1TaHERDwnname",
                                                              "geoLv1TaHERDwnpconname:rotLv1TaHERDwnname * geoAreaTubeFwdpconname");
  TGeoVolume *volLv1TaHERDwn = new TGeoVolume((format("%1%volLv1TaHERDwnname") % crown).str().c_str(), geoLv1TaHERDwn, strMedLv1TaHERDwn);

  //-   put volume
  volLv1TaHERDwn->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaHERDwn, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. HER pipe Forward Forward

  //get parameters from .xml file
  GearDir cLv2VacHERDwn(content, "Lv2VacHERDwn/");
  double Lv2VacHERDwnR1 = cLv2VacHERDwn.getParamLength("R1");
  string strMatLv2VacHERDwn = cLv2VacHERDwn.getParamString("Material");
  TGeoMedium* strMedLv2VacHERDwn = gGeoManager->GetMedium(strMatLv2VacHERDwn.c_str());

  //define geometry
  TGeoPcon* geoLv2VacHERDwnpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaHERDwnL1;
  geoLv2VacHERDwnpcon->DefineSection(0, zpos, 0.0, Lv2VacHERDwnR1);
  zpos = Lv1TaHERDwnL2;
  geoLv2VacHERDwnpcon->DefineSection(1, zpos, 0.0, Lv2VacHERDwnR1);
  geoLv2VacHERDwnpcon->SetName("geoLv2VacHERDwnpconname");         //### SET! ###

  //-   define volume
  TGeoCompositeShape* geoLv2VacHERDwn = new TGeoCompositeShape("geoLv2VacHERDwnname",
      "geoLv2VacHERDwnpconname:rotLv1TaHERDwnname * geoAreaTubeFwdpconname");
  TGeoVolume *volLv2VacHERDwn = new TGeoVolume("volLv2VacHERDwnname", geoLv2VacHERDwn, strMedLv2VacHERDwn);
  volLv2VacHERDwn->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacHERDwn->SetLineColor(kGray);
  volLv1TaHERDwn->AddNode(volLv2VacHERDwn, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //=
  //==========

  //==========
  //= beam pipe Backward Backward

  //get parameters from .xml file
  GearDir cAreaTubeBwd(content, "AreaTubeBwd/");
  double AreaTubeBwdR1 = cAreaTubeBwd.getParamLength("R1");
  double AreaTubeBwdD1 = cAreaTubeBwd.getParamLength("D1");
  double AreaTubeBwdD2 = cAreaTubeBwd.getParamLength("D2");

  //define geometry
  TGeoPcon* geoAreaTubeBwdpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = -AreaTubeBwdD1;
  geoAreaTubeBwdpcon->DefineSection(0, zpos, 0.0, AreaTubeBwdR1);
  zpos = -AreaTubeBwdD2;
  geoAreaTubeBwdpcon->DefineSection(1, zpos, 0.0, AreaTubeBwdR1);
  geoAreaTubeBwdpcon->SetName("geoAreaTubeBwdpconname");         //### SET! ###

  //----------
  //- Lv1. LER pipe Backward Backward

  //get parameters from .xml file
  GearDir cLv1TaLERDwn(content, "Lv1TaLERDwn/");
  double Lv1TaLERDwnR1 = cLv1TaLERDwn.getParamLength("R1");
  double Lv1TaLERDwnL1 = cLv1TaLERDwn.getParamLength("L1");
  double Lv1TaLERDwnL2 = cLv1TaLERDwn.getParamLength("L2");
  double Lv1TaLERDwnA1 = cLv1TaLERDwn.getParamAngle("A1");
  string strMatLv1TaLERDwn = cLv1TaLERDwn.getParamString("Material");
  TGeoMedium* strMedLv1TaLERDwn = gGeoManager->GetMedium(strMatLv1TaLERDwn.c_str());

  //define geometry
  TGeoPcon* geoLv1TaLERDwnpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaLERDwnL1;
  geoLv1TaLERDwnpcon->DefineSection(0, zpos, 0.0, Lv1TaLERDwnR1);
  zpos = Lv1TaLERDwnL2;
  geoLv1TaLERDwnpcon->DefineSection(1, zpos, 0.0, Lv1TaLERDwnR1);
  geoLv1TaLERDwnpcon->SetName("geoLv1TaLERDwnpconname");         //### SET! ###
  TGeoRotation* rotLv1TaLERDwn = new TGeoRotation("rotLv1TaLERDwnname", 90., Lv1TaLERDwnA1 / Unit::deg + 180., -90.);
  rotLv1TaLERDwn->RegisterYourself();

  //-   define volume
  TGeoCompositeShape* geoLv1TaLERDwn = new TGeoCompositeShape("geoLv1TaLERDwnname",
                                                              "geoLv1TaLERDwnpconname:rotLv1TaLERDwnname * geoAreaTubeBwdpconname");
  TGeoVolume *volLv1TaLERDwn = new TGeoVolume((format("%1%volLv1TaLERDwnname") % crown).str().c_str(), geoLv1TaLERDwn, strMedLv1TaLERDwn);

  //-   put volume
  volLv1TaLERDwn->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaLERDwn, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. LER pipe Backward Backward

  //get parameters from .xml file
  GearDir cLv2VacLERDwn(content, "Lv2VacLERDwn/");
  double Lv2VacLERDwnR1 = cLv2VacLERDwn.getParamLength("R1");
  string strMatLv2VacLERDwn = cLv2VacLERDwn.getParamString("Material");
  TGeoMedium* strMedLv2VacLERDwn = gGeoManager->GetMedium(strMatLv2VacLERDwn.c_str());

  //define geometry
  TGeoPcon* geoLv2VacLERDwnpcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaLERDwnL1;
  geoLv2VacLERDwnpcon->DefineSection(0, zpos, 0.0, Lv2VacLERDwnR1);
  zpos = Lv1TaLERDwnL2;
  geoLv2VacLERDwnpcon->DefineSection(1, zpos, 0.0, Lv2VacLERDwnR1);
  geoLv2VacLERDwnpcon->SetName("geoLv2VacLERDwnpconname");         //### SET! ###

  //-   define volume
  TGeoCompositeShape* geoLv2VacLERDwn = new TGeoCompositeShape("geoLv2VacLERDwnname",
      "geoLv2VacLERDwnpconname:rotLv1TaLERDwnname * geoAreaTubeBwdpconname");
  TGeoVolume *volLv2VacLERDwn = new TGeoVolume("volLv2VacLERDwnname", geoLv2VacLERDwn, strMedLv2VacLERDwn);
  volLv2VacLERDwn->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacLERDwn->SetLineColor(kGray);
  volLv1TaLERDwn->AddNode(volLv2VacLERDwn, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv1. HER pipe Backward Backward

  //get parameters from .xml file
  GearDir cLv1TaHERUp(content, "Lv1TaHERUp/");
  double Lv1TaHERUpR1 = cLv1TaHERUp.getParamLength("R1");
  double Lv1TaHERUpL1 = cLv1TaHERUp.getParamLength("L1");
  double Lv1TaHERUpL2 = cLv1TaHERUp.getParamLength("L2");
  double Lv1TaHERUpA1 = cLv1TaHERUp.getParamAngle("A1");
  string strMatLv1TaHERUp = cLv1TaHERUp.getParamString("Material");
  TGeoMedium* strMedLv1TaHERUp = gGeoManager->GetMedium(strMatLv1TaHERUp.c_str());

  //define geometry
  TGeoPcon* geoLv1TaHERUppcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaHERUpL1;
  geoLv1TaHERUppcon->DefineSection(0, zpos, 0.0, Lv1TaHERUpR1);
  zpos = Lv1TaHERUpL2;
  geoLv1TaHERUppcon->DefineSection(1, zpos, 0.0, Lv1TaHERUpR1);
  geoLv1TaHERUppcon->SetName("geoLv1TaHERUppconname");         //### SET! ###
  TGeoRotation* rotLv1TaHERUp = new TGeoRotation("rotLv1TaHERUpname", 90., Lv1TaHERUpA1 / Unit::deg + 180., -90.);
  rotLv1TaHERUp->RegisterYourself();

  //-   define volume
  TGeoCompositeShape* geoLv1TaHERUp = new TGeoCompositeShape("geoLv1TaHERUpname",
                                                             "geoLv1TaHERUppconname:rotLv1TaHERUpname * geoAreaTubeBwdpconname");
  TGeoVolume *volLv1TaHERUp = new TGeoVolume((format("%1%volLv1TaHERUpname") % crown).str().c_str(), geoLv1TaHERUp, strMedLv1TaHERUp);

  //-   put volume
  volLv1TaHERUp->SetLineColor(kGray + 3);
  volIRPipe->AddNode(volLv1TaHERUp, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //----------
  //- Lv2. HER pipe Backward Backward

  //get parameters from .xml file
  GearDir cLv2VacHERUp(content, "Lv2VacHERUp/");
  double Lv2VacHERUpR1 = cLv2VacHERUp.getParamLength("R1");
  string strMatLv2VacHERUp = cLv2VacHERUp.getParamString("Material");
  TGeoMedium* strMedLv2VacHERUp = gGeoManager->GetMedium(strMatLv2VacHERUp.c_str());

  //define geometry
  TGeoPcon* geoLv2VacHERUppcon = new TGeoPcon(0, 360, 2);//### NEW! ###
  zpos = Lv1TaHERUpL1;
  geoLv2VacHERUppcon->DefineSection(0, zpos, 0.0, Lv2VacHERUpR1);
  zpos = Lv1TaHERUpL2;
  geoLv2VacHERUppcon->DefineSection(1, zpos, 0.0, Lv2VacHERUpR1);
  geoLv2VacHERUppcon->SetName("geoLv2VacHERUppconname");         //### SET! ###

  //-   define volume
  TGeoCompositeShape* geoLv2VacHERUp = new TGeoCompositeShape("geoLv2VacHERUpname",
                                                              "geoLv2VacHERUppconname:rotLv1TaHERUpname * geoAreaTubeBwdpconname");
  TGeoVolume *volLv2VacHERUp = new TGeoVolume("volLv2VacHERUpname", geoLv2VacHERUp, strMedLv2VacHERUp);
  volLv2VacHERUp->SetField(new IRVolumeUserInfo());

  //-   put volume
  volLv2VacHERUp->SetLineColor(kGray);
  volLv1TaHERUp->AddNode(volLv2VacHERUp, 1, new TGeoTranslation(0.0, 0.0, 0.0));

  //-
  //----------

  //=
  //==========
}
