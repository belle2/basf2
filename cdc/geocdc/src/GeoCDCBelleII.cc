/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/geocdc/GeoCDCBelleII.h>
#include <cdc/simcdc/CDCSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoTrd1.h>

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

GeoCDCBelleII regGeoCDCBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoCDCBelleII::GeoCDCBelleII() : CreatorBase("CDCBelleII")
{
  setDescription("Creates the TGeo objects for the CDC geometry of the Belle II detector.");
  addSensitiveDetector("SD_", new CDCSensitiveDetector("CDCSensitiveDetector", (2*24)*eV, 10*MeV)); //The CDC subdetector uses the "SD_" prefix to flag its sensitive volumes
}


GeoCDCBelleII::~GeoCDCBelleII()
{

}


void GeoCDCBelleII::create(GearDir& content)
{
  //------------------------
  // Get global parameters
  //------------------------
  double globalRotAngle = (180.0 / M_PI) * content.getParamAngle("Rotation");
  double globalOffsetZ  = content.getParamLength("OffsetZ");
  string Helium  = content.getParamString("Helium");
  string Ethane  = content.getParamString("Ethane");
  string Aluminum  = content.getParamString("Aluminum");
  string Tungsten  = content.getParamString("Tungsten");
  string CFRP  = content.getParamString("CFRP");
  string NEMA_G10_Plate  = content.getParamString("NEMA_G10_Plate");

  TGeoRotation* geoRot = new TGeoRotation("CDCRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpCDC = addSubdetectorGroup("CDC", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));

  //-------------------------------
  // Get parameters of outer wall
  //-------------------------------
  int nOuterWall = content.getNumberNodes("OuterWalls/OuterWall");

  string* outerWallName   = new string[nOuterWall];
  double* outerWallInnerR = new double[nOuterWall];
  double* outerWallOuterR = new double[nOuterWall];
  double* outerWallBZ     = new double[nOuterWall];
  double* outerWallFZ     = new double[nOuterWall];

  // Used in density calculation.
  double rmin_outerWall;

  for (int iOuterWall = 0; iOuterWall < nOuterWall; ++iOuterWall) {
    GearDir outerWallContent(content);
    outerWallContent.append((format("OuterWalls/OuterWall[%1%]/") % (iOuterWall + 1)).str());

    string sOuterWallID = outerWallContent.getParamString("@id");
    int outerWallID = atoi(sOuterWallID.c_str());
    outerWallName[outerWallID] = "OuterWall_" + sOuterWallID + "_" + outerWallContent.getParamString("Name");
    outerWallInnerR[outerWallID] = outerWallContent.getParamLength("InnerR");
    outerWallOuterR[outerWallID] = outerWallContent.getParamLength("OuterR");
    outerWallBZ[outerWallID]  = outerWallContent.getParamLength("BackwardZ");
    outerWallFZ[outerWallID]   = outerWallContent.getParamLength("ForwardZ");

    if (outerWallContent.getParamString("Name") == "Shield") rmin_outerWall = outerWallContent.getParamLength("InnerR");
  }

  //-------------------------------
  // Get parameters of inner wall
  //-------------------------------
  int nInnerWall = content.getNumberNodes("InnerWalls/InnerWall");

  string* innerWallName   = new string[nInnerWall];
  double* innerWallInnerR = new double[nInnerWall];
  double* innerWallOuterR = new double[nInnerWall];
  double* innerWallBZ     = new double[nInnerWall];
  double* innerWallFZ     = new double[nInnerWall];

  // Used in density calculation.
  double rmax_innerWall;

  for (int iInnerWall = 0; iInnerWall < nInnerWall; ++iInnerWall) {
    GearDir innerWallContent(content);
    innerWallContent.append((format("InnerWalls/InnerWall[%1%]/") % (iInnerWall + 1)).str());

    string sInnerWallID = innerWallContent.getParamString("@id");
    int innerWallID = atoi(sInnerWallID.c_str());
    innerWallName[innerWallID] = "InnerWall_" + sInnerWallID + "_" + innerWallContent.getParamString("Name");
    innerWallInnerR[innerWallID] = innerWallContent.getParamLength("InnerR");
    innerWallOuterR[innerWallID] = innerWallContent.getParamLength("OuterR");
    innerWallBZ[innerWallID]  = innerWallContent.getParamLength("BackwardZ");
    innerWallFZ[innerWallID]   = innerWallContent.getParamLength("ForwardZ");

    if (innerWallContent.getParamString("Name") == "Shield") rmax_innerWall = innerWallContent.getParamLength("OuterR");
  }

  //-----------------------------------------------------------------------
  // Get sense wire and field wire information, radius and total numbers.
  //-----------------------------------------------------------------------
  GearDir senseWire(content);
  senseWire.append("SenseWire/");
  double diameter_senseWire = senseWire.getParamLength("Diameter");
  int num_senseWire = atoi((senseWire.getParamString("Number")).c_str());

  GearDir fieldWire(content);
  fieldWire.append("FieldWire/");
  double diameter_fieldWire = fieldWire.getParamLength("Diameter");
  int num_fieldWire = atoi((fieldWire.getParamString("Number")).c_str());

  //----------------
  // Get Material
  //----------------
  TGeoMedium* medHelium = gGeoManager->GetMedium(Helium.c_str());
  TGeoMedium* medEthane = gGeoManager->GetMedium(Ethane.c_str());
  TGeoMedium* medAluminum = gGeoManager->GetMedium(Aluminum.c_str());
  TGeoMedium* medTungsten = gGeoManager->GetMedium(Tungsten.c_str());
  TGeoMedium* medCFRP = gGeoManager->GetMedium(CFRP.c_str());
  TGeoMedium* medNEMA_G10_Plate = gGeoManager->GetMedium(NEMA_G10_Plate.c_str());

  TGeoMaterial* matHelium = medHelium->GetMaterial();
  TGeoMaterial* matEthane = medEthane->GetMaterial();
  TGeoMaterial* matAluminum = medAluminum->GetMaterial();
  TGeoMaterial* matTungsten = medTungsten->GetMaterial();

  // Calculate average density

  // Total cross section
  double totalCS = M_PI * (rmin_outerWall * rmin_outerWall - rmax_innerWall * rmax_innerWall);

  // Sense wire cross section
  double senseCS = M_PI * (diameter_senseWire / 2) * (diameter_senseWire / 2) * num_senseWire;

  // Field wire cross section
  double fieldCS = M_PI * (diameter_fieldWire / 2) * (diameter_fieldWire / 2) * num_fieldWire;

  // Density
  double denHelium = matHelium->GetDensity() / 2.0;
  double denEthane = matEthane->GetDensity() / 2.0;
  double denAluminum = matAluminum->GetDensity() * (fieldCS / totalCS);
  double denTungsten = matTungsten->GetDensity() * (senseCS / totalCS);

  double density = denHelium + denEthane + denAluminum + denTungsten;

  // Create cdc gaswire
  TGeoMixture* cdcMix = new TGeoMixture("CDCGasWire", 4);
  cdcMix->AddElement(matHelium, denHelium / density);
  cdcMix->AddElement(matEthane, denEthane / density);
  cdcMix->AddElement(matTungsten, denTungsten / density);
  cdcMix->AddElement(matAluminum, denAluminum / density);
  cdcMix->SetDensity(density);

  TGeoMedium* cdcMed = new TGeoMedium(cdcMix->GetName(), 301, cdcMix);

  //-----------------------
  // Construct outer wall
  //----------------------
  TGeoVolumeAssembly* volGrpOuterWall = new TGeoVolumeAssembly("OuterWall");
  volGrpCDC->AddNode(volGrpOuterWall, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  for (int iOuterWall = 0; iOuterWall < nOuterWall; ++iOuterWall) {
    double length = (outerWallFZ[iOuterWall] - outerWallBZ[iOuterWall]) / 2.0;
    if (strstr((outerWallName[iOuterWall]).c_str(), "MiddleWall") != NULL) {
      TGeoVolume* outerWallTube = gGeoManager->MakeTube((outerWallName[iOuterWall]).c_str(), medCFRP, outerWallInnerR[iOuterWall], outerWallOuterR[iOuterWall], length);
      outerWallTube->SetLineColor(kGray + 3);
      volGrpOuterWall->AddNode(outerWallTube, iOuterWall, new TGeoTranslation(0.0, 0.0, (length + outerWallBZ[iOuterWall])));
    } else {
      TGeoVolume* outerWallTube = gGeoManager->MakeTube((outerWallName[iOuterWall]).c_str(), medAluminum, outerWallInnerR[iOuterWall], outerWallOuterR[iOuterWall], length);
      outerWallTube->SetLineColor(kGray);
      volGrpOuterWall->AddNode(outerWallTube, iOuterWall, new TGeoTranslation(0.0, 0.0, (length + outerWallBZ[iOuterWall])));
    }
  }

  //-----------------------
  // Construct inner wall
  //-----------------------
  TGeoVolumeAssembly* volGrpInnerWall = new TGeoVolumeAssembly("InnerWall");
  volGrpCDC->AddNode(volGrpInnerWall, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  for (int iInnerWall = 0; iInnerWall < nInnerWall; ++iInnerWall) {
    double length = (innerWallFZ[iInnerWall] - innerWallBZ[iInnerWall]) / 2.0;
    if (strstr((innerWallName[iInnerWall]).c_str(), "MiddleWall") != NULL) {
      TGeoVolume* innerWallTube = gGeoManager->MakeTube((innerWallName[iInnerWall]).c_str(), medCFRP, innerWallInnerR[iInnerWall], innerWallOuterR[iInnerWall], length);
      innerWallTube->SetLineColor(kGray + 3);
      volGrpInnerWall->AddNode(innerWallTube, iInnerWall, new TGeoTranslation(0.0, 0.0, (length + innerWallBZ[iInnerWall])));
    } else {
      TGeoVolume* innerWallTube = gGeoManager->MakeTube((innerWallName[iInnerWall]).c_str(), medAluminum, innerWallInnerR[iInnerWall], innerWallOuterR[iInnerWall], length);
      innerWallTube->SetLineColor(kGray);
      volGrpInnerWall->AddNode(innerWallTube, iInnerWall, new TGeoTranslation(0.0, 0.0, (length + innerWallBZ[iInnerWall])));
    }
  }

  //-----------------------
  // Get number of layers
  //-----------------------
  int nSLayer = content.getNumberNodes("SLayers/SLayer");
  int nFLayer = content.getNumberNodes("FLayers/FLayer");
  int nEndplate = content.getNumberNodes("Endplates/Endplate");

  if (nSLayer != nEndplate) {
    B2ERROR("The number of sensitive layers is not same with the number of endplates, exit.");
    exit(0);
  }

  double* slayerRadius = new double[nSLayer];
  double* slayerZBack  = new double[nSLayer];
  double* slayerZFor   = new double[nSLayer];

  string** epName   = new string*[nSLayer];
  double** epInnerR = new double*[nSLayer];
  double** epOuterR = new double*[nSLayer];
  double** epBZ     = new double*[nSLayer];
  double** epFZ     = new double*[nSLayer];
  int* nEndplateLayer = new int[nSLayer];

  double* flayerRadius = new double[nFLayer];
  double* flayerZBack  = new double[nFLayer];
  double* flayerZFor   = new double[nFLayer];

  //--------------------------------------------------
  // Get parameters for sensitive layer and endplates
  //--------------------------------------------------
  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
    // Get parameters of sensitive layers
    GearDir layerContent(content);
    layerContent.append((format("SLayers/SLayer[%1%]/") % (iSLayer + 1)).str());

    string layerID = layerContent.getParamString("@id");
    int ilayerID = atoi(layerID.c_str());
    slayerRadius[ilayerID] = layerContent.getParamLength("Radius");
    slayerZBack[ilayerID]  = layerContent.getParamLength("BackwardZ");
    slayerZFor[ilayerID]   = layerContent.getParamLength("ForwardZ");

    // Get parameters of endplates
    GearDir epContent(content);
    epContent.append((format("Endplates/Endplate[%1%]/") % (iSLayer + 1)).str());
    string sepID = epContent.getParamString("@id");
    int epID = atoi(sepID.c_str());
    int nEPLayer = epContent.getNumberNodes("EndplateLayer");

    nEndplateLayer[epID]     = nEPLayer;

    epName[epID]   = new string[nEPLayer];
    epInnerR[epID] = new double[nEPLayer];
    epOuterR[epID] = new double[nEPLayer];
    epBZ[epID]     = new double[nEPLayer];
    epFZ[epID]     = new double[nEPLayer];

    for (int iEPLayer = 0; iEPLayer < nEPLayer; ++iEPLayer) {
      GearDir epLayerContent(epContent);
      epLayerContent.append((format("EndplateLayer[%1%]/") % (iEPLayer + 1)).str());
      string sepLayerID = epLayerContent.getParamString("@id");
      int epLayerID = atoi(sepLayerID.c_str());
      epName[epID][epLayerID] = epLayerContent.getParamString("Name");
      epInnerR[epID][epLayerID] = epLayerContent.getParamLength("InnerR");
      epOuterR[epID][epLayerID] = epLayerContent.getParamLength("OuterR");
      epBZ[epID][epLayerID] = epLayerContent.getParamLength("BackwardZ");
      epFZ[epID][epLayerID] = epLayerContent.getParamLength("ForwardZ");
    }
  }

  //--------------------------------
  // Get parameters for field layer
  //--------------------------------
  for (int iFLayer = 0; iFLayer < nFLayer; ++iFLayer) {
    GearDir layerContent(content);
    layerContent.append((format("FLayers/FLayer[%1%]/") % (iFLayer + 1)).str());

    string layerID = layerContent.getParamString("@id");
    int ilayerID = atoi(layerID.c_str());
    flayerRadius[ilayerID] = layerContent.getParamLength("Radius");
    flayerZBack[ilayerID]  = layerContent.getParamLength("BackwardZ");
    flayerZFor[ilayerID]   = layerContent.getParamLength("ForwardZ");
  }

  //----------------------------
  // Get length of feedthrough
  //----------------------------
  GearDir feedthroughContent(content);
  feedthroughContent.append("FeedThrough/");

  double length_feedthrough  = feedthroughContent.getParamLength("Length");

  //------------------------------------------
  // Construct sensitive layers and endplates
  //------------------------------------------
  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
    // Get the number of endplate layers
    int nEPLayer = nEndplateLayer[iSLayer];

    // Construct a layer group, in this group, including sensitive layers and endplates
    //double z_min, z_max;
    //if(iSLayer >= 0 && iSLayer <= 14) {
    //  z_min = epBZ[iSLayer][nEPLayer/2 - 1];
    //  z_max = epFZ[iSLayer][nEPLayer - 1];
    //}
    //else if(iSLayer >= 15 && iSLayer <= 18) {
    //  z_min = epBZ[iSLayer][nEPLayer/2 - 2];
    //  z_max = epFZ[iSLayer][nEPLayer - 1];
    //}
    //else if(iSLayer == 19) {
    //  z_min = epBZ[iSLayer][(nEPLayer-1)/2 - 1];
    //  z_max = epFZ[iSLayer][nEPLayer - 1];
    //}
    //else {
    //  z_min = epBZ[iSLayer][nEPLayer/2 - 1];
    //  z_max = epFZ[iSLayer][nEPLayer - 1];
    //}

    TGeoVolumeAssembly* volGrpLayer = new TGeoVolumeAssembly((format("CDCLayer_%1%") % iSLayer).str().c_str());
    volGrpCDC->AddNode(volGrpLayer, iSLayer, new TGeoTranslation(0.0, 0.0, 0.0));

    // Get parameters for sensitive layer: left, middle and right.
    double rmin_sensitive_left, rmax_sensitive_left, rmin_sensitive_middle, rmax_sensitive_middle, rmin_sensitive_right, rmax_sensitive_right;
    double zback_sensitive_left, zfor_sensitive_left, zback_sensitive_middle, zfor_sensitive_middle, zback_sensitive_right, zfor_sensitive_right;

    if (iSLayer == 0) {
      rmin_sensitive_left = epOuterR[iSLayer][0];
      rmax_sensitive_left = flayerRadius[iSLayer];
      zback_sensitive_left = slayerZBack[iSLayer];
      zfor_sensitive_left = epFZ[iSLayer][0];

      rmin_sensitive_middle = innerWallOuterR[0];
      rmax_sensitive_middle = flayerRadius[iSLayer];
      zback_sensitive_middle = epFZ[iSLayer][0];
      zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2];

      rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2];
      rmax_sensitive_right = flayerRadius[iSLayer];
      zback_sensitive_right = epBZ[iSLayer][nEPLayer/2];
      zfor_sensitive_right = slayerZFor[iSLayer];
    } else if (iSLayer >= 1 && iSLayer <= 14) {
      rmin_sensitive_left = epOuterR[iSLayer][1];
      rmax_sensitive_left = flayerRadius[iSLayer];
      zback_sensitive_left = slayerZBack[iSLayer];
      zfor_sensitive_left = epFZ[iSLayer][1];

      rmin_sensitive_middle = flayerRadius[iSLayer-1];
      rmax_sensitive_middle = flayerRadius[iSLayer];
      zback_sensitive_middle = epFZ[iSLayer][1];
      zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2+1];

      rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2+1];
      rmax_sensitive_right = flayerRadius[iSLayer];
      zback_sensitive_right = epBZ[iSLayer][nEPLayer/2+1];
      zfor_sensitive_right = slayerZFor[iSLayer];
    } else if (iSLayer >= 15 && iSLayer <= 18) {
      if (iSLayer == 15) {
        rmin_sensitive_left = epOuterR[iSLayer][1];
        rmax_sensitive_left = flayerRadius[iSLayer];
        zback_sensitive_left = slayerZBack[iSLayer];
        zfor_sensitive_left = epFZ[iSLayer][1];

        rmin_sensitive_middle = flayerRadius[iSLayer-1];
        rmax_sensitive_middle = flayerRadius[iSLayer];
        zback_sensitive_middle = epFZ[iSLayer][1];
        zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2];

        rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2];
        rmax_sensitive_right = flayerRadius[iSLayer];
        zback_sensitive_right = epBZ[iSLayer][nEPLayer/2];
        zfor_sensitive_right = slayerZFor[iSLayer];
      } else {
        rmin_sensitive_left = epOuterR[iSLayer][0];
        rmax_sensitive_left = flayerRadius[iSLayer];
        zback_sensitive_left = slayerZBack[iSLayer];
        zfor_sensitive_left = epFZ[iSLayer][0];

        rmin_sensitive_middle = flayerRadius[iSLayer-1];
        rmax_sensitive_middle = flayerRadius[iSLayer];
        zback_sensitive_middle = epFZ[iSLayer][0];
        zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2];

        rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2];
        rmax_sensitive_right = flayerRadius[iSLayer];
        zback_sensitive_right = epBZ[iSLayer][nEPLayer/2];
        zfor_sensitive_right = slayerZFor[iSLayer];
      }
    } else if (iSLayer >= 19 && iSLayer < (nSLayer - 1)) {
      rmin_sensitive_left = epOuterR[iSLayer][0];
      rmax_sensitive_left = flayerRadius[iSLayer];
      zback_sensitive_left = slayerZBack[iSLayer];
      zfor_sensitive_left = epFZ[iSLayer][0];

      rmin_sensitive_middle = flayerRadius[iSLayer-1];
      rmax_sensitive_middle = flayerRadius[iSLayer];
      zback_sensitive_middle = epFZ[iSLayer][0];
      zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2];

      rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2];
      rmax_sensitive_right = flayerRadius[iSLayer];
      zback_sensitive_right = epBZ[iSLayer][nEPLayer/2];
      zfor_sensitive_right = slayerZFor[iSLayer];
    } else {
      rmin_sensitive_left = epOuterR[iSLayer][0];
      rmax_sensitive_left = epInnerR[iSLayer][nEPLayer/2-1];
      zback_sensitive_left = slayerZBack[iSLayer];
      zfor_sensitive_left = epFZ[iSLayer][0];

      rmin_sensitive_middle = flayerRadius[iSLayer-1];
      rmax_sensitive_middle = outerWallInnerR[0];
      zback_sensitive_middle = epFZ[iSLayer][0];
      zfor_sensitive_middle = epBZ[iSLayer][nEPLayer/2];

      rmin_sensitive_right = epOuterR[iSLayer][nEPLayer/2];
      rmax_sensitive_right = epInnerR[iSLayer][nEPLayer-1];
      zback_sensitive_right = epBZ[iSLayer][nEPLayer/2];
      zfor_sensitive_right = slayerZFor[iSLayer];
    }

    // Check if build left sensitive tube
    if ((zfor_sensitive_left - zback_sensitive_left) > length_feedthrough) {
      //==========================================================
      //    zback_sensitive_left
      //          |
      //         \|/
      //  _____________________
      //  |       |// 1 // |  |
      //  |       |==ft====|2 |         (ft = feedthrouth)
      //  |_______|____1___|__|
      //  |_______|___________|
      //                      |
      //                     \|/
      //                zfor_sensitive_left
      //==========================================================

      // Build a tube with metarial cdcMed for area 1
      TGeoVolume* leftTube = gGeoManager->MakeTube((format("CDCLayer_%1%_leftTube") % iSLayer).str().c_str(),
                                                   cdcMed, rmin_sensitive_left, rmax_sensitive_left, length_feedthrough / 2.0);
      volGrpLayer->AddNode(leftTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zback_sensitive_left + length_feedthrough / 2.0)));

      // Build left sensitive tube (area 2)
      TGeoVolume* leftSensitiveTube = gGeoManager->MakeTube((format("SD_CDCLayer_%1%_left") % iSLayer).str().c_str(),
                                                            cdcMed, rmin_sensitive_left, rmax_sensitive_left, (zfor_sensitive_left - zback_sensitive_left - length_feedthrough) / 2.0);
      volGrpLayer->AddNode(leftSensitiveTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left + length_feedthrough) / 2.0));
    } else {
      //==========================================================
      //    zback_sensitive_left
      //          |
      //         \|/
      //  _________________________
      //  |       |//// 1 ////| 2 |
      //  |       |======ft========   (ft = feedthrouth)
      //  |_______|____1______| 2 |
      //  |_______|___________|___|
      //                      |
      //                     \|/
      //                zfor_sensitive_left
      //==========================================================

      // Build a tube with metarial cdcMed for area 1
      TGeoVolume* leftTube = gGeoManager->MakeTube((format("CDCLayer_%1%_leftTube") % iSLayer).str().c_str(),
                                                   cdcMed, rmin_sensitive_left, rmax_sensitive_left, (zfor_sensitive_left - zback_sensitive_left) / 2.0);
      volGrpLayer->AddNode(leftTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left) / 2.0));

      // Build a tube with metarial cdcMed for area 2
      TGeoVolume* leftMidTube = gGeoManager->MakeTube((format("CDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(),
                                                      cdcMed, rmin_sensitive_middle, rmax_sensitive_middle,
                                                      (length_feedthrough - zfor_sensitive_left + zback_sensitive_left) / 2.0);
      volGrpLayer->AddNode(leftMidTube, iSLayer, new TGeoTranslation(0.0, 0.0, (length_feedthrough + zfor_sensitive_left + zback_sensitive_left) / 2.0));

      // Reset zback_sensitive_middle
      zback_sensitive_middle = length_feedthrough + zback_sensitive_left;
    }

    // Check if build right sensitive tube
    if ((zfor_sensitive_right - zback_sensitive_right) > length_feedthrough) {
      //==========================================================
      //              zfor_sensitive_right
      //                      |
      //                     \|/
      //  _____________________________
      //  |       |     1     |///////|
      //  |   2   |====ft=====|///////|  (ft = feedthrouth)
      //  |_______|____1______|_______|
      //  |_______|___________|_______|
      //  |
      // \|/
      // zback_sensitive_right
      //==========================================================

      // Build a tube with metarial cdcMed for area 1
      TGeoVolume* rightTube = gGeoManager->MakeTube((format("CDCLayer_%1%_rightTube") % iSLayer).str().c_str(),
                                                    cdcMed, rmin_sensitive_right, rmax_sensitive_right, length_feedthrough / 2.0);
      volGrpLayer->AddNode(rightTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_right - length_feedthrough / 2.0)));

      // Build right sensitive tube (area 2)
      TGeoVolume* rightSensitiveTube = gGeoManager->MakeTube((format("SD_CDCLayer_%1%_right") % iSLayer).str().c_str(),
                                                             cdcMed, rmin_sensitive_right, rmax_sensitive_right, (zfor_sensitive_right - zback_sensitive_right - length_feedthrough) / 2.0);
      volGrpLayer->AddNode(rightSensitiveTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right - length_feedthrough) / 2.0));
    } else {
      //==========================================================
      //              zfor_sensitive_right
      //                      |
      //                     \|/
      //  _____________________________
      //  |       |     1     |///////|
      //  |============ft=====|///////|  (ft = feedthrouth)
      //  |       |____1______|_______|
      //  |_______|___________|_______|
      //          |
      //         \|/
      //         zback_sensitive_right
      //==========================================================

      // Build a tube with metarial cdcMed for area 1
      TGeoVolume* rightTube = gGeoManager->MakeTube((format("CDCLayer_%1%_rightTube") % iSLayer).str().c_str(),
                                                    cdcMed, rmin_sensitive_right, rmax_sensitive_right, (zfor_sensitive_right - zback_sensitive_right) / 2.0);
      volGrpLayer->AddNode(rightTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right) / 2.0));

      // Build a tube with metarial cdcMed for area 2
      TGeoVolume* rightMidTube = gGeoManager->MakeTube((format("CDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(),
                                                       cdcMed, rmin_sensitive_middle, rmax_sensitive_middle,
                                                       (length_feedthrough - zfor_sensitive_right + zback_sensitive_right) / 2.0);
      volGrpLayer->AddNode(rightMidTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zback_sensitive_right - length_feedthrough + zfor_sensitive_right) / 2.0));

      // Reset zback_sensitive_middle
      zfor_sensitive_middle = zfor_sensitive_right - length_feedthrough;
    }

    // Middle sensitive tube
    TGeoVolume* middleSensitiveTube = gGeoManager->MakeTube((format("SD_CDCLayer_%1%_middle") % iSLayer).str().c_str(),
                                                            cdcMed, rmin_sensitive_middle, rmax_sensitive_middle, (zfor_sensitive_middle - zback_sensitive_middle) / 2.0);
    volGrpLayer->AddNode(middleSensitiveTube, iSLayer, new TGeoTranslation(0.0, 0.0, (zfor_sensitive_middle + zback_sensitive_middle) / 2.0));

    // Endplates
    for (int iEPLayer = 0; iEPLayer < nEPLayer; ++iEPLayer) {
      std::ostringstream endplateName;
      endplateName << "CDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
      double length_endplate = (epFZ[iSLayer][iEPLayer] - epBZ[iSLayer][iEPLayer]) / 2.0;
      TGeoVolume* endplateTube = gGeoManager->MakeTube(endplateName.str().c_str(),
                                                       medAluminum, epInnerR[iSLayer][iEPLayer], epOuterR[iSLayer][iEPLayer], length_endplate);
      endplateTube->SetLineColor(kGray);
      volGrpLayer->AddNode(endplateTube, iSLayer, new TGeoTranslation(0.0, 0.0, (epFZ[iSLayer][iEPLayer] + epBZ[iSLayer][iEPLayer]) / 2.0));
    }
  }

  //--------------------------------------------------
  // Get paprameters and construct electronics boards
  //--------------------------------------------------
  int nEB = content.getNumberNodes("ElectronicsBoards/ElectronicsBoard");
  TGeoVolumeAssembly* volGrpEB = new TGeoVolumeAssembly("ElectronicsBoards");
  volGrpCDC->AddNode(volGrpEB, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  for (int iEB = 0; iEB < nEB; ++iEB) {
    // Get parameters
    GearDir ebContent(content);
    ebContent.append((format("ElectronicsBoards/ElectronicsBoard[%1%]/") % (iEB + 1)).str());
    string sebID = ebContent.getParamString("@id");
    int ebID = atoi(sebID.c_str());
    double ebInnerR = ebContent.getParamLength("EBInnerR");
    double ebOuterR = ebContent.getParamLength("EBOuterR");
    double ebBZ = ebContent.getParamLength("EBBackwardZ");
    double ebFZ = ebContent.getParamLength("EBForwardZ");

    // Construct electronics boards
    TGeoVolume* ebTube = gGeoManager->MakeTube((format("ElectronicsBoard_Layer%1%") % ebID).str().c_str(),
                                               medNEMA_G10_Plate, ebInnerR, ebOuterR, (ebFZ - ebBZ) / 2.0);
    ebTube->SetLineColor(kGreen);
    volGrpEB->AddNode(ebTube, ebID, new TGeoTranslation(0.0, 0.0, (ebFZ + ebBZ) / 2.0));
  }

  //--------------------------------------
  // Get parameters and construct covers
  //--------------------------------------
  int nCover = content.getNumberNodes("Covers/Cover");
  TGeoVolumeAssembly* volGrpCover = new TGeoVolumeAssembly("Covers");
  volGrpCDC->AddNode(volGrpCover, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  for (int iCover = 0; iCover < nCover; ++iCover) {
    // Get parameters
    GearDir coverContent(content);
    coverContent.append((format("Covers/Cover[%1%]/") % (iCover + 1)).str());
    string scoverID = coverContent.getParamString("@id");
    int coverID = atoi(scoverID.c_str());
    string coverName = coverContent.getParamString("Name");
    double coverInnerR1 = coverContent.getParamLength("InnerR1");
    double coverInnerR2 = coverContent.getParamLength("InnerR2");
    double coverThick = coverContent.getParamLength("Thickness");
    double coverAngle = coverContent.getParamAngle("Angle");
    double coverPosZ = coverContent.getParamLength("PosZ");

    if (coverName == "BackwardCover") {
      double rmin2 = coverInnerR1;
      double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
      double rmin1 = coverInnerR2;
      double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
      double coverLength = (rmin1 - rmin2) / std::tan(coverAngle);
      // Construct covers
      TGeoVolume* coverCone = gGeoManager->MakeCone((format("BackwardCover%1%") % coverID).str().c_str(),
                                                    medAluminum, coverLength / 2.0, rmin1, rmax1, rmin2, rmax2);
      coverCone->SetLineColor(kGray);
      volGrpCover->AddNode(coverCone, coverID, new TGeoTranslation(0.0, 0.0, coverPosZ - coverLength / 2.0));
    } else {
      double rmin1 = coverInnerR1;
      double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
      double rmin2 = coverInnerR2;
      double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
      double coverLength = (rmin2 - rmin1) / std::tan(coverAngle);
      // Construct covers
      TGeoVolume* coverCone = gGeoManager->MakeCone((format("ForwardCover%1%") % coverID).str().c_str(),
                                                    medAluminum, coverLength / 2.0, rmin1, rmax1, rmin2, rmax2);
      coverCone->SetLineColor(kGray);
      volGrpCover->AddNode(coverCone, coverID, new TGeoTranslation(0.0, 0.0, coverPosZ + coverLength / 2.0));
    }
  }
}
