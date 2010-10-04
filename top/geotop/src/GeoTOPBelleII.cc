/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geotop/GeoTOPBelleII.h>

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoPgon.h>
#include <TVector2.h>
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
//  activateAutoSensitiveVolumes("SD_");
}


GeoTOPBelleII::~GeoTOPBelleII()
{

}


void GeoTOPBelleII::create(GearDir& content)
{

  GearDir bars(content);
  bars.append("Bars/");

  //----------------------------------------
  //         Get global parameters
  //----------------------------------------

  double globalRotAngle = content.getParamAngle("Rotation") / deg; //global rotation of the detector
  double globalOffsetZ  = content.getParamLength("OffsetZ"); //global offset in the Z direction

  int N = bars.getParamLength("Nbar"); //Number of bars
  double R = bars.getParamLength("Radius"); //Distance from the IP
  double ZF = bars.getParamLength("QZForward"); //qurtz length=ZF-ZB
  double ZB = bars.getParamLength("QZBackward"); //absolute position of the forward and backward edge of the quartz bar
  double A = bars.getParamLength("QWidth"); //quartz width
  double B = bars.getParamLength("QThickness"); //quartz thickness
  double Ez = bars.getParamLength("QZext"); //quartz extension in z direction
  double Eyup = bars.getParamLength("QYextup"); //quartz extension in y upwards directions
  double Eydown = bars.getParamLength("QYextdown"); //quartz extension in y downwards direction
  string BarMaterial =  bars.getParamString("BarMaterial");


  //----------------------------------------
  //        Add subdetector group
  //----------------------------------------

  TGeoRotation* geoRot = new TGeoRotation("TOPRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volTOP = addSubdetectorGroup("TOP", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));


  //----------------------------------------
  //           Build quartz bar
  //----------------------------------------

  TGeoMedium* boxMed = gGeoManager->GetMedium(BarMaterial.c_str());
  TGeoVolumeAssembly* barsegment = new TGeoVolumeAssembly("TOPbarsegment");

  TGeoXtru *box = new TGeoXtru(2);
  //TGeoBBox* box2 = new TGeoBBox("test",A/2.,B/2.,-ZB/2.);

  Double_t xv[6] = {ZB - Ez, ZB, ZF, ZF, ZB, ZB - Ez};
  Double_t yv[6] = {B / 2.0 + Eydown, B / 2.0, B / 2.0, -B / 2.0, -B / 2.0, -B / 2.0 - Eyup};

  box->DefinePolygon(6, xv, yv);
  box->DefineSection(0, -A / 2.0, 0.0, 0.0, 1.0);
  box->DefineSection(1, A / 2.0, 0.0, 0.0, 1.0);

  TGeoVolume *quartzBox = new TGeoVolume("quartzBox", box, boxMed);

  quartzBox->SetTransparency(80.0);
  quartzBox->SetLineColor(38);


  //----------------------------------------
  //           Build PMT
  //----------------------------------------

  GearDir PMTs(content);
  PMTs.append("PMTs/");
  GearDir Module(PMTs);
  Module.append("Module/");

  double Xgap = PMTs.getParamLength("Xgap");
  double Ygap = PMTs.getParamLength("Ygap");
  double x0  = PMTs.getParamLength("x0");
  double y0  = PMTs.getParamLength("y0");
  int nPMTx = PMTs.getParamLength("nPMTx");
  int nPMTy = PMTs.getParamLength("nPMTy");


  string wallMaterial =  Module.getParamString("wallMaterial");
  double ModuleXSize = Module.getParamLength("ModuleXSize");
  double ModuleYSize  = Module.getParamLength("ModuleYSize");
  double ModuleZSize = Module.getParamLength("ModuleZSize");

  double SensXSize = Module.getParamLength("SensXSize");
  double SensYSize = Module.getParamLength("SensYSize");
  double SensThickness = Module.getParamLength("SensThickness");
  string sensMaterial = Module.getParamString("sensMaterial");

  string winMaterial = Module.getParamString("winMaterial");
  double WindowThickness = Module.getParamLength("WindowThickness");

  int PadXNum = Module.getParamLength("PadXNum");
  int PadYNum = Module.getParamLength("PadYNum");

  double BottomThickness = Module.getParamLength("BottomThickness");
  string botMaterial =  Module.getParamString("botMaterial");


  TGeoMedium* wallMed = gGeoManager->GetMedium(wallMaterial.c_str());
  TGeoMedium* winMed = gGeoManager->GetMedium(winMaterial.c_str());
  TGeoMedium* sensMed = gGeoManager->GetMedium(sensMaterial.c_str());
  TGeoMedium* botMed = gGeoManager->GetMedium(botMaterial.c_str());


  TGeoVolumeAssembly* PMT = new TGeoVolumeAssembly("PMT");

  TGeoBBox* TOuterShell = new TGeoBBox("TOuterShell", ModuleXSize / 2., ModuleYSize / 2., ModuleZSize / 2.);
  TGeoBBox* TInnerShell = new TGeoBBox("TInnerShell", SensXSize / 2., SensYSize / 2., ModuleZSize / 1.9);
  TGeoCompositeShape *Tshell = new TGeoCompositeShape("Tshell", "TOuterShell-TInnerShell");
  TGeoVolume* PMTshell = new TGeoVolume("PMTshell", Tshell, wallMed);
  PMTshell->SetLineColor(2);

  PMT->AddNode(PMTshell, 1);

  TGeoBBox* winShape = new TGeoBBox("winShape", SensXSize / 2., SensYSize / 2., WindowThickness / 2.);
  TGeoVolume* detWin = new TGeoVolume("detWin", winShape, winMed);
  detWin->SetLineColor(38);
  PMT->AddNode(detWin, 2, new TGeoTranslation(0.0, 0.0, (ModuleZSize - WindowThickness) / 2.));

  TGeoBBox* sensShape = new TGeoBBox("sensShape", SensXSize / 2., SensYSize / 2., SensThickness / 2.);
  TGeoVolume* detSens = new TGeoVolume("SD_detSens", sensShape, sensMed);
  detSens->SetLineColor(29);
  PMT->AddNode(detSens, 3, new TGeoTranslation(0.0, 0.0, (ModuleZSize - SensThickness) / 2. - WindowThickness));

  TGeoBBox* bottomShape = new TGeoBBox("sensShape", SensXSize / 2., SensYSize / 2., BottomThickness / 2.);
  TGeoVolume* detBot = new TGeoVolume("detBottom", bottomShape, botMed);
  detBot->SetLineColor(8);
  PMT->AddNode(detBot, 4, new TGeoTranslation(0.0, 0.0, (-ModuleZSize + BottomThickness) / 2.));

  //----------------------------------------
  //           Combine PMT and quartz bar
  //----------------------------------------

  TGeoTranslation tPMTz("Transy", 0.0, 0.0, ZB - Ez - ModuleZSize / 2.);

  barsegment->AddNode(quartzBox, 1, new TGeoRotation("barrot", 90.0, 90.0, 90.0));

  for (int i = 0; i < nPMTx; i++) {
    for (int j = 0; j < nPMTy; j++) {

      TGeoTranslation tPMTx("tPMTx", -1.0*A / 2 + x0 + ModuleXSize / 2. + (ModuleXSize + Xgap)*i, 0.0, 0.0);
      TGeoTranslation tPMTy("tPMTy", 0.0, -B / 2. - Eydown + y0 + ModuleYSize / 2. + (ModuleYSize + Ygap)*j, 0.0);

      barsegment->AddNode(PMT, (i + 1)*(j + 1), new TGeoHMatrix(tPMTx*tPMTy*tPMTz));

    }
  }

  //----------------------------------------
  //    Position barsegments around CDC
  //----------------------------------------

  TGeoTranslation t("tr", 0.0, R, 0.0);

//  int i=0;
  for (int i = 0; i < N; i++) {
    TGeoRotation r("gRot", 0.0, 0.0, 360*i / ((double)N));

    volTOP->AddNode(barsegment, i + 1, new TGeoHMatrix(r*t));

  }
}

