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
#include <cdc/simulation/CDCSensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <cmath>
#include <boost/format.hpp>

#include <G4Material.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4Cons.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>
#include <G4NistManager.hh>
#include <G4RotationMatrix.hh>
#include <iostream>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace CDC {

    /**
     * Register the GeoCreator.
     */

    geometry::CreatorFactory<GeoCDCCreator> GeoCDCFactory("CDCCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoCDCCreator::GeoCDCCreator()
    {
      m_sensitive = new CDCSensitiveDetector("CDCSensitiveDetector", (2 * 24)*eV, 10 * MeV);
      logical_cdc = 0;
      physical_cdc = 0;

    }


    GeoCDCCreator::~GeoCDCCreator()
    {

    }


    void GeoCDCCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /*type*/)
    {

      //may be convenient to set true for geometry debug
      const bool simplifiedGeometry = false;
      //      const bool simplifiedGeometry = true;

      //------------------------
      // Get global parameters
      //------------------------
      //      double globalRotAngle = (180.0 / M_PI) * content.getAngle("Rotation");
      double globalOffsetZ  = content.getLength("OffsetZ");
      string Helium  = content.getString("Helium");
      string Ethane  = content.getString("Ethane");
      string Aluminum  = content.getString("Aluminum");
      string Tungsten  = content.getString("Tungsten");
      string CFRP  = content.getString("CFRP");
      string NEMA_G10_Plate  = content.getString("NEMA_G10_Plate");
      string CDCGlue  = content.getString("CDCGlue");

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
      double rmin_outerWall = 1500.0 * mm;

      for (int iOuterWall = 0; iOuterWall < nOuterWall; ++iOuterWall) {
        GearDir outerWallContent(content);
        outerWallContent.append((format("/OuterWalls/OuterWall[%1%]/") % (iOuterWall + 1)).str());

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
      double rmax_innerWall = 0.0;

      for (int iInnerWall = 0; iInnerWall < nInnerWall; ++iInnerWall) {
        GearDir innerWallContent(content);
        innerWallContent.append((format("/InnerWalls/InnerWall[%1%]/") % (iInnerWall + 1)).str());

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
      senseWire.append("/SenseWire/");
      double diameter_senseWire = senseWire.getLength("Diameter");
      int num_senseWire = atoi((senseWire.getString("Number")).c_str());

      GearDir fieldWire(content);
      fieldWire.append("/FieldWire/");
      double diameter_fieldWire = fieldWire.getLength("Diameter");
      int num_fieldWire = atoi((fieldWire.getString("Number")).c_str());

      //----------------
      // Get Material
      //----------------

      G4Material* medHelium = G4Material::GetMaterial(Helium.c_str());
      G4Material* medEthane = geometry::Materials::get(Ethane.c_str());
      G4Material* medAluminum = G4Material::GetMaterial(Aluminum.c_str());
      G4Material* medTungsten = G4Material::GetMaterial(Tungsten.c_str());
      G4Material* medCFRP = geometry::Materials::get(CFRP.c_str());
      G4Material* medNEMA_G10_Plate = geometry::Materials::get(NEMA_G10_Plate.c_str());
      G4Material* medGlue = geometry::Materials::get(CDCGlue.c_str());
      G4Material* medAir = geometry::Materials::get("Air");

      //
      // Calculate average density
      //

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

      /*
      // Define material for inner volume (for tentative use)
      double rBound = 24.3; //boundary betw. inner and outer
      int nSenseWiresInInner = 8 * 160;
      int nFieldWiresInInner = 8 * (160 + 320) - 160;
      totalCS = (rBound * rBound - rmax_innerWall * rmax_innerWall);
      senseCS = (diameter_senseWire / 2) * (diameter_senseWire / 2) * nSenseWiresInInner;
      fieldCS = (diameter_fieldWire / 2) * (diameter_fieldWire / 2) * nFieldWiresInInner;
      denAluminum = medAluminum->GetDensity() * (fieldCS / totalCS);
      denTungsten = medTungsten->GetDensity() * (senseCS / totalCS);
      density = denHelium + denEthane + denAluminum + denTungsten;
      G4Material* med4Inner = new G4Material("CDCGasWire4Inner", density, 4);
      med4Inner->AddMaterial(medHelium, denHelium / density);
      med4Inner->AddMaterial(medEthane, denEthane / density);
      med4Inner->AddMaterial(medTungsten, denTungsten / density);
      med4Inner->AddMaterial(medAluminum, denAluminum / density);

      // Define material for outer volume(for tentative use)
      totalCS = (rmin_outerWall * rmin_outerWall - rBound * rBound);
      senseCS = (diameter_senseWire / 2) * (diameter_senseWire / 2) * (num_senseWire - nSenseWiresInInner);
      fieldCS = (diameter_fieldWire / 2) * (diameter_fieldWire / 2) * (num_fieldWire - nFieldWiresInInner);
      denAluminum = medAluminum->GetDensity() * (fieldCS / totalCS);
      denTungsten = medTungsten->GetDensity() * (senseCS / totalCS);
      density = denHelium + denEthane + denAluminum + denTungsten;
      G4Material* med4Outer = new G4Material("CDCGasWire4Outer", density, 4);
      med4Outer->AddMaterial(medHelium, denHelium / density);
      med4Outer->AddMaterial(medEthane, denEthane / density);
      med4Outer->AddMaterial(medTungsten, denTungsten / density);
      med4Outer->AddMaterial(medAluminum, denAluminum / density);
      */

      // Create cdc gas
      double density = denHelium + denEthane + denAluminum + denTungsten;
      G4Material* cdcMed = new G4Material("CDCGasWire", density, 4);
      cdcMed->AddMaterial(medHelium, denHelium / density);
      cdcMed->AddMaterial(medEthane, denEthane / density);
      cdcMed->AddMaterial(medTungsten, denTungsten / density);
      cdcMed->AddMaterial(medAluminum, denAluminum / density);

      G4Material* cdcMedGas = cdcMed;
      CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();

      if (cdcgp.getMaterialDefinitionMode() == 2) {
        double density = denHelium + denEthane;
        cdcMedGas = new G4Material("realCDCGas", density, 2);
        cdcMedGas->AddMaterial(medHelium, denHelium / density);
        cdcMedGas->AddMaterial(medEthane, denEthane / density);
        /*
        G4double a = 1.01 * g/mole;
        G4Element* elH = new G4Element("Hydrogen", "H", 1., a);
        a = 16.00 * g/mole;
        G4Element* elO = new G4Element("Oxygen", "O", 8., a);
        density = 1.00 * g/cm3;
        G4Material* H2O = new G4Material("Water", density, 2);
        H2O->AddElement(elH, 2);
        H2O->AddElement(elO, 1);
        */
      }

      //------------------------------
      // Construct mother volume
      //------------------------------
      //      CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();
      //      double motherInnerR = cdcgp.motherInnerR();
      //      double motherOuterR = cdcgp.motherOuterR();
      //      double motherLength = cdcgp.motherLength();

      //G4Tubs* solid_cdc = new G4Tubs("SolidCDC", motherInnerR*cm, motherOuterR*cm, motherLength*cm / 2.0, 0*deg, 360.*deg);
      //replace Tube with Polycone

      double momZ[7];
      double momRmin[7];
      double momRmax[7];

      for (int iBound = 0 ; iBound < 7 ; iBound++) {
        momZ[iBound] = cdcgp.momZ(iBound);
        momRmin[iBound] = cdcgp.momRmin(iBound);
        momRmax[iBound] = 1140.0;
      }

      G4Polycone* solid_cdc =
        new G4Polycone("SolidCDC", 0 * deg, 360.*deg, 7, momZ, momRmin, momRmax);

      logical_cdc = new G4LogicalVolume(solid_cdc, medAir, "logicalCDC", 0, 0, 0);
      physical_cdc = new G4PVPlacement(0, G4ThreeVector(0., 0., globalOffsetZ * cm), logical_cdc, "physicalCDC", &topVolume, false, 0);


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
          G4Tubs* outerWallTubeShape = new G4Tubs(outerWallName1.str(), outerWallInnerR[iOuterWall]*cm, outerWallOuterR[iOuterWall]*cm, length * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* outerWallTube = new G4LogicalVolume(outerWallTubeShape, medCFRP, outerWallName2.str(), 0, 0, 0);
          outerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + outerWallBZ[iOuterWall])*cm), outerWallTube, outerWallName3.str(), logical_cdc, false, iOuterWall);
          }
        } else {
          std::ostringstream outerWallName1;
          outerWallName1 << "solid" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName2;
          outerWallName2 << "logical" << (outerWallName[iOuterWall]).c_str();
          std::ostringstream outerWallName3;
          outerWallName3 << "physical" << (outerWallName[iOuterWall]).c_str();
          G4Tubs* outerWallTubeShape = new G4Tubs(outerWallName1.str(), outerWallInnerR[iOuterWall]*cm, outerWallOuterR[iOuterWall]*cm, length * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* outerWallTube = new G4LogicalVolume(outerWallTubeShape, medAluminum, outerWallName2.str(), 0, 0, 0);
          outerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + outerWallBZ[iOuterWall])*cm), outerWallTube, outerWallName3.str(), logical_cdc, false, iOuterWall);
          }
        }
      }

      //-----------------------
      // Construct inner wall
      //-----------------------
      for (int iInnerWall = 0; iInnerWall < nInnerWall; ++iInnerWall) {
        double length = (innerWallFZ[iInnerWall] - innerWallBZ[iInnerWall]) / 2.0;
        //        std::cout << "half z " << length << std::endl;
        // CFRP layer 0.46 mmt
        if (strstr((innerWallName[iInnerWall]).c_str(), "MiddleWall") != NULL) {
          std::ostringstream innerWallName1;
          innerWallName1 << "solid" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName2;
          innerWallName2 << "logical" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName3;
          innerWallName3 << "physical" << (innerWallName[iInnerWall]).c_str();
          G4Tubs* innerWallTubeShape = new G4Tubs(innerWallName1.str(), innerWallInnerR[iInnerWall]*cm, innerWallOuterR[iInnerWall]*cm, length * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medCFRP, innerWallName2.str(), 0, 0, 0);
          innerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          // Commented by M. U. June 3rd, 2013
          //          G4VPhysicalVolume* phyinnerWallTube;
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + innerWallBZ[iInnerWall])*cm), innerWallTube, innerWallName3.str(), logical_cdc, false, iInnerWall);
          }
        } else if (strstr((innerWallName[iInnerWall]).c_str(), "MiddleGlue") != NULL) { // Glue layer 0.005 mmt
          std::ostringstream innerWallName1;
          innerWallName1 << "solid" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName2;
          innerWallName2 << "logical" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName3;
          innerWallName3 << "physical" << (innerWallName[iInnerWall]).c_str();
          G4Tubs* innerWallTubeShape = new G4Tubs(innerWallName1.str(), innerWallInnerR[iInnerWall]*cm, innerWallOuterR[iInnerWall]*cm, length * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medGlue, innerWallName2.str(), 0, 0, 0);
          innerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          // Commented by M. U. June 3rd, 2013
          // G4VPhysicalVolume* phyinnerWallTube;
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + innerWallBZ[iInnerWall])*cm), innerWallTube, innerWallName3.str(), logical_cdc, false, iInnerWall);
          }

        } else { // Al layer 0.1 mmt
          std::ostringstream innerWallName1;
          innerWallName1 << "solid" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName2;
          innerWallName2 << "logical" << (innerWallName[iInnerWall]).c_str();
          std::ostringstream innerWallName3;
          innerWallName3 << "physical" << (innerWallName[iInnerWall]).c_str();
          G4Tubs* innerWallTubeShape = new G4Tubs(innerWallName1.str(), innerWallInnerR[iInnerWall]*cm, innerWallOuterR[iInnerWall]*cm, length * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medAluminum, innerWallName2.str(), 0, 0, 0);
          innerWallTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
          // Commented by M. U. June 3rd, 2013
          //          G4VPhysicalVolume* phyinnerWallTube;
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + innerWallBZ[iInnerWall])*cm), innerWallTube, innerWallName3.str(), logical_cdc, false, iInnerWall);
          }
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
        layerContent.append((format("/SLayers/SLayer[%1%]/") % (iSLayer + 1)).str());

        string layerID = layerContent.getString("@id");
        int ilayerID = atoi(layerID.c_str());
        slayerRadius[ilayerID] = layerContent.getLength("Radius");
        slayerZBack[ilayerID]  = layerContent.getLength("BackwardZ");
        slayerZFor[ilayerID]   = layerContent.getLength("ForwardZ");


        // Get parameters of endplates

        GearDir epContent(content);
        epContent.append((format("/Endplates/Endplate[%1%]/") % (iSLayer + 1)).str());
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
          epLayerContent.append((format("/EndplateLayer[%1%]/") % (iEPLayer + 1)).str());
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
        layerContent.append((format("/FLayers/FLayer[%1%]/") % (iFLayer + 1)).str());

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
      feedthroughContent.append("/FeedThrough/");

      double length_feedthrough  = feedthroughContent.getLength("Length");

      //------------------------------------------
      // Construct sensitive layers and endplates
      //------------------------------------------

      for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
        //  if (cdcgp.getMaterialDefinitionMode() == 1) {
        //    cdcMed = (iSLayer <= 7) ? med4Inner : med4Outer;
        //  }
        // Get the number of endplate layers
        int nEPLayer = nEndplateLayer[iSLayer];


        // Get parameters for sensitive layer: left, middle and right.
        double rmin_sensitive_left, rmax_sensitive_left;
        double rmin_sensitive_middle, rmax_sensitive_middle;
        double rmin_sensitive_right, rmax_sensitive_right;
        double zback_sensitive_left, zfor_sensitive_left;
        double zback_sensitive_middle, zfor_sensitive_middle;
        double zback_sensitive_right, zfor_sensitive_right;

        if (iSLayer == 0) {

          rmin_sensitive_left = epOuterR[iSLayer][0];
          rmax_sensitive_left = flayerRadius[iSLayer];
          zback_sensitive_left = slayerZBack[iSLayer];
          zfor_sensitive_left = epFZ[iSLayer][0];

          rmin_sensitive_middle = innerWallOuterR[0];
          rmax_sensitive_middle = flayerRadius[iSLayer];
          zback_sensitive_middle = epFZ[iSLayer][0];
          zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2];

          rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2];
          rmax_sensitive_right = flayerRadius[iSLayer];
          zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2];
          zfor_sensitive_right = slayerZFor[iSLayer];

        } else if (iSLayer >= 1 && iSLayer <= 14) {

          rmin_sensitive_left = epOuterR[iSLayer][1];
          rmax_sensitive_left = flayerRadius[iSLayer];
          zback_sensitive_left = slayerZBack[iSLayer];
          zfor_sensitive_left = epFZ[iSLayer][1];

          rmin_sensitive_middle = flayerRadius[iSLayer - 1];
          rmax_sensitive_middle = flayerRadius[iSLayer];
          zback_sensitive_middle = epFZ[iSLayer][1];
          zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2 + 1];

          rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2 + 1];
          rmax_sensitive_right = flayerRadius[iSLayer];
          zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2 + 1];
          zfor_sensitive_right = slayerZFor[iSLayer];
        } else if (iSLayer >= 15 && iSLayer <= 18) {
          if (iSLayer == 15) {
            rmin_sensitive_left = epOuterR[iSLayer][1];
            rmax_sensitive_left = flayerRadius[iSLayer];
            zback_sensitive_left = slayerZBack[iSLayer];
            zfor_sensitive_left = epFZ[iSLayer][1];

            rmin_sensitive_middle = flayerRadius[iSLayer - 1];
            rmax_sensitive_middle = flayerRadius[iSLayer];
            zback_sensitive_middle = epFZ[iSLayer][1];
            zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2];

            rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2];
            rmax_sensitive_right = flayerRadius[iSLayer];
            zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2];
            zfor_sensitive_right = slayerZFor[iSLayer];
          } else {
            // to remove the overlap. T.Hara
            //rmin_sensitive_left = epOuterR[iSLayer][0];
            rmin_sensitive_left = epOuterR[iSLayer][1];
            rmax_sensitive_left = flayerRadius[iSLayer];
            zback_sensitive_left = slayerZBack[iSLayer];
            //zfor_sensitive_left = epFZ[iSLayer][0];
            zfor_sensitive_left = epFZ[iSLayer][1];

            rmin_sensitive_middle = flayerRadius[iSLayer - 1];
            rmax_sensitive_middle = flayerRadius[iSLayer];
            //zback_sensitive_middle = epFZ[iSLayer][0];
            zback_sensitive_middle = epFZ[iSLayer][1];
            zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2];

            rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2];
            rmax_sensitive_right = flayerRadius[iSLayer];
            zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2];
            zfor_sensitive_right = slayerZFor[iSLayer];
          }
        } else if (iSLayer >= 19 && iSLayer < (nSLayer - 1)) {
          rmin_sensitive_left = epOuterR[iSLayer][0];
          rmax_sensitive_left = flayerRadius[iSLayer];
          zback_sensitive_left = slayerZBack[iSLayer];
          zfor_sensitive_left = epFZ[iSLayer][0];

          rmin_sensitive_middle = flayerRadius[iSLayer - 1];
          rmax_sensitive_middle = flayerRadius[iSLayer];
          zback_sensitive_middle = epFZ[iSLayer][0];
          zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2];

          rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2];
          rmax_sensitive_right = flayerRadius[iSLayer];
          zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2];
          zfor_sensitive_right = slayerZFor[iSLayer];
        } else {
          rmin_sensitive_left = epOuterR[iSLayer][0];
          //rmax_sensitive_left = epInnerR[iSLayer][nEPLayer/2-1]; to remove overlap : T.Hara
          rmax_sensitive_left = epOuterR[iSLayer][nEPLayer / 2 - 1];
          zback_sensitive_left = slayerZBack[iSLayer];
          zfor_sensitive_left = epFZ[iSLayer][0];

          rmin_sensitive_middle = flayerRadius[iSLayer - 1];
          rmax_sensitive_middle = outerWallInnerR[0];
          zback_sensitive_middle = epFZ[iSLayer][0];
          zfor_sensitive_middle = epBZ[iSLayer][nEPLayer / 2];

          rmin_sensitive_right = epOuterR[iSLayer][nEPLayer / 2];
          rmax_sensitive_right = epOuterR[iSLayer][nEPLayer - 1];
          zback_sensitive_right = epBZ[iSLayer][nEPLayer / 2];
          zfor_sensitive_right = slayerZFor[iSLayer];
        }




        // Check if build left sensitive tube
        if ((zfor_sensitive_left - zback_sensitive_left) > length_feedthrough) {
          //    std::cout <<"left doif " << iSLayer <<" "<< zfor_sensitive_left - zback_sensitive_left << std::endl;
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
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left * cm, rmax_sensitive_left * cm, length_feedthrough * cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_left + length_feedthrough / 2.0)*cm), leftTube, (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Build left sensitive tube (area 2)
          G4Tubs* leftSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), rmin_sensitive_left * cm, rmax_sensitive_left * cm, (zfor_sensitive_left - zback_sensitive_left - length_feedthrough)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* leftSensitiveTube = new G4LogicalVolume(leftSensitiveTubeShape, cdcMed, (format("logicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), 0, 0, 0);
          leftSensitiveTube->SetSensitiveDetector(m_sensitive);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left + length_feedthrough)*cm / 2.0), leftSensitiveTube, (format("physicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }
        } else {
          //    std::cout <<"left doelse " << iSLayer << std::endl;
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
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left * cm, rmax_sensitive_left * cm, (zfor_sensitive_left - zback_sensitive_left)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left)*cm / 2.0), leftTube, (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Build a tube with metarial cdcMed for area 2
          G4Tubs* leftMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), rmin_sensitive_middle * cm, rmax_sensitive_middle * cm, (length_feedthrough - zfor_sensitive_left + zback_sensitive_left)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* leftMidTube = new G4LogicalVolume(leftMidTubeShape, cdcMed, (format("logicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length_feedthrough + zfor_sensitive_left + zback_sensitive_left)*cm / 2.0), leftMidTube, (format("physicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Reset zback_sensitive_middle
          zback_sensitive_middle = length_feedthrough + zback_sensitive_left;
        }

        // Check if build right sensitive tube
        if ((zfor_sensitive_right - zback_sensitive_right) > length_feedthrough) {
          //    std::cout <<"right doif" << iSLayer <<" "<< zfor_sensitive_right - zback_sensitive_right << std::endl;
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
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), rmin_sensitive_right * cm, rmax_sensitive_right * cm, length_feedthrough * cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right - length_feedthrough / 2.0)*cm), rightTube, (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Build right sensitive tube (area 2)
          G4Tubs* rightSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), rmin_sensitive_right * cm, rmax_sensitive_right * cm, (zfor_sensitive_right - zback_sensitive_right - length_feedthrough)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* rightSensitiveTube = new G4LogicalVolume(rightSensitiveTubeShape, cdcMed, (format("logicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), 0, 0, 0);
          rightSensitiveTube->SetSensitiveDetector(m_sensitive);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right - length_feedthrough)*cm / 2.0), rightSensitiveTube, (format("physicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }
        } else {
          //    std::cout <<"right doelse" << iSLayer << std::endl;
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
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), rmin_sensitive_right * cm, rmax_sensitive_right * cm, (zfor_sensitive_right - zback_sensitive_right)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right)*cm / 2.0), rightTube, (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Build a tube with metarial cdcMed for area 2
          G4Tubs* rightMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), rmin_sensitive_middle * cm, rmax_sensitive_middle * cm, (length_feedthrough - zfor_sensitive_right + zback_sensitive_right)*cm / 2.0, 0 * deg, 360.*deg);
          G4LogicalVolume* rightMidTube = new G4LogicalVolume(rightMidTubeShape, cdcMed, (format("logicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), 0, 0, 0);
          if (!simplifiedGeometry) {
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_right - length_feedthrough + zfor_sensitive_right)*cm / 2.0), rightMidTube, (format("physicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          }

          // Reset zback_sensitive_middle
          zfor_sensitive_middle = zfor_sensitive_right - length_feedthrough;
        }


        std::cout << iSLayer << " " << zfor_sensitive_middle << " " <<
                  zback_sensitive_middle << std::endl;

        // Middle sensitive tube
        G4Tubs* middleSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), rmin_sensitive_middle * cm, rmax_sensitive_middle * cm, (zfor_sensitive_middle - zback_sensitive_middle)*cm / 2.0, 0 * deg, 360.*deg);
        G4LogicalVolume* middleSensitiveTube = new G4LogicalVolume(middleSensitiveTubeShape, cdcMedGas, (format("logicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), 0, 0, 0);
        middleSensitiveTube->SetSensitiveDetector(m_sensitive);
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_middle + zback_sensitive_middle)*cm / 2.0), middleSensitiveTube, (format("physicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

        //  std::cout << (format("solidSD_CDCLayer_%1%_middle") % iSLayer).str().c_str() << std::endl;

        if (cdcgp.getMaterialDefinitionMode() == 2) {
          G4String sName = "sWire";
          const G4int ic = 0;
          TVector3 wb = cdcgp.wireBackwardPosition(iSLayer, ic);
          //    G4double rsense0 = wb.Perp();
          TVector3 wf = cdcgp.wireForwardPosition(iSLayer, ic);
          G4double tAtZ0 = -wb.Z() / (wf.Z() - wb.Z()); //t: param. along the wire
          TVector3 wAtZ0 = wb + tAtZ0 * (wf - wb);
          //additional chop of wire; must be larger than 126um*(1/10), where 126um is the field wire diameter; 1/10: approx. stereo angle
          const G4double epsl = 25.e-4; // (in cm);
          G4double reductionBwd = (zback_sensitive_middle + epsl) / wb.Z();
          //chop the wire at zback_sensitive_middle for avoiding overlap; this is because the wire length defined by wb and wf is larger than the length of middle sensitive tube
          wb = reductionBwd * (wb - wAtZ0) + wAtZ0;
          //    std::cout <<"layer,rmin_sensitive_middle,rmax_sensitive_middle,rsense0,rsense=" << iSLayer <<" "<< rmin_sensitive_middle <<" "<< rmax_sensitive_middle <<" "<< rsense0 <<" "<< wb.Perp() << std::endl;
          //chop wire at  zfor_sensitive_middle for avoiding overlap
          G4double reductionFwd = (zfor_sensitive_middle - epsl) / wf.Z();
          wf = reductionFwd * (wf - wAtZ0) + wAtZ0;
          //    std::cout <<"reductionBwd,reductionFwd= " << reductionBwd <<" "<< reductionFwd << std::endl;

          const G4double wireHalfLength = 0.5 * (wf - wb).Mag() * cm;
          const G4double sWireRadius = 0.5 * cdcgp.senseWireDiameter() * cm;
          //    const G4double sWireRadius = 63.e-4 * cm;
          //    std::cout <<"sense w radius (mm)= " << sWireRadius << std::endl;
          //    exit(-1);

          //    const G4double sWireRadius = 15.e-4 * cm;
          G4Tubs* middleSensitiveSwireShape = new G4Tubs(sName, 0., sWireRadius, wireHalfLength, 0., 360. * deg);
          G4LogicalVolume* middleSensitiveSwire = new G4LogicalVolume(middleSensitiveSwireShape, medTungsten, sName);
          //    middleSensitiveSwire->SetSensitiveDetector(m_sensitive);
          middleSensitiveSwire->SetVisAttributes(G4VisAttributes::GetInvisible()); // <- to speed up visualization

          G4String fName = "fWire";
          const G4double fWireRadius  = 0.5 * cdcgp.fieldWireDiameter() * cm;
          G4Tubs* middleSensitiveFwireShape = new G4Tubs(fName, 0., fWireRadius, wireHalfLength, 0., 360. * deg);
          G4LogicalVolume* middleSensitiveFwire = new G4LogicalVolume(middleSensitiveFwireShape, medAluminum, fName);
          //    middleSensitiveFwire->SetSensitiveDetector(m_sensitive);
          middleSensitiveFwire->SetVisAttributes(G4VisAttributes::GetInvisible()); // <- to speed up visualization

          const G4double diameter = cdcgp.fieldWireDiameter();

          const G4int nCells = cdcgp.nWiresInLayer(iSLayer);
          const G4double dphi = M_PI / nCells;
          const TVector3 unitZ(0., 0., 1.);

          for (int ic = 0; ic < nCells; ++ic) {
            //define sense wire
            TVector3 wb = cdcgp.wireBackwardPosition(iSLayer, ic);
            TVector3 wf = cdcgp.wireForwardPosition(iSLayer, ic);
            G4double tAtZ0 = -wb.Z() / (wf.Z() - wb.Z());
            TVector3 wAtZ0 = wb + tAtZ0 * (wf - wb);
            G4double reductionBwd = (zback_sensitive_middle + epsl) / wb.Z();
            wb = reductionBwd * (wb - wAtZ0) + wAtZ0;
            G4double reductionFwd = (zfor_sensitive_middle - epsl) / wf.Z();
            wf = reductionFwd * (wf - wAtZ0) + wAtZ0;

            G4double thetaYZ = -asin((wf - wb).Y() / (wf - wb).Mag());

            TVector3 fMinusBInZX((wf - wb).X(), 0., (wf - wb).Z());
            G4double thetaZX = asin((unitZ.Cross(fMinusBInZX)).Y() / fMinusBInZX.Mag());
            G4RotationMatrix rotM;
            //      std::cout <<"deg,rad= " << deg <<" "<< rad << std::endl;
            rotM.rotateX(thetaYZ * rad);
            rotM.rotateY(thetaZX * rad);

            G4ThreeVector xyz(0.5 * (wb.X() + wf.X()) * cm,
                              0.5 * (wb.Y() + wf.Y()) * cm, 0.);

            //      std::cout <<"0 x,y= " << xyz.getX() <<" "<< xyz.getY() << std::endl;
            //Calling G4PVPlacement with G4Transform3D is convenient because it rotates the object instead of rotating the coordinate-frame; rotM is copied so it does not have to be created on heep by new.
            new G4PVPlacement(G4Transform3D(rotM, xyz), middleSensitiveSwire, sName, middleSensitiveTube, false, ic);

            //define field wire #1 (placed at the same phi but at the outer r boundary)
            TVector3 wbF = wb;
            G4double rF = rmax_sensitive_middle - 0.5 * diameter;
            //      std::cout <<"iSLayer,rF= " << iSLayer <<" "<< rF <<" "<< std::endl;
            G4double phi = atan2(wbF.Y(), wbF.X());
            wbF.SetX(rF * cos(phi));
            wbF.SetY(rF * sin(phi));

            TVector3 wfF = wf;
            rF = rmax_sensitive_middle - 0.5 * diameter;
            phi = atan2(wfF.Y(), wfF.X());
            wfF.SetX(rF * cos(phi));
            wfF.SetY(rF * sin(phi));

            thetaYZ = -asin((wfF - wbF).Y() / (wfF - wbF).Mag());

            fMinusBInZX = wfF - wbF;
            fMinusBInZX.SetY(0.);
            thetaZX = asin((unitZ.Cross(fMinusBInZX)).Y() / fMinusBInZX.Mag());

            G4RotationMatrix rotM1;
            rotM1.rotateX(thetaYZ * rad);
            rotM1.rotateY(thetaZX * rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * cm);

            if (iSLayer != nSLayer - 1) {
              //        std::cout <<"1 x,y= " << xyz.getX() <<" "<< xyz.getY() << std::endl;
              new G4PVPlacement(G4Transform3D(rotM1, xyz), middleSensitiveFwire, fName, middleSensitiveTube, false, ic);
            }

            //define field wire #2 (placed at the same radius but shifted by dphi)
            wbF = wb;
            rF = wbF.Perp();
            phi = atan2(wbF.Y(), wbF.X());
            wbF.SetX(rF * cos(phi + dphi));
            wbF.SetY(rF * sin(phi + dphi));

            wfF = wf;
            rF = wfF.Perp();
            phi = atan2(wfF.Y(), wfF.X());
            wfF.SetX(rF * cos(phi + dphi));
            wfF.SetY(rF * sin(phi + dphi));

            thetaYZ = -asin((wfF - wbF).Y() / (wfF - wbF).Mag());

            fMinusBInZX = wfF - wbF;
            fMinusBInZX.SetY(0.);
            thetaZX = asin((unitZ.Cross(fMinusBInZX)).Y() / fMinusBInZX.Mag());

            G4RotationMatrix rotM2;
            rotM2.rotateX(thetaYZ * rad);
            rotM2.rotateY(thetaZX * rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * cm);

            //      std::cout <<"2 x,y= " << xyz.getX() <<" "<< xyz.getY() << std::endl;
            new G4PVPlacement(G4Transform3D(rotM2, xyz), middleSensitiveFwire, fName, middleSensitiveTube, false, ic + nCells);

            //define field wire #3 (placed at the cell corner)
            wbF = wb;
            rF = rmax_sensitive_middle - 0.5 * diameter;
            phi = atan2(wbF.Y(), wbF.X());
            wbF.SetX(rF * cos(phi + dphi));
            wbF.SetY(rF * sin(phi + dphi));

            wfF = wf;
            rF = rmax_sensitive_middle - 0.5 * diameter;
            phi = atan2(wfF.Y(), wfF.X());
            wfF.SetX(rF * cos(phi + dphi));
            wfF.SetY(rF * sin(phi + dphi));

            thetaYZ = -asin((wfF - wbF).Y() / (wfF - wbF).Mag());

            fMinusBInZX = wfF - wbF;
            fMinusBInZX.SetY(0.);
            thetaZX = asin((unitZ.Cross(fMinusBInZX)).Y() / fMinusBInZX.Mag());

            G4RotationMatrix rotM3;
            rotM3.rotateX(thetaYZ * rad);
            rotM3.rotateY(thetaZX * rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * cm);

            if (iSLayer != nSLayer - 1) {
              //        std::cout <<"3 x,y= " << xyz.getX() <<" "<< xyz.getY() << std::endl;
              new G4PVPlacement(G4Transform3D(rotM3, xyz), middleSensitiveFwire, fName, middleSensitiveTube, false, ic + 2 * nCells);
            }
          }  // end of wire loop
        }  // end of wire definitions

        // Endplates
        const int nEPLayer4Loop = (simplifiedGeometry) ? -1 : nEPLayer;
        for (int iEPLayer = 0; iEPLayer < nEPLayer4Loop; ++iEPLayer) {
          std::ostringstream endplateName;
          endplateName << "solidCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          std::ostringstream endplateName1;
          endplateName1 << "logicalCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          std::ostringstream endplateName2;
          endplateName2 << "physicalCDCLayer_" << iSLayer << "_" << epName[iSLayer][iEPLayer] << "_" << iEPLayer;
          double length_endplate = (epFZ[iSLayer][iEPLayer] - epBZ[iSLayer][iEPLayer]) / 2.0;
          G4Tubs* endplateTubeShape = new G4Tubs(endplateName.str().c_str(), epInnerR[iSLayer][iEPLayer]*cm, epOuterR[iSLayer][iEPLayer]*cm, length_endplate * cm, 0 * deg, 360.*deg);
          G4LogicalVolume* endplateTube = new G4LogicalVolume(endplateTubeShape, medAluminum, endplateName1.str().c_str(), 0, 0);
          endplateTube->SetVisAttributes(G4VisAttributes(G4Colour(1., 1., 0.)));
          // Commented by M. U. June 3rd, 2013

          if (iSLayer != (nSLayer - 1) || (iEPLayer != 2 && iEPLayer != 5))
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (epFZ[iSLayer][iEPLayer] + epBZ[iSLayer][iEPLayer])*cm / 2.0), endplateTube, endplateName2.str().c_str(), logical_cdc, false, iSLayer);
        } //end of iEPLayer loop
      } //end of iSLayer loop

      //--------------------------------------------------
      // Get paprameters and construct electronics boards
      //--------------------------------------------------
      const int nEB = (simplifiedGeometry) ? -1 : content.getNumberNodes("ElectronicsBoards/ElectronicsBoard");
      for (int iEB = 0; iEB < nEB; ++iEB) {
        // Get parameters
        GearDir ebContent(content);
        ebContent.append((format("/ElectronicsBoards/ElectronicsBoard[%1%]/") % (iEB + 1)).str());
        string sebID = ebContent.getString("@id");
        int ebID = atoi(sebID.c_str());
        double ebInnerR = ebContent.getLength("EBInnerR");
        double ebOuterR = ebContent.getLength("EBOuterR");
        double ebBZ = ebContent.getLength("EBBackwardZ");
        double ebFZ = ebContent.getLength("EBForwardZ");

        // Construct electronics boards
        G4Tubs* ebTubeShape = new G4Tubs((format("solidSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), ebInnerR * cm, ebOuterR * cm, (ebFZ - ebBZ)*cm / 2.0, 0 * deg, 360.*deg);

        G4LogicalVolume* ebTube = new G4LogicalVolume(ebTubeShape, medNEMA_G10_Plate, (format("logicalSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), 0, 0, 0);
        //ebTube->SetSensitiveDetector(m_sensitive);
        ebTube->SetSensitiveDetector(new BkgSensitiveDetector("CDC", iEB));
        ebTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 0.)));
        // Commented by M. U. June 3rd, 2013
        //        G4VPhysicalVolume* phyebTube;
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (ebFZ + ebBZ)*cm / 2.0), ebTube, (format("physicalSD_ElectronicsBoard_Layer%1%") % ebID).str().c_str(), logical_cdc, false, ebID);
      }

      //--------------------------------------
      // Get parameters and construct covers
      //--------------------------------------
      const int nCover = (simplifiedGeometry) ? -1 : content.getNumberNodes("Covers/Cover");
      for (int iCover = 0; iCover < nCover; ++iCover) {
        // Get parameters
        GearDir coverContent(content);
        coverContent.append((format("/Covers/Cover[%1%]/") % (iCover + 1)).str());
        string scoverID = coverContent.getString("@id");
        int coverID = atoi(scoverID.c_str());
        string coverName = coverContent.getString("Name");
        double coverInnerR1 = coverContent.getLength("InnerR1");
        double coverInnerR2 = coverContent.getLength("InnerR2");
        double coverThick = coverContent.getLength("Thickness");
        double coverAngle = coverContent.getAngle("Angle");
        double coverPosZ = coverContent.getLength("PosZ");


        if (coverName == "BackwardCover") {
          //          double rmin2 = coverInnerR1;
          //          double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
          double rmin1 = coverInnerR2;
          double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
          //          double coverLength = (rmin1 - rmin2) / std::tan(coverAngle);

          if (coverID == 2) {
            G4Tubs* coverTubeShape = new G4Tubs((format("solidBackwardCover%1%") % coverID).str().c_str(), 28.8 * cm, rmax1 * cm, coverThick * cm / 2.0, 0.*deg, 360.*deg);
            G4LogicalVolume* coverTube = new G4LogicalVolume(coverTubeShape, medAluminum, (format("logicalBackwardCover%1%") % coverID).str().c_str(), 0, 0, 0);
            coverTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 1.)));

            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, coverPosZ * cm - coverThick * cm / 2.0), coverTube, (format("physicalBackwardCover%1%") % coverID).str().c_str(), logical_cdc, false, coverID);
          }
        } else {

          double rmin1 = coverInnerR1;
          double rmax1 = rmin1 + coverThick / std::cos(coverAngle);
          double rmin2 = coverInnerR2;
          double rmax2 = rmin2 + coverThick / std::cos(coverAngle);
          double coverLength = (rmin2 - rmin1) / std::tan(coverAngle);


          // Construct covers
          if (coverID == 4) {
            G4Tubs* coverTubeShape = new G4Tubs((format("solidForwardCover%1%") % coverID).str().c_str(), 43.8 * cm, 113.71 * cm, coverThick * cm / 2.0, 0.*deg, 360.*deg);
            G4LogicalVolume* coverTube = new G4LogicalVolume(coverTubeShape, medAluminum, (format("logicalForwardCover%1%") % coverID).str().c_str(), 0, 0, 0);
            coverTube->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 1.)));

            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 165.519 * cm - coverThick * cm / 2.0), coverTube, (format("physicalForwardCover%1%") % coverID).str().c_str(), logical_cdc, false, coverID);

          } else {
            G4Cons* coverConeShape = new G4Cons((format("solidForwardCover%1%") % coverID).str().c_str(), rmin1 * cm, rmax1 * cm, rmin2 * cm, rmax2 * cm, coverLength * cm / 2.0, 0.*deg, 360.*deg);
            G4LogicalVolume* coverCone = new G4LogicalVolume(coverConeShape, medAluminum, (format("logicalForwardCover%1%") % coverID).str().c_str(), 0, 0, 0);
            coverCone->SetVisAttributes(G4VisAttributes(G4Colour(0., 1., 1.)));
            new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (coverPosZ + coverLength / 2.0)*cm), coverCone, (format("physicalForwardCover%1%") % coverID).str().c_str(), logical_cdc, false, coverID);
          }
        }
      }




      const bool nshieldflag = (simplifiedGeometry) ? false : true;
      if (nshieldflag) {
        //
        // B4C for shilding material of neutron
        // 2012.4.22   M. Uchida
        //
        //
        // double AA = 10.811*g/mole
        // Atomic mass for B (natural abanduns)

        G4NistManager* man = G4NistManager::Instance();
        G4Material* C2H4 = man->FindOrBuildMaterial("G4_POLYETHYLENE");
        G4Material* elB   = man->FindOrBuildMaterial("G4_B");
        // G4Material* B4C = man->FindOrBuildMaterial("G4_BORON_CARBIDE");

        // 5% borated polyethylene = SWX201
        // http://www.deqtech.com/Shieldwerx/Products/swx201hd.htm
        G4Material* boratedpoly05 = new G4Material("BoratedPoly05", 1.06 * g / cm3, 2);
        boratedpoly05->AddMaterial(elB, 0.05);
        boratedpoly05->AddMaterial(C2H4, 0.95);
        // 30% borated polyethylene = SWX210
        G4Material* boratedpoly30 = new G4Material("BoratedPoly30", 1.19 * g / cm3, 2);
        boratedpoly30->AddMaterial(elB, 0.30);
        boratedpoly30->AddMaterial(C2H4, 0.70);

        G4Material* shieldMat = C2H4;


        /*
              double AA = 10.01 * g / mole ; // Atomic mass for 10B  90%
              double ZZ = 5.0;
              G4Element* elB = new G4Element("Boron", "B", ZZ, AA);

              AA = 12.011 * g / mole; // Atomic mass for C
              ZZ = 6.0;
              G4Element* elC = new G4Element("Carbon", "C", ZZ, AA);

              const G4double denB4C = 1.25 * g / cm3; // density of B4C with silicon
              // Notice: B4C density itself is 2.51 g/cm3
              // B4C : Silicon  = 1 : 1 -> density = 2.51x0.5

              G4Material* B4C = new G4Material("B4C", denB4C, 2);
              B4C->AddElement(elB, 4);
              B4C->AddElement(elC, 1);
        G4Material* shieldMat = B4C;
        */

        G4Tubs* bgShieldTubeShape_0 = new G4Tubs("solid_B4C_0", (20.25 + 8.5) * cm, (20.25 + 8.5 + 30.0) * cm, 3.0 * cm / 2.0, 0 * deg, 360.*deg);
        G4LogicalVolume* bgShieldTube_0 = new G4LogicalVolume(bgShieldTubeShape_0, shieldMat, "logical_B4C_0", 0, 0, 0);
        bgShieldTube_0->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (-98.5 + 1.5)*cm), bgShieldTube_0, "physical_B4C_0", logical_cdc, false, 0);

        G4Tubs* bgShieldTubeShape_1 = new G4Tubs("solid_B4C_1", (20.25 + 8.5) * cm, (20.25 + 8.5 + 15.0) * cm, 7.0 * cm / 2.0, 0 * deg, 360.*deg);
        G4LogicalVolume* bgShieldTube_1 = new G4LogicalVolume(bgShieldTubeShape_1, shieldMat, "logical_B4C_1", 0, 0, 0);
        bgShieldTube_1->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (-98.5 + 3.0 + 3.5)*cm), bgShieldTube_1, "physical_B4C_1", logical_cdc, false, 0);
        //increase length of physical_B4C_1 from 5->7, to protect iner EB layers.

        G4Tubs* bgShieldTubeShape_2 = new G4Tubs("solid_B4C_2", (20.25 + 8.5) * cm, (20.25 + 8.5 + 3.0) * cm, 8.5 * cm / 2.0, 0 * deg, 360.*deg);
        G4LogicalVolume* bgShieldTube_2 = new G4LogicalVolume(bgShieldTubeShape_2, shieldMat, "logical_B4C_2", 0, 0, 0);
        bgShieldTube_2->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, -84.25 * cm), bgShieldTube_2, "physical_B4C_2", logical_cdc, false, 0);

        G4Tubs* bgShieldTubeShape_3 = new G4Tubs("solid_B4C_3", (20.25 + 5.75) * cm, (20.25 + 5.75 + 3.0) * cm, 11.5 * cm / 2.0, 0 * deg, 360.*deg);
        G4LogicalVolume* bgShieldTube_3 = new G4LogicalVolume(bgShieldTubeShape_3, shieldMat, "logical_B4C_3", 0, 0, 0);
        bgShieldTube_3->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, -61.250 * cm), bgShieldTube_3, "physical_B4C_3", logical_cdc, false, 0);


        G4Cons* bgShieldConeShape_4 = new G4Cons("solid_B4C_4", (20.25 + 8.5) * cm, (20.25 + 8.5 + 3.0) * cm, (20.25 + 5.75)* cm, (20.25 + 5.75 + 3) * cm,  13.0 * cm / 2.0, 0.*deg, 360.*deg);
        G4LogicalVolume* bgShieldCone_4 = new G4LogicalVolume(bgShieldConeShape_4, shieldMat, "logical_B4c_4", 0, 0, 0);
        bgShieldCone_4->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, -73.5 * cm), bgShieldCone_4, "physicalbgShield_B4C_4", logical_cdc, false, 0);
        // Thanh edited physicalbgshield_B4C_4.

        G4Cons* bgShieldConeShape_5 = new G4Cons("solid_B4C_5", (20.25 + 5.25) * cm, (20.25 + 5.25 + 3.0) * cm, (20.25 - 8.0 + 4.5)* cm, (20.25 - 8.0 + 4.5 + 3.0) * cm,  15.0 * cm / 2.0, 0.*deg, 360.*deg);
        G4LogicalVolume* bgShieldCone_5 = new G4LogicalVolume(bgShieldConeShape_5, shieldMat, "logical_B4C_5", 0, 0, 0);
        bgShieldCone_5->SetVisAttributes(G4VisAttributes(G4Colour(0., 0., 1.)));
        // Commented by M. U. June 3rd, 2013
        //        G4VPhysicalVolume* physbgShieldCone_5;
        // new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, -48.0 * cm), bgShieldCone_5, "physicalbgShield_B4C_5", logical_cdc, false, 0);

      }


      delete [] nEndplateLayer;
      delete [] flayerRadius;
      delete [] flayerZBack;
      delete [] flayerZFor;

      delete [] slayerRadius;
      delete [] slayerZBack;
      delete [] slayerZFor;

      delete [] outerWallInnerR;
      delete [] outerWallOuterR;
      delete [] outerWallBZ;
      delete [] outerWallFZ;
      delete [] innerWallInnerR;
      delete [] innerWallOuterR;
      delete [] innerWallBZ;
      delete [] innerWallFZ;

      //      G4cout << *(G4Material::GetMaterialTable());
    }

  }

}
