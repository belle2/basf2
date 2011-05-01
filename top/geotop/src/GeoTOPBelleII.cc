/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geotop/GeoTOPBelleII.h>
#include <top/geotop/TOPGeometryPar.h>
#include <top/simtop/TOPSensitiveDetector.h>
#include <top/simtop/TOPSensitiveQuartz.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <geometry/dataobjects/OpticalUserInfo.h>
#include <geometry/utilities/GeoReader.h>
#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TVector2.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoPgon.h>
#include <TGeoTube.h>
#include <TGeoBBox.h>
#include <TGeoArb8.h>
#include <TGeoSphere.h>
#include <TGeoCompositeShape.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoTOPBelleII regGeoTOPBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoTOPBelleII::GeoTOPBelleII() : CreatorBase("TOPBelleII")
{
  setDescription("Creates the TGeo objects for the TOP geometry of the Belle II detector.");
  addSensitiveDetector("SD_", new TOPSensitiveDetector("TOPSensitiveDetector"));
  addSensitiveDetector("SQ_", new TOPSensitiveQuartz("TOPSensitiveQuartz"));
}


GeoTOPBelleII::~GeoTOPBelleII()
{
}

void GeoTOPBelleII::create(GearDir& content)
{
  //----------------------------------------
  //         Get global parameters
  //----------------------------------------

  TOPGeometryPar* topgp = TOPGeometryPar::Instance(); //Object for reding parameters from xml

  double DetectorRotation = topgp->GetDetectorRotation() / Unit::deg;
  double DetectorZShift = topgp->GetDetectorZShift();
  int NumberOfBars = topgp->GetNumberOfBars();
  double DetectorInnerRadius = topgp->GetDetectorInnerRadius();
  double QuartzWidth = topgp->GetQuartzWidth();
  double QuartzExtWidth = topgp->GetQuartzExtWidth();
  double QuartzThickness = topgp->GetQuartzThickness();
  double QuartzBPos = topgp->GetQuartzBPos();
  double QuartzLenSeg1 = topgp->GetQuartzLenSeg1();
  double QuartzLenSeg2 = topgp->GetQuartzLenSeg2();
  double QuartzLenMir = topgp->GetQuartzLenMir();
//!  double QuartzZExt = topgp->GetQuartzZExt();
//!  double QuartzYExtUp = topgp->GetQuartzYExtUp();
//!  double QuartzYExtDown = topgp->GetQuartzYExtDown();
  double GlueThickness1 = topgp->GetGlueThickness1();
  double GlueThickness2 = topgp->GetGlueThickness2();
  double GlueThickness3 = topgp->GetGlueThickness3();
//!  double MirrorCenterX = topgp->GetMirrorCenterX();
//!  double MirrorCenterY = topgp->GetMirrorCenterY();
  double MirrorR = topgp->GetMirrorR();
//!  double GapPMTX = topgp->GetGapPMTX();
//!  double GapPMTY = topgp->GetGapPMTY();
//!  double GapPMTX0 = topgp->GetGapPMTX0();
//!  double GapPMTY0 = topgp->GetGapPMTY0();
//!  int PMTNX = topgp->GetPMTNX();
//!  int PMTNY = topgp->GetPMTNY();
  double PMTSizeX = topgp->GetPMTSizeX();
  double PMTSizeY = topgp->GetPMTSizeY();
  double PMTSizeZ = topgp->GetPMTSizeZ();
  double ActiveSizeX = topgp->GetActiveSizeX();
  double ActiveSizeY = topgp->GetActiveSizeY();
  double ActiveSizeZ = topgp->GetActiveSizeZ();
  double WindowThickness = topgp->GetWindowThickness();
  double BottomThickness = topgp->GetBottomThickness();
  /*  double QBBThickness = topgp->GetQBBThickness();
    double QBBSideThickness = topgp->GetQBBSideThickness();
    double QBBFronThickness = topgp->GetQBBFronThickness();
    double AirGapUp = topgp->GetAirGapUp();
    double AirGapDown = topgp->GetAirGapDown();
    double AirGapSide = topgp->GetAirGapSide();
    double QBBForwardPos = topgp->GetQBBForwardPos();
    double QBBBackwardPos = topgp->GetQBBBackwardPos();
  */
  // Here we directly read the material of the detector since it's the only place we need this

  string barMaterial =  content.getParamString("Bars/BarMaterial");
  string wallMaterial =  content.getParamString("PMTs/Module/wallMaterial");
  string sensMaterial = content.getParamString("PMTs/Module/sensMaterial");
  string winMaterial = content.getParamString("PMTs/Module/winMaterial");
  string botMaterial =  content.getParamString("PMTs/Module/botMaterial");
  string pannelMaterial =  content.getParamString("Support/PannelMaterial");
  string epoxMaterial =  content.getParamString("Bars/Glue/GlueMaterial");

  //----------------------------------------
  //        Add subdetector group
  //----------------------------------------

  TGeoRotation* geoRot = new TGeoRotation("TOPRot", 90.0, DetectorRotation, 0.0);
  TGeoVolumeAssembly* volGrpTOP = addSubdetectorGroup("TOP", new TGeoCombiTrans(0.0, 0.0, DetectorZShift, geoRot));

  /*  TGeoMedium* boxmedium = gGeoManager->GetMedium("TOPAir");
    TGeoTube* contTubeSh = new TGeoTube("contTubeSh", 0, 150, 300);
    TGeoVolume* contTube = new TGeoVolume("contTube", contTubeSh, boxmedium);
    volGrpTOP->AddNode(contTube, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  */
  //-----------------------------------------------------------
  //              Build quartz bar segments and the epox glue
  //
  //            epox                epox                 epox
  //             V                   V                    V
  //         ------------------------------------------
  //             |     segment 1     |     segment 2      |
  //             ------------------------------------------
  //
  //-----------------------------------------------------------

  TGeoMedium* barMedium = gGeoManager->GetMedium(barMaterial.c_str()); // Create the medium from database
  TGeoMedium* epoxMedium = gGeoManager->GetMedium(epoxMaterial.c_str()); // Create the medium from database

  TGeoBBox* seg1 = new TGeoBBox("seg1", QuartzWidth / 2.0, QuartzThickness / 2., QuartzLenSeg1 / 2.);//Make the first segment of the qurtz bar
  TGeoBBox* seg2 = new TGeoBBox("seg2", QuartzWidth / 2.0, QuartzThickness / 2., QuartzLenSeg2 / 2.);//Make the second segment of the qurtz bar


  TGeoBBox* epox1 = new TGeoBBox("epox1", QuartzExtWidth / 2.0, QuartzThickness / 2., GlueThickness1 / 2.);//Make glue between the first and second qurtz segment
  TGeoBBox* epox2 = new TGeoBBox("epox2", QuartzWidth / 2.0, QuartzThickness / 2., GlueThickness2 / 2.);//Make glue between the first and second qurtz segment
  TGeoBBox* epox3 = new TGeoBBox("epox3", QuartzWidth / 2.0, QuartzThickness / 2., GlueThickness3 / 2.);//Make glue between the second segment and the mirror segment

  TGeoVolume *qseg1 = new TGeoVolume("SQ_quartzSeg1", seg1, barMedium); //Apply medium to quarz segment
  TGeoVolume *qseg2 = new TGeoVolume("SQ_quartzSeg2", seg2, barMedium); //Apply medium to quarz segment


  TGeoVolume *glue1 = new TGeoVolume("SQ_epox1", epox1, epoxMedium); //Apply medium to quarz segment
  TGeoVolume *glue2 = new TGeoVolume("SQ_epox2", epox2, epoxMedium); //Apply medium to quarz segment
  TGeoVolume *glue3 = new TGeoVolume("SQ_epox3", epox3, epoxMedium); //Apply medium to quarz segment

  qseg1->SetTransparency(80); //Colour setting for visual representation no effect on simulation
  qseg1->SetLineColor(38); //Colour setting for visual representation no effect on simulation
  qseg2->SetTransparency(80); //Colour setting for visual representation no effect on simulation
  qseg2->SetLineColor(38); //Colour setting for visual representation no effect on simulation

  glue1->SetTransparency(50); //Colour setting for visual representation no effect on simulation
  glue1->SetLineColor(4); //Colour setting for visual representation no effect on simulation
  glue2->SetTransparency(50); //Colour setting for visual representation no effect on simulation
  glue2->SetLineColor(4); //Colour setting for visual representation no effect on simulation
  glue3->SetTransparency(50); //Colour setting for visual representation no effect on simulation
  glue3->SetLineColor(4); //Colour setting for visual representation no effect on simulation


  //-----------------------------------------------------------
  //
  //              Build the spherical mirror
  //
  //-----------------------------------------------------------

  TGeoBBox* seg3 = new TGeoBBox("seg3", QuartzWidth / 2.0, QuartzThickness / 2., QuartzLenMir / 2.);//Make the second segment of the qurtz bar

  double phimax = TMath::ATan(QuartzThickness / MirrorR) / TMath::Pi() * 180 / 1.9;
  double thetamax = TMath::ATan(QuartzWidth / MirrorR) / TMath::Pi() * 180 / 1.9;

  new TGeoSphere("cutSphere", MirrorR, MirrorR + QuartzLenMir / 2., 90 - thetamax, 90 + thetamax, 90 - 0.1*phimax, 90 + 2*phimax);

  TGeoTranslation *tseg3 = new TGeoTranslation("tseg3", 0.0, 0.0, QuartzLenSeg1 + QuartzLenSeg2 + (QuartzLenMir) / 2. + GlueThickness2 + GlueThickness3 + QuartzBPos);

  TGeoRotation *rmir = new TGeoRotation("rmir", 90.0, 90.0, 0.0);
  TGeoCombiTrans *ctmir = new TGeoCombiTrans("ctmir", 0.0, QuartzThickness / 2., QuartzLenSeg1 + QuartzLenSeg2 + QuartzLenMir + GlueThickness2 + GlueThickness3 + QuartzBPos - MirrorR, rmir);


  tseg3->RegisterYourself();
  ctmir->RegisterYourself();

  TGeoCompositeShape *CSqmir = new TGeoCompositeShape("CSqmir", "(seg3:tseg3)*(cutSphere:ctmir)");//Difference of outer in innner shells


  TGeoVolume *qseg3 = new TGeoVolume("SQ_quartzSeg3", seg3, barMedium); //Apply medium to quarz segment
  TGeoVolume *qmir = new TGeoVolume("qmir", CSqmir, barMedium); //Apply medium to quarz segment


  GearDir MirrorCont(content);
  MirrorCont.append("/Bars/Mirror");
  OpticalUserInfo* surfaceInfo = GeoReader::readOpticalSurface(MirrorCont);
  qmir->SetField(surfaceInfo);


// qsphere->SetTransparency(80); //Colour setting for visual representation no effect on simulation
  qmir->SetLineColor(14); //Colour setting for visual representation no effect on simulation

  qseg3->SetTransparency(80); //Colour setting for visual representation no effect on simulation
  qseg3->SetLineColor(38); //Colour setting for visual representation no effect on simulation


  //----------------------------------------
  //              Build extension volume
  //
  //    ---
  //    | |  extension volume
  //    |/
  //
  //----------------------------------------

  /*
      Double_t vertices[16] = {0., QuartzThickness / 2.0,
             0., (-1.0)*QuartzThickness / 2.0,
                               (-1.0)*QuartzZExt, (-1.0)*QuartzThickness / 2.0 - QuartzYExtUp,
                               (-1.0)*QuartzZExt, QuartzThickness / 2.0 + QuartzYExtDown,
                               0., QuartzThickness / 2.0,
             0., (-1.0)*QuartzThickness / 2.0,
                               (-1.0)*QuartzZExt, (-1.0)*QuartzThickness / 2.0 - QuartzYExtUp,
                               (-1.0)*QuartzZExt, QuartzThickness / 2.0 + QuartzYExtDown
                              };

      TGeoArb8* extension = new TGeoArb8("extension", QuartzWidth / 2.0, vertices); //the object to create the exension volume

      TGeoBBox* extension2 = new TGeoBBox("extension2",  QuartzZExt / 2.,(QuartzThickness + QuartzYExtDown) / 2.0, QuartzWidth / 2.);//Make the quartz bar without extension

      TGeoVolume *extensionBox = new TGeoVolume("extensionBox", extension2, barMedium); //Apply medium to quarz box

      extensionBox->SetTransparency(80); //Colour setting for visual representation no effect on simulation
      extensionBox->SetLineColor(38); //Colour setting for visual representation no effect on simulation
  */
  //----------------------------------------
  //           Build support structure
  //----------------------------------------

//This has to be decided how

  //----------------------------------------
  //           Build PMT
  //----------------------------------------

  TGeoMedium* wallMed = gGeoManager->GetMedium(wallMaterial.c_str()); //Define all the necesary materials
  TGeoMedium* winMed = gGeoManager->GetMedium(winMaterial.c_str());
  TGeoMedium* sensMed = gGeoManager->GetMedium(sensMaterial.c_str());
  TGeoMedium* botMed = gGeoManager->GetMedium(botMaterial.c_str());


  TGeoVolumeAssembly* PMT = new TGeoVolumeAssembly("PMT");//The volume assembly that will house the PMT

  new TGeoBBox("TOuterShell", PMTSizeX / 2., PMTSizeY / 2., PMTSizeZ / 2.);//Outer cube
  new TGeoBBox("TInnerShell", ActiveSizeX / 2., ActiveSizeY / 2., PMTSizeZ / 1.9);//inner Cube
  TGeoCompositeShape *Tshell = new TGeoCompositeShape("Tshell", "TOuterShell-TInnerShell");//Difference of outer in innner shells
  TGeoVolume* PMTshell = new TGeoVolume("PMTshell", Tshell, wallMed); //Assign medium to shell
  PMTshell->SetLineColor(2); //Asign red colour to shell just for display in root

  PMT->AddNode(PMTshell, 1); //Add shell to asembley

  TGeoBBox* winShape = new TGeoBBox("winShape", ActiveSizeX / 2., ActiveSizeY / 2., WindowThickness / 2.);//Construct window
  TGeoVolume* detWin = new TGeoVolume("detWin", winShape, winMed);//Asign medium to window
  detWin->SetLineColor(38); //Window color just for diplay in root
  PMT->AddNode(detWin, 2, new TGeoTranslation(0.0, 0.0, (PMTSizeZ - WindowThickness) / 2.));// Position window

  TGeoBBox* sensShape = new TGeoBBox("sensShape", ActiveSizeX / 2., ActiveSizeY / 2., ActiveSizeZ / 2.);
  TGeoVolume* detSens = new TGeoVolume("SD_detSens", sensShape, sensMed);
  detSens->SetLineColor(8);//29);
  PMT->AddNode(detSens, 3, new TGeoTranslation(0.0, 0.0, (PMTSizeZ - ActiveSizeZ) / 2. - WindowThickness));

  TGeoBBox* bottomShape = new TGeoBBox("sensShape", ActiveSizeX / 2., ActiveSizeY / 2., BottomThickness / 2.);
  TGeoVolume* detBot = new TGeoVolume("detBottom", bottomShape, botMed);
  detBot->SetLineColor(8);
  PMT->AddNode(detBot, 4, new TGeoTranslation(0.0, 0.0, (-PMTSizeZ + BottomThickness) / 2.));

  //----------------------------------------
  //           Combine PMT and quartz bar
  //----------------------------------------

  TGeoVolumeAssembly* barsegment = new TGeoVolumeAssembly("TOPbarsegment"); //Difine assmbly in which the bar will be housed

  TGeoTranslation tseg1("tseg1", 0.0, 0.0, (QuartzLenSeg1) / 2. + QuartzBPos);
  TGeoTranslation tglue1("tglue1", 0.0, 0.0, (-GlueThickness1) / 2. + QuartzBPos);

  TGeoTranslation tseg2("tseg2", 0.0, 0.0, QuartzLenSeg1 + (QuartzLenSeg2) / 2. + GlueThickness2 + QuartzBPos);
  TGeoTranslation tglue2("tglue2", 0.0, 0.0, QuartzLenSeg1 + (GlueThickness2) / 2. + QuartzBPos);

  TGeoTranslation Tseg3("Tseg3", 0.0, 0.0, QuartzLenSeg1 + QuartzLenSeg2 + (QuartzLenMir) / 2. + GlueThickness2 + GlueThickness3 + QuartzBPos);
  TGeoTranslation tglue3("tglue3", 0.0, 0.0, QuartzLenSeg1 + QuartzLenSeg2 + GlueThickness2 + (GlueThickness3) / 2. + QuartzBPos);

  TGeoTranslation tseg3m("tseg3m", 0.0, 0.0, -(QuartzLenSeg1 + QuartzLenSeg2 + (QuartzLenMir) / 2. + GlueThickness2 + GlueThickness3 + QuartzBPos));

  barsegment->AddNode(qseg1, 1, new TGeoHMatrix(tseg1));
  barsegment->AddNode(qseg2, 2, new TGeoHMatrix(tseg2));
  barsegment->AddNode(qseg3, 3, new TGeoHMatrix(Tseg3));

  barsegment->AddNode(glue1, 4, new TGeoHMatrix(tglue1));
  barsegment->AddNode(glue2, 5, new TGeoHMatrix(tglue2));
  barsegment->AddNode(glue3, 6, new TGeoHMatrix(tglue3));


  qseg3->AddNode(qmir, 100, new TGeoHMatrix(tseg3m));


  TGeoBBox* sv = new TGeoBBox("sv", QuartzWidth / 2.0, QuartzThickness / 2., ActiveSizeZ / 2.);
  TGeoVolume* detsv = new TGeoVolume("SD_detsv", sv, sensMed);
  detsv->SetLineColor(1);
  detsv->SetTransparency(60);
  TGeoTranslation tdetsv("tdetsv", 0.0, 0.0, -(ActiveSizeZ) / 2. - GlueThickness1 + QuartzBPos);
  barsegment->AddNode(detsv, 7, new TGeoHMatrix(tdetsv));

  //----------------------------------------
  //    Position barsegments around CDC
  //----------------------------------------

  TGeoTranslation t("tr", 0.0, DetectorInnerRadius + QuartzThickness / 2.0 , 0.0);

  for (int i = 0; i < NumberOfBars; i++) {
    TGeoRotation r("gRot", 0.0, 0.0, 360*i / ((double)NumberOfBars));

    volGrpTOP->AddNode(barsegment, i + 1, new TGeoHMatrix(r*t));

  }

// contTube->AddNode(qsphere, 100,new TGeoHMatrix(tglue1));
}

