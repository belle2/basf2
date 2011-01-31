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

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

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
#include <TGeoXtru.h>
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
//  addSensitiveDetector("SD_", new TOPSensitiveDetector("TOPSensitiveDetector"));
//  addSensitiveDetector("SQ_", new TOPSensitiveQuartz("TOPSensitiveQuartz"));
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

  double DetectorRotation = topgp->GetDetectorRotation() / Unit::deg; //Variables defined in TOPGeometryPar.h
  double DetectorZShift = topgp->GetDetectorZShift();
  int NumberOfBars = topgp->GetNumberOfBars();
  double DetectorInnerRadius = topgp->GetDetectorInnerRadius();
  double QuartzWidth = topgp->GetQuartzWidth();
  double QuartzThickness = topgp->GetQuartzThickness();
  double QuartzFPos = topgp->GetQuartzFPos();
  double QuartzBPos = topgp->GetQuartzBPos();
  double QuartzZExt = topgp->GetQuartzZExt();
  double QuartzYExtUp = topgp->GetQuartzYExtUp();
  double QuartzYExtDown = topgp->GetQuartzYExtDown();
  double GapPMTX = topgp->GetGapPMTX();
  double GapPMTY = topgp->GetGapPMTY();
  double GapPMTX0 = topgp->GetGapPMTX0();
  double GapPMTY0 = topgp->GetGapPMTY0();
  int PMTNX = topgp->GetPMTNX();
  int PMTNY = topgp->GetPMTNY();
  double PMTSizeX = topgp->GetPMTSizeX();
  double PMTSizeY = topgp->GetPMTSizeY();
  double PMTSizeZ = topgp->GetPMTSizeZ();
  double ActiveSizeX = topgp->GetActiveSizeX();
  double ActiveSizeY = topgp->GetActiveSizeY();
  double ActiveSizeZ = topgp->GetActiveSizeZ();
  double WindowThickness = topgp->GetWindowThickness();
  double BottomThickness = topgp->GetBottomThickness();
  double QBBThickness = topgp->GetQBBThickness();
  double QBBSideThickness = topgp->GetQBBSideThickness();
  double QBBFronThickness = topgp->GetQBBFronThickness();
  double AirGapUp = topgp->GetAirGapUp();
  double AirGapDown = topgp->GetAirGapDown();
  double AirGapSide = topgp->GetAirGapSide();
  double QBBForwardPos = topgp->GetQBBForwardPos();
  double QBBBackwardPos = topgp->GetQBBBackwardPos();

  // Here we directly read the material of the detector since it's the only place we need this

  string barMaterial =  content.getParamString("Bars/BarMaterial");
  string wallMaterial =  content.getParamString("PMTs/Module/wallMaterial");
  string sensMaterial = content.getParamString("PMTs/Module/sensMaterial");
  string winMaterial = content.getParamString("PMTs/Module/winMaterial");
  string botMaterial =  content.getParamString("PMTs/Module/botMaterial");
  string pannelMaterial =  content.getParamString("Support/PannelMaterial");

  //----------------------------------------
  //        Add subdetector group
  //----------------------------------------

  TGeoRotation* geoRot = new TGeoRotation("TOPRot", 90.0, DetectorRotation, 0.0);
  TGeoVolumeAssembly* volGrpTOP = addSubdetectorGroup("TOP", new TGeoCombiTrans(0.0, 0.0, DetectorZShift, geoRot));


  //----------------------------------------
  //           Build quartz bar
  //----------------------------------------

  TGeoMedium* barMedium = gGeoManager->GetMedium(barMaterial.c_str()); // Create the medium from database
  TGeoVolumeAssembly* barsegment = new TGeoVolumeAssembly("TOPbarsegment"); //Difine assmbly in which the bar will be housed

  TGeoXtru *box = new TGeoXtru(2); //TGeoXtru the object with which the quartz bar is made

  // Difine the x and y vertices of the quartz box
  Double_t xv[6] = {QuartzBPos - QuartzZExt, QuartzBPos, QuartzFPos, QuartzFPos, QuartzBPos, QuartzBPos - QuartzZExt};
  Double_t yv[6] = {QuartzThickness / 2.0 + QuartzYExtDown, QuartzThickness / 2.0, QuartzThickness / 2.0, -QuartzThickness / 2.0, -QuartzThickness / 2.0, -QuartzThickness / 2.0 - QuartzYExtUp};



  //Use the vertices to create shape
  box->DefinePolygon(6, xv, yv);
  box->DefineSection(0, -QuartzWidth / 2.0, 0.0, 0.0, 1.0); //Define the width of the quarz box
  box->DefineSection(1, QuartzWidth / 2.0, 0.0, 0.0, 1.0);

  TGeoVolume *quartzBox = new TGeoVolume("quartzBox", box, barMedium); //Apply medium to quarz box

  quartzBox->SetTransparency(80.0); //Corlour seting for visual representation no effect on simulation
  quartzBox->SetLineColor(38); //Corlour seting for visual representation no effect on simulation

  //----------------------------------------
  //           Build support structure
  //----------------------------------------

  TGeoMedium* supportMedium = gGeoManager->GetMedium(pannelMaterial.c_str()); // Create the medium from database

  TGeoXtru *QBB1 = new TGeoXtru(2); //TGeoXtru the object with which the QBB support structure is made
  TGeoXtru *QBB2 = new TGeoXtru(2); //TGeoXtru the object with which the QBB support structure is made
  QBB1->SetName("QBB1");
  QBB2->SetName("QBB2");
  // Difine the x and y vertices of the support structure outer side
  Double_t xv2[8] = {QBBBackwardPos,
                     QuartzBPos - QuartzZExt,
                     QuartzBPos,
                     QBBForwardPos,
                     QBBForwardPos,
                     QuartzBPos,
                     QuartzBPos - QuartzZExt,
                     QBBBackwardPos
                    };

  Double_t yv2[8] = {QuartzThickness / 2.0 + QuartzYExtDown + AirGapDown + QBBThickness,
                     QuartzThickness / 2.0 + QuartzYExtDown + AirGapDown + QBBThickness,
                     QuartzThickness / 2.0 + AirGapDown + QBBThickness,
                     QuartzThickness / 2.0 + AirGapDown + QBBThickness,
                     -QuartzThickness / 2.0 - AirGapUp - QBBThickness,
                     -QuartzThickness / 2.0 - AirGapUp - QBBThickness,
                     -QuartzThickness / 2.0 - QuartzYExtUp - AirGapUp - QBBThickness,
                     -QuartzThickness / 2.0 - QuartzYExtUp - AirGapUp - QBBThickness
                    };

  // Difine the x and y vertices of the support structure inner side
  Double_t xv3[8] = {QBBBackwardPos + QBBFronThickness,
                     QuartzBPos - QuartzZExt,
                     QuartzBPos,
                     QBBForwardPos - QBBFronThickness,
                     QBBForwardPos - QBBFronThickness,
                     QuartzBPos,
                     QuartzBPos - QuartzZExt,
                     QBBBackwardPos + QBBFronThickness
                    };

  Double_t yv3[8] = {QuartzThickness / 2.0 + QuartzYExtDown + AirGapDown,
                     QuartzThickness / 2.0 + QuartzYExtDown + AirGapDown,
                     QuartzThickness / 2.0 + AirGapDown,
                     QuartzThickness / 2.0 + AirGapDown,
                     -QuartzThickness / 2.0 - AirGapUp,
                     -QuartzThickness / 2.0 - AirGapUp,
                     -QuartzThickness / 2.0 - QuartzYExtUp - AirGapUp,
                     -QuartzThickness / 2.0 - QuartzYExtUp - AirGapUp
                    };

  //Use the vertices to create shape
  QBB1->DefinePolygon(8, xv2, yv2);
  QBB1->DefineSection(0, -QuartzWidth / 2.0 - QBBSideThickness - AirGapSide, 0.0, 0.0, 1.0); //Define the width of the quarz QBB support
  QBB1->DefineSection(1, QuartzWidth / 2.0 + QBBSideThickness + AirGapSide, 0.0, 0.0, 1.0);

  QBB2->DefinePolygon(8, xv3, yv3);
  QBB2->DefineSection(0, -QuartzWidth / 2.0 - AirGapSide, 0.0, 0.0, 1.0); //Define the width of the quarz QBB support inner side
  QBB2->DefineSection(1, QuartzWidth / 2.0 + AirGapSide, 0.0, 0.0, 1.0);

  TGeoCompositeShape *QBBShell = new TGeoCompositeShape("QBBShell", "QBB1-QBB2");//Difference of outer in innner shells

  TGeoVolume *support = new TGeoVolume("QBBSupport", QBBShell, supportMedium); //Apply medium QBB shell

  support->SetTransparency(20.0); //Corlour seting for visual representation no effect on simulation
  support->SetLineColor(17); //Corlour seting for visual representation no effect on simulation

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
  detSens->SetLineColor(29);
  PMT->AddNode(detSens, 3, new TGeoTranslation(0.0, 0.0, (PMTSizeZ - ActiveSizeZ) / 2. - WindowThickness));

  TGeoBBox* bottomShape = new TGeoBBox("sensShape", ActiveSizeX / 2., ActiveSizeY / 2., BottomThickness / 2.);
  TGeoVolume* detBot = new TGeoVolume("detBottom", bottomShape, botMed);
  detBot->SetLineColor(8);
  PMT->AddNode(detBot, 4, new TGeoTranslation(0.0, 0.0, (-PMTSizeZ + BottomThickness) / 2.));

  //----------------------------------------
  //           Combine PMT and quartz bar
  //----------------------------------------

  TGeoTranslation tPMTz("Transy", 0.0, 0.0, QuartzBPos - QuartzZExt - PMTSizeZ / 2.);

  barsegment->AddNode(quartzBox, 1, new TGeoRotation("barrot", 90.0, 90.0, 90.0));
  barsegment->AddNode(support, 1, new TGeoRotation("barrot", 90.0, 90.0, 90.0));

  for (int i = 0; i < PMTNX; i++) {
    for (int j = 0; j < PMTNY; j++) {

      TGeoTranslation tPMTx("tPMTx", -1.0*QuartzWidth / 2. + GapPMTX0 + PMTSizeX / 2. + (PMTSizeX + GapPMTX)*i, 0.0, 0.0);
      TGeoTranslation tPMTy("tPMTy", 0.0, -QuartzThickness / 2. - QuartzYExtDown + GapPMTY0 + PMTSizeY / 2. + (PMTSizeY + GapPMTY)*j, 0.0);

      barsegment->AddNode(PMT, (i + 1)*(j + 1), new TGeoHMatrix(tPMTx*tPMTy*tPMTz));

    }
  }

  //----------------------------------------
  //    Position barsegments around CDC
  //----------------------------------------

  TGeoTranslation t("tr", 0.0, DetectorInnerRadius + QuartzThickness / 2.0 , 0.0);

//  int i=0;
  for (int i = 0; i < NumberOfBars; i++) {
    TGeoRotation r("gRot", 0.0, 0.0, 360*i / ((double)NumberOfBars));

    volGrpTOP->AddNode(barsegment, i + 1, new TGeoHMatrix(r*t));

  }/**/
}

