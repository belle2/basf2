/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/geometry/GeoCDCCreator.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/simcdc/CDCSensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <cmath>
#include <boost/format.hpp>

#include <G4Material.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>

#include <iostream>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace cdc {
//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    geometry::CreatorFactory<GeoCDCCreator> GeoCDCFactory("CDCCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    GeoCDCCreator::GeoCDCCreator()
    {
      m_sensitive = new CDCSensitiveDetector("CDCSensitiveDetector", (2*24)*eV, 10*MeV);
      logical_cdc = 0;
      physical_cdc = 0;
    }


    GeoCDCCreator::~GeoCDCCreator()
    {

    }


    void GeoCDCCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {
      //------------------------
      // Get global parameters
      //------------------------
      double globalRotAngle = (180.0 / M_PI) * content.getAngle("Rotation");
      double globalOffsetZ  = content.getLength("OffsetZ");
      string Helium  = content.getString("Helium");
      string Ethane  = content.getString("Ethane");
      string Aluminum  = content.getString("Aluminum");
      string Tungsten  = content.getString("Tungsten");
      string CFRP  = content.getString("CFRP");
      string NEMA_G10_Plate  = content.getString("NEMA_G10_Plate");

      //TGeoRotation* geoRot = new TGeoRotation("CDCRot", 90.0, globalRotAngle, 0.0);
      //TGeoVolumeAssembly* volGrpCDC = addSubdetectorGroup("CDC", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));

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

        string sOuterWallID = outerWallContent.getString("@id");
        int outerWallID = atoi(sOuterWallID.c_str());
        outerWallName[outerWallID] = "OuterWall_" + sOuterWallID + "_" + outerWallContent.getString("Name");
        outerWallInnerR[outerWallID] = outerWallContent.getLength("InnerR");
        outerWallOuterR[outerWallID] = outerWallContent.getLength("OuterR");
        outerWallBZ[outerWallID]  = outerWallContent.getLength("BackwardZ");
        outerWallFZ[outerWallID]   = outerWallContent.getLength("ForwardZ");

        if (outerWallContent.getString("Name") == "Shield") rmin_outerWall = outerWallContent.getLength("InnerR");
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

        string sInnerWallID = innerWallContent.getString("@id");
        int innerWallID = atoi(sInnerWallID.c_str());
        innerWallName[innerWallID] = "InnerWall_" + sInnerWallID + "_" + innerWallContent.getString("Name");
        innerWallInnerR[innerWallID] = innerWallContent.getLength("InnerR");
        innerWallOuterR[innerWallID] = innerWallContent.getLength("OuterR");
        innerWallBZ[innerWallID]  = innerWallContent.getLength("BackwardZ");
        innerWallFZ[innerWallID]   = innerWallContent.getLength("ForwardZ");

        if (innerWallContent.getString("Name") == "Shield") rmax_innerWall = innerWallContent.getLength("OuterR");
      }

      //-----------------------------------------------------------------------
      // Get sense wire and field wire information, radius and total numbers.
      //-----------------------------------------------------------------------
      GearDir senseWire(content);
      senseWire.append("SenseWire/");
      double diameter_senseWire = senseWire.getLength("Diameter");
      int num_senseWire = atoi((senseWire.getString("Number")).c_str());

      GearDir fieldWire(content);
      fieldWire.append("FieldWire/");
      double diameter_fieldWire = fieldWire.getLength("Diameter");
      int num_fieldWire = atoi((fieldWire.getString("Number")).c_str());

      //----------------
      // Get Material
      //----------------
      G4Material* medHelium = geometry::Materials::get(Helium.c_str());
      G4Material* medEthane = geometry::Materials::get(Ethane.c_str());
      G4Material* medAluminum = geometry::Materials::get(Aluminum.c_str());
      G4Material* medTungsten = geometry::Materials::get(Tungsten.c_str());
      G4Material* medCFRP = geometry::Materials::get(CFRP.c_str());
      G4Material* medNEMA_G10_Plate = geometry::Materials::get(NEMA_G10_Plate.c_str());
      G4Material* medAir = geometry::Materials::get("Air");


      // Calculate average density

      // Total cross section
      double totalCS = M_PI * (rmin_outerWall * rmin_outerWall - rmax_innerWall * rmax_innerWall);

      // Sense wire cross section
      double senseCS = M_PI * (diameter_senseWire / 2) * (diameter_senseWire / 2) * num_senseWire;

      // Field wire cross section
      double fieldCS = M_PI * (diameter_fieldWire / 2) * (diameter_fieldWire / 2) * num_fieldWire;

      // Density
      double denHelium = medHelium->GetDensity() / 2.0;
      double denEthane = medEthane->GetDensity() / 2.0;
      double denAluminum = medAluminum->GetDensity() * (fieldCS / totalCS);
      double denTungsten = medTungsten->GetDensity() * (senseCS / totalCS);

      double density = denHelium + denEthane + denAluminum + denTungsten;

      // Create cdc gaswire
      G4Material* cdcMed = new G4Material("CDCGasWire", density, 4);
      cdcMed->AddMaterial(medHelium, denHelium / density);
      cdcMed->AddMaterial(medEthane, denEthane / density);
      cdcMed->AddMaterial(medTungsten, denTungsten / density);
      cdcMed->AddMaterial(medAluminum, denAluminum / density);

      //------------------------------
      // Construct mother volume
      //------------------------------
      CDCGeometryPar * cdcgp = CDCGeometryPar::Instance();
      double motherInnerR = cdcgp->motherInnerR();
      double motherOuterR = cdcgp->motherOuterR();
      double motherLength = cdcgp->motherLength();

      G4Tubs* solid_cdc = new G4Tubs("SolidCDC", motherInnerR, motherOuterR, motherLength / 2.0, 0*deg, 360.*deg);
      logical_cdc = new G4LogicalVolume(solid_cdc, medAir, "logicalCDC", 0, 0, 0);
      physical_cdc = new G4PVPlacement(0, G4ThreeVector(0., 0., globalOffsetZ), logical_cdc, "physicalCDC", &topVolume, false, 0);

      //-----------------------
      // Construct outer wall
      //----------------------
      for (int iOuterWall = 0; iOuterWall < nOuterWall; ++iOuterWall) {
        double length = (outerWallFZ[iOuterWall] - outerWallBZ[iOuterWall]) / 2.0;
        if (strstr((outerWallName[iOuterWall]).c_str(), "MiddleWall") != NULL) {
          std::ostringstream outerWallName1;
          outerWallName1 << "solid" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName2;
          outerWallName2 << "logical" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName3;
          outerWallName3 << "physical" << (outerWallName[iOuterWall]).c_str();
          G4Tubs* outerWallTubeShape = new G4Tubs(outerWallName1.str(), outerWallInnerR[iOuterWall], outerWallOuterR[iOuterWall], length, 0*deg, 360.*deg);
          G4LogicalVolume* outerWallTube = new G4LogicalVolume(outerWallTubeShape, medCFRP, outerWallName2.str(), 0, 0, 0);
          outerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          G4VPhysicalVolume* phyouterWallTube;
          phyouterWallTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + outerWallBZ[iOuterWall])), outerWallTube, outerWallName3.str(), logical_cdc, false, iOuterWall);
        } else {
          std::ostringstream outerWallName1;
          outerWallName1 << "solid" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName2;
          outerWallName2 << "logical" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName3;
          outerWallName3 << "physical" << (outerWallName[iOuterWall]).c_str();
          G4Tubs* outerWallTubeShape = new G4Tubs(outerWallName1.str(), outerWallInnerR[iOuterWall], outerWallOuterR[iOuterWall], length, 0*deg, 360.*deg);
          G4LogicalVolume* outerWallTube = new G4LogicalVolume(outerWallTubeShape, medAluminum, outerWallName2.str(), 0, 0, 0);
          outerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          G4VPhysicalVolume* phyouterWallTube;
          phyouterWallTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + outerWallBZ[iOuterWall])), outerWallTube, outerWallName3.str(), logical_cdc, false, iOuterWall);
        }
      }

      //-----------------------
      // Construct inner wall
      //-----------------------
      for (int iInnerWall = 0; iInnerWall < nInnerWall; ++iInnerWall) {
        double length = (innerWallFZ[iInnerWall] - innerWallBZ[iInnerWall]) / 2.0;
        if (strstr((innerWallName[iInnerWall]).c_str(), "MiddleWall") != NULL) {
          std::ostringstream innerWallName1;
          innerWallName1 << "solid" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName2;
          innerWallName2 << "logical" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName3;
          innerWallName3 << "physical" << (innerWallName[iInnerWall]).c_str();
          G4Tubs* innerWallTubeShape = new G4Tubs(innerWallName1.str(), innerWallInnerR[iInnerWall], innerWallOuterR[iInnerWall], length, 0*deg, 360.*deg);
          G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medCFRP, innerWallName2.str(), 0, 0, 0);
          innerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          G4VPhysicalVolume* phyinnerWallTube;
          phyinnerWallTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + innerWallBZ[iInnerWall])), innerWallTube, innerWallName3.str(), logical_cdc, false, iInnerWall);
        } else {
          std::ostringstream innerWallName1;
          innerWallName1 << "solid" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName2;
          innerWallName2 << "logical" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName3;
          innerWallName3 << "physical" << (innerWallName[iInnerWall]).c_str();
          G4Tubs* innerWallTubeShape = new G4Tubs(innerWallName1.str(), innerWallInnerR[iInnerWall], innerWallOuterR[iInnerWall], length, 0*deg, 360.*deg);
          G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medAluminum, innerWallName2.str(), 0, 0, 0);
          innerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          G4VPhysicalVolume* phyinnerWallTube;
          phyinnerWallTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + innerWallBZ[iInnerWall])), innerWallTube, innerWallName3.str(), logical_cdc, false, iInnerWall);
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

        string layerID = layerContent.getString("@id");
        int ilayerID = atoi(layerID.c_str());
        slayerRadius[ilayerID] = layerContent.getLength("Radius");
        slayerZBack[ilayerID]  = layerContent.getLength("BackwardZ");
        slayerZFor[ilayerID]   = layerContent.getLength("ForwardZ");

        // Get parameters of endplates
        GearDir epContent(content);
        epContent.append((format("Endplates/Endplate[%1%]/") % (iSLayer + 1)).str());
        string sepID = epContent.getString("@id");
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
          string sepLayerID = epLayerContent.getString("@id");
          int epLayerID = atoi(sepLayerID.c_str());
          epName[epID][epLayerID] = epLayerContent.getString("Name");
          epInnerR[epID][epLayerID] = epLayerContent.getLength("InnerR");
          epOuterR[epID][epLayerID] = epLayerContent.getLength("OuterR");
          epBZ[epID][epLayerID] = epLayerContent.getLength("BackwardZ");
          epFZ[epID][epLayerID] = epLayerContent.getLength("ForwardZ");
        }
      }

      //--------------------------------
      // Get parameters for field layer
      //--------------------------------
      for (int iFLayer = 0; iFLayer < nFLayer; ++iFLayer) {
        GearDir layerContent(content);
        layerContent.append((format("FLayers/FLayer[%1%]/") % (iFLayer + 1)).str());

        string layerID = layerContent.getString("@id");
        int ilayerID = atoi(layerID.c_str());
        flayerRadius[ilayerID] = layerContent.getLength("Radius");
        flayerZBack[ilayerID]  = layerContent.getLength("BackwardZ");
        flayerZFor[ilayerID]   = layerContent.getLength("ForwardZ");
      }

      //----------------------------
      // Get length of feedthrough
      //----------------------------
      GearDir feedthroughContent(content);
      feedthroughContent.append("FeedThrough/");

      double length_feedthrough  = feedthroughContent.getLength("Length");

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
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left, rmax_sensitive_left, length_feedthrough / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyleftTube;
          phyleftTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_left + length_feedthrough / 2.0)), leftTube, (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Build left sensitive tube (area 2)
          G4Tubs* leftSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), rmin_sensitive_left, rmax_sensitive_left, (zfor_sensitive_left - zback_sensitive_left - length_feedthrough) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* leftSensitiveTube = new G4LogicalVolume(leftSensitiveTubeShape, cdcMed, (format("logicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), 0, 0, 0);
          leftSensitiveTube->SetSensitiveDetector(m_sensitive);
          G4VPhysicalVolume* phyleftSensitiveTube;
          phyleftSensitiveTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left + length_feedthrough) / 2.0), leftSensitiveTube, (format("physicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
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
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left, rmax_sensitive_left, (zfor_sensitive_left - zback_sensitive_left) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyleftTube;
          phyleftTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left) / 2.0), leftTube, (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Build a tube with metarial cdcMed for area 2
          G4Tubs* leftMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), rmin_sensitive_middle, rmax_sensitive_middle, (length_feedthrough - zfor_sensitive_left + zback_sensitive_left) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* leftMidTube = new G4LogicalVolume(leftMidTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyleftMidTube;
          phyleftMidTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length_feedthrough + zfor_sensitive_left + zback_sensitive_left) / 2.0), leftMidTube, (format("physicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

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
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), rmin_sensitive_right, rmax_sensitive_right, length_feedthrough / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyrightTube;
          phyrightTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right - length_feedthrough / 2.0)), rightTube, (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Build right sensitive tube (area 2)
          G4Tubs* rightSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), rmin_sensitive_right, rmax_sensitive_right, (zfor_sensitive_right - zback_sensitive_right - length_feedthrough) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* rightSensitiveTube = new G4LogicalVolume(rightSensitiveTubeShape, cdcMed, (format("logicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), 0, 0, 0);
          rightSensitiveTube->SetSensitiveDetector(m_sensitive);
          G4VPhysicalVolume* phyrightSensitiveTube;
          phyrightSensitiveTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right - length_feedthrough) / 2.0), rightSensitiveTube, (format("physicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
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
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), rmin_sensitive_right, rmax_sensitive_right, (zfor_sensitive_right - zback_sensitive_right) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyrightTube;
          phyrightTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right) / 2.0), rightTube, (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Build a tube with metarial cdcMed for area 2
          G4Tubs* rightMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), rmin_sensitive_middle, rmax_sensitive_middle, (length_feedthrough - zfor_sensitive_right + zback_sensitive_right) / 2.0, 0*deg, 360.*deg);
          G4LogicalVolume* rightMidTube = new G4LogicalVolume(rightMidTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), 0, 0, 0);
          G4VPhysicalVolume* phyrightMidTube;
          phyrightMidTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_right - length_feedthrough + zfor_sensitive_right) / 2.0), rightMidTube, (format("physicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Reset zback_sensitive_middle
          zfor_sensitive_middle = zfor_sensitive_right - length_feedthrough;
        }

        // Middle sensitive tube
        G4Tubs* middleSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), rmin_sensitive_middle, rmax_sensitive_middle, (zfor_sensitive_middle - zback_sensitive_middle) / 2.0, 0*deg, 360.*deg);
        G4LogicalVolume* middleSensitiveTube = new G4LogicalVolume(middleSensitiveTubeShape, cdcMed, (format("logicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), 0, 0, 0);
        middleSensitiveTube->SetSensitiveDetector(m_sensitive);
        G4VPhysicalVolume* phymiddleSensitiveTube;
        phymiddleSensitiveTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_middle + zback_sensitive_middle) / 2.0), middleSensitiveTube, (format("physicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

        // Endplates
        for (int iEPLayer = 0; iEPLayer < nEPLayer; ++iEPLayer) {
          std::ostringstream endplateName;
          endplateName << "solidCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          std::ostringstream endplateName1;
          endplateName1 << "logicalCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          std::ostringstream endplateName2;
          endplateName2 << "physicalCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          double length_endplate = (epFZ[iSLayer][iEPLayer] - epBZ[iSLayer][iEPLayer]) / 2.0;
          G4Tubs* endplateTubeShape = new G4Tubs(endplateName.str().c_str(), epInnerR[iSLayer][iEPLayer], epOuterR[iSLayer][iEPLayer], length_endplate, 0*deg, 360.*deg);
          G4LogicalVolume* endplateTube = new G4LogicalVolume(endplateTubeShape, medAluminum, endplateName1.str().c_str(), 0, 0);
          endplateTube->SetVisAttributes(G4VisAttributes(G4Colour(1., 1., 0.)));
          G4VPhysicalVolume* phyendplateTube;
          phyendplateTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (epFZ[iSLayer][iEPLayer] + epBZ[iSLayer][iEPLayer]) / 2.0), endplateTube, endplateName2.str().c_str(), logical_cdc, false, iSLayer);
        }
      }

      //--------------------------------------------------
      // Get paprameters and construct electronics boards
      //--------------------------------------------------
      int nEB = content.getNumberNodes("ElectronicsBoards/ElectronicsBoard");
      for (int iEB = 0; iEB < nEB; ++iEB) {
        // Get parameters
        GearDir ebContent(content);
        ebContent.append((format("ElectronicsBoards/ElectronicsBoard[%1%]/") % (iEB + 1)).str());
        string sebID = ebContent.getString("@id");
        int ebID = atoi(sebID.c_str());
        double ebInnerR = ebContent.getLength("EBInnerR");
        double ebOuterR = ebContent.getLength("EBOuterR");
        double ebBZ = ebContent.getLength("EBBackwardZ");
        double ebFZ = ebContent.getLength("EBForwardZ");

        // Construct electronics boards
        G4Tubs* ebTubeShape = new G4Tubs((format("solidSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), ebInnerR, ebOuterR, (ebFZ - ebBZ) / 2.0, 0*deg, 360.*deg);
        G4LogicalVolume* ebTube = new G4LogicalVolume(ebTubeShape, medNEMA_G10_Plate, (format("logicalSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), 0, 0, 0);
        ebTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
        G4VPhysicalVolume* phyebTube;
        phyebTube = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (ebFZ + ebBZ) / 2.0), ebTube, (format("physicalSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), logical_cdc, false, ebID);
      }

      //--------------------------------------
      // Get parameters and construct covers
      //--------------------------------------
      int nCover = content.getNumberNodes("Covers/Cover");
      for (int iCover = 0; iCover < nCover; ++iCover) {
        // Get parameters
        GearDir coverContent(content);
        coverContent.append((format("Covers/Cover[%1%]/") % (iCover + 1)).str());
        string scoverID = coverContent.getString("@id");
        int coverID = atoi(scoverID.c_str());
        string coverName = coverContent.getString("Name");
        double coverInnerR1 = coverContent.getLength("InnerR1");
        double coverInnerR2 = coverContent.getLength("InnerR2");
        double coverThick = coverContent.getLength("Thickness");
        double coverAngle = coverContent.getAngle("Angle");
        double coverPosZ = coverContent.getLength("PosZ");

        if (coverName == "BackwardCover") {
          double rmin2 = coverInnerR1;
          double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
          double rmin1 = coverInnerR2;
          double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
          double coverLength = (rmin1 - rmin2) / std::tan(coverAngle);
          // Construct covers
          G4Cons* coverConeShape = new G4Cons((format("solidBackwardCover%1%") % coverID).str().c_str(), rmin1, rmax1, rmin2, rmax2, coverLength / 2.0, 0.*deg, 360.*deg);
          G4LogicalVolume* coverCone = new G4LogicalVolume(coverConeShape, medAluminum, (format("logicalBackwardCover%1%") % coverID).str().c_str(), 0, 0, 0);
          coverCone->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 1.)));
          G4VPhysicalVolume* phycoverCone;
          phycoverCone = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, coverPosZ - coverLength / 2.0), coverCone, (format("physicalBackwardCover%1%") % coverID).str().c_str(), logical_cdc, false, coverID);
        } else {
          double rmin1 = coverInnerR1;
          double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
          double rmin2 = coverInnerR2;
          double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
          double coverLength = (rmin2 - rmin1) / std::tan(coverAngle);
          // Construct covers
          G4Cons* coverConeShape = new G4Cons((format("solidForwardCover%1%") % coverID).str().c_str(), rmin1, rmax1, rmin2, rmax2, coverLength / 2.0, 0.*deg, 360.*deg);
          G4LogicalVolume* coverCone = new G4LogicalVolume(coverConeShape, medAluminum, (format("logicalForwardCover%1%") % coverID).str().c_str(), 0, 0, 0);
          coverCone->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 1.)));
          G4VPhysicalVolume* phycoverCone;
          phycoverCone = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, coverPosZ + coverLength / 2.0), coverCone, (format("physicalForwardCover%1%") % coverID).str().c_str(), logical_cdc, false, coverID);
        }
      }
    }

  }
}
