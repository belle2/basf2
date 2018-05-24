/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida, Eiichi Nakano                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/geometry/GeoCDCCreator.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeoControlPar.h>
#include <cdc/simulation/CDCSimControlPar.h>
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
#include <G4Torus.hh>
#include <G4Trd.hh>
#include <G4SubtractionSolid.hh>
#include <G4PVReplica.hh>
#include <G4VSolid.hh>

#include <G4Polycone.hh>
#include <G4Cons.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>
#include <G4RotationMatrix.hh>
#include <G4UserLimits.hh>
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
      //      std::cout << "GeoCDCCreator constructor called" << std::endl;
      // Set job control params. before sensitivedetector and gometry construction
      CDCSimControlPar::getInstance();
      CDCGeoControlPar::getInstance();

      m_sensitive = new CDCSensitiveDetector("CDCSensitiveDetector", (2 * 24)* CLHEP::eV, 10 * CLHEP::MeV);
      m_bkgsensitive = NULL;
      logical_cdc = 0;
      physical_cdc = 0;
      m_VisAttributes.clear();
      m_VisAttributes.push_back(new G4VisAttributes(false)); // for "invisible"
      m_userLimits.clear();
    }


    GeoCDCCreator::~GeoCDCCreator()
    {
      delete m_sensitive;
      if (m_bkgsensitive) delete m_bkgsensitive;
      for (G4VisAttributes* visAttr : m_VisAttributes) delete visAttr;
      m_VisAttributes.clear();
      for (G4UserLimits* userLimits : m_userLimits) delete userLimits;
      m_userLimits.clear();
    }

    void GeoCDCCreator::createGeometry(const CDCGeometry& geo, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      //      std::cout << "createGeometry called" << std::endl;
      const G4double realTemperture = (273.15 + 23.) * CLHEP::kelvin;
      G4Material* medHelium = geometry::Materials::get("CDCHeGas");
      G4Material* medEthane = geometry::Materials::get("CDCEthaneGas");
      G4Material* medAluminum = geometry::Materials::get("Al");
      G4Material* medTungsten = geometry::Materials::get("W");
      G4Material* medCFRP = geometry::Materials::get("CFRP");
      G4Material* medNEMA_G10_Plate = geometry::Materials::get("NEMA_G10_Plate");
      G4Material* medGlue = geometry::Materials::get("CDCGlue");
      G4Material* medAir = geometry::Materials::get("Air");

      // Nakano
      G4double h2odensity = 1.000 * CLHEP::g / CLHEP::cm3;
      G4double a = 1.01 * CLHEP::g / CLHEP::mole;
      G4Element* elH = new G4Element("Hydrogen", "H", 1., a);
      a = 16.00 * CLHEP::g / CLHEP::mole;
      G4Element* elO = new G4Element("Oxygen", "O", 8., a);
      G4Material* medH2O = new G4Material("Water", h2odensity, 2);
      medH2O->AddElement(elH, 2);
      medH2O->AddElement(elO, 1);
      G4Material* medCopper = geometry::Materials::get("Cu");
      G4Material* medHV = geometry::Materials::get("CDCHVCable");
      //G4Material* medFiber = geometry::Materials::get("CDCOpticalFiber");
      //G4Material* medCAT7 = geometry::Materials::get("CDCCAT7");
      //G4Material* medTRG = geometry::Materials::get("CDCOpticalFiberTRG");

      // Total cross section
      const double rmax_innerWall = geo.getFiducialRmin();
      const double rmin_outerWall = geo.getFiducialRmax();
      const double diameter_senseWire = geo.getSenseDiameter();
      const double diameter_fieldWire = geo.getFieldDiameter();
      const double num_senseWire = static_cast<double>(geo.getNSenseWires());
      const double num_fieldWire = static_cast<double>(geo.getNFieldWires());
      double totalCS = M_PI * (rmin_outerWall * rmin_outerWall - rmax_innerWall * rmax_innerWall);

      // Sense wire cross section
      double senseCS = M_PI * (diameter_senseWire / 2) * (diameter_senseWire / 2) * num_senseWire;

      // Field wire cross section
      double fieldCS = M_PI * (diameter_fieldWire / 2) * (diameter_fieldWire / 2) * num_fieldWire;

      // Density
      const double denHelium = medHelium->GetDensity() / 2.0;
      const double denEthane = medEthane->GetDensity() / 2.0;
      const double denAluminum = medAluminum->GetDensity() * (fieldCS / totalCS);
      const double denTungsten = medTungsten->GetDensity() * (senseCS / totalCS);
      const double density = denHelium + denEthane + denAluminum + denTungsten;
      G4Material* cdcMed = new G4Material("CDCGasWire", density, 4, kStateGas, realTemperture);
      cdcMed->AddMaterial(medHelium, denHelium / density);
      cdcMed->AddMaterial(medEthane, denEthane / density);
      cdcMed->AddMaterial(medTungsten, denTungsten / density);
      cdcMed->AddMaterial(medAluminum, denAluminum / density);

      G4Material* cdcMedGas = cdcMed;

      CDCGeometryPar& cdcgp = CDCGeometryPar::Instance(&geo);
      CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();
      //      std::cout << gcp.getMaterialDefinitionMode() << std::endl;

      //      if (cdcgp.getMaterialDefinitionMode() == 2) {
      if (gcp.getMaterialDefinitionMode() == 2) {
        const double density2 = denHelium + denEthane;
        cdcMedGas = new G4Material("CDCRealGas", density2, 2, kStateGas, realTemperture);
        cdcMedGas->AddMaterial(medHelium, denHelium / density2);
        cdcMedGas->AddMaterial(medEthane, denEthane / density2);
      }

      if (gcp.getPrintMaterialTable()) {
        G4cout << *(G4Material::GetMaterialTable());
      }

      const auto& mother = geo.getMotherVolume();
      const auto& motherRmin = mother.getRmin();
      const auto& motherRmax = mother.getRmax();
      const auto& motherZ = mother.getZ();
      G4Polycone* solid_cdc =
        new G4Polycone("solidCDC", 0 * CLHEP::deg, 360.* CLHEP::deg,
                       mother.getNNodes(), motherZ.data(),
                       motherRmin.data(), motherRmax.data());
      logical_cdc = new G4LogicalVolume(solid_cdc, medAir, "logicalCDC", 0, 0, 0);
      physical_cdc = new G4PVPlacement(0, G4ThreeVector(geo.getGlobalOffsetX() * CLHEP::cm, geo.getGlobalOffsetY() * CLHEP::cm,
                                                        geo.getGlobalOffsetZ() * CLHEP::cm), logical_cdc, "physicalCDC",
                                       &topVolume, false,
                                       0);


      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      for (const auto& wall : geo.getOuterWalls()) {
        const int iOuterWall = wall.getId();
        const string wallName = wall.getName();
        const double wallRmin = wall.getRmin();
        const double wallRmax = wall.getRmax();
        const double wallZfwd = wall.getZfwd();
        const double wallZbwd = wall.getZbwd();
        const double length = (wallZfwd - wallZbwd) / 2.0;


        G4Material* medWall = medAir;
        if (strstr((wallName).c_str(), "MiddleWall") != NULL) {
          medWall = medCFRP;
        } else {
          medWall = medAluminum;
        }
        G4Tubs* outerWallTubeShape = new G4Tubs("solid" + wallName, wallRmin * CLHEP::cm,
                                                wallRmax * CLHEP::cm, length * CLHEP::cm, 0 * CLHEP::deg, 360.*CLHEP::deg);

        G4LogicalVolume* outerWallTube = new G4LogicalVolume(outerWallTubeShape, medWall, "solid" + wallName, 0, 0, 0);
        outerWallTube->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + wallZbwd)*CLHEP::cm), outerWallTube, "logical" + wallName,
                          logical_cdc, false, iOuterWall);
      }


      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      for (const auto& wall : geo.getInnerWalls()) {
        const string wallName = wall.getName();
        const double wallRmin = wall.getRmin();
        const double wallRmax = wall.getRmax();
        const double wallZfwd = wall.getZfwd();
        const double wallZbwd = wall.getZbwd();
        const double length = (wallZfwd - wallZbwd) / 2.0;
        const int iInnerWall = wall.getId();

        G4Material* medWall = medAir;
        if (strstr(wallName.c_str(), "MiddleWall") != NULL) {
          medWall = medCFRP;
        } else if (strstr(wallName.c_str(), "MiddleGlue") != NULL) { // Glue layer 0.005 mmt
          medWall = medGlue;
        } else { // Al layer 0.1 mmt
          medWall = medAluminum;
        }

        G4Tubs* innerWallTubeShape = new G4Tubs("solid" + wallName, wallRmin * CLHEP::cm,
                                                wallRmax * CLHEP::cm, length * CLHEP::cm, 0 * CLHEP::deg, 360.*CLHEP::deg);
        G4LogicalVolume* innerWallTube = new G4LogicalVolume(innerWallTubeShape, medWall, "logical" + wallName, 0, 0, 0);
        innerWallTube->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length + wallZbwd)*CLHEP::cm), innerWallTube, "physical" + wallName,
                          logical_cdc, false, iInnerWall);


      }



      //
      // Construct sensitive layers.
      //
      const int nSLayer = geo.getNSenseLayers();
      const double length_feedthrough  = geo.getFeedthroughLength();
      for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
        const auto& endplate = geo.getEndPlate(iSLayer);
        const int nEPLayer = endplate.getNEndPlateLayers();
        // Get parameters for sensitive layer: left, middle and right.
        double rmin_sensitive_left, rmax_sensitive_left;
        double rmin_sensitive_middle, rmax_sensitive_middle;
        double rmin_sensitive_right, rmax_sensitive_right;
        double zback_sensitive_left, zfor_sensitive_left;
        double zback_sensitive_middle, zfor_sensitive_middle;
        double zback_sensitive_right, zfor_sensitive_right;

        if (iSLayer == 0) {
          const auto& epLayerBwd = endplate.getEndPlateLayer(0);
          const auto& epLayerFwd = endplate.getEndPlateLayer(nEPLayer / 2);
          const auto& senseLayer = geo.getSenseLayer(iSLayer);
          const auto& fieldLayer = geo.getFieldLayer(iSLayer);

          rmin_sensitive_left = epLayerBwd.getRmax();
          rmax_sensitive_left = fieldLayer.getR();
          zback_sensitive_left = senseLayer.getZbwd();
          zfor_sensitive_left = epLayerBwd.getZfwd();

          rmin_sensitive_middle = (geo.getInnerWall(0)).getRmax();
          rmax_sensitive_middle = fieldLayer.getR();
          zback_sensitive_middle = epLayerBwd.getZbwd();
          zfor_sensitive_middle = epLayerFwd.getZbwd();

          rmin_sensitive_right = epLayerFwd.getRmax();
          rmax_sensitive_right = fieldLayer.getR();
          zback_sensitive_right = epLayerFwd.getZbwd();
          zfor_sensitive_right = senseLayer.getZfwd();
        } else if (iSLayer >= 1 && iSLayer <= 14) {
          const auto& epLayerBwd = endplate.getEndPlateLayer(1);
          const auto& epLayerFwd = endplate.getEndPlateLayer((nEPLayer / 2) + 1);
          const auto& senseLayer = geo.getSenseLayer(iSLayer);
          const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
          const auto& fieldLayerOut = geo.getFieldLayer(iSLayer);

          rmin_sensitive_left = epLayerBwd.getRmax();
          rmax_sensitive_left = fieldLayerOut.getR();
          zback_sensitive_left = senseLayer.getZbwd();
          zfor_sensitive_left = epLayerBwd.getZfwd();

          rmin_sensitive_middle = fieldLayerIn.getR();
          rmax_sensitive_middle = fieldLayerOut.getR();
          zback_sensitive_middle = epLayerBwd.getZfwd();
          zfor_sensitive_middle = epLayerFwd.getZbwd();

          rmin_sensitive_right = epLayerFwd.getRmax();
          rmax_sensitive_right = fieldLayerOut.getR();
          zback_sensitive_right = epLayerFwd.getZbwd();
          zfor_sensitive_right = senseLayer.getZfwd();
        } else if (iSLayer >= 15 && iSLayer <= 18) {
          const auto& epLayerBwd = endplate.getEndPlateLayer(1);
          const auto& epLayerFwd = endplate.getEndPlateLayer(nEPLayer / 2);
          const auto& senseLayer = geo.getSenseLayer(iSLayer);
          const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
          const auto& fieldLayerOut = geo.getFieldLayer(iSLayer);

          rmin_sensitive_left = epLayerBwd.getRmax();
          rmax_sensitive_left = fieldLayerOut.getR();
          zback_sensitive_left = senseLayer.getZbwd();
          zfor_sensitive_left = epLayerBwd.getZfwd();

          rmin_sensitive_middle = fieldLayerIn.getR();
          rmax_sensitive_middle = fieldLayerOut.getR();
          zback_sensitive_middle = epLayerBwd.getZfwd();
          zfor_sensitive_middle = epLayerFwd.getZbwd();

          rmin_sensitive_right = epLayerFwd.getRmax();
          rmax_sensitive_right = fieldLayerOut.getR();
          zback_sensitive_right = epLayerFwd.getZbwd();
          zfor_sensitive_right = senseLayer.getZfwd();
        } else if (iSLayer >= 19 && iSLayer < 55) {
          const auto& epLayerBwd = endplate.getEndPlateLayer(0);
          const auto& epLayerFwd = endplate.getEndPlateLayer(nEPLayer / 2);
          const auto& senseLayer = geo.getSenseLayer(iSLayer);
          const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
          const auto& fieldLayerOut = geo.getFieldLayer(iSLayer);

          rmin_sensitive_left = epLayerBwd.getRmax();
          rmax_sensitive_left = fieldLayerOut.getR();
          zback_sensitive_left = senseLayer.getZbwd();
          zfor_sensitive_left = epLayerBwd.getZfwd();

          rmin_sensitive_middle = fieldLayerIn.getR();
          rmax_sensitive_middle = fieldLayerOut.getR();
          zback_sensitive_middle = epLayerBwd.getZfwd();
          zfor_sensitive_middle = epLayerFwd.getZbwd();

          rmin_sensitive_right = epLayerFwd.getRmax();
          rmax_sensitive_right = fieldLayerOut.getR();
          zback_sensitive_right = epLayerFwd.getZbwd();
          zfor_sensitive_right = senseLayer.getZfwd();

        } else if (iSLayer == 55) {

          const auto& epLayerBwdIn = endplate.getEndPlateLayer(0);
          const auto& epLayerBwdOut = endplate.getEndPlateLayer((nEPLayer / 2) - 1);
          const auto& epLayerFwdIn = endplate.getEndPlateLayer(nEPLayer / 2);
          const auto& epLayerFwdOut = endplate.getEndPlateLayer(nEPLayer - 1);
          const auto& senseLayer = geo.getSenseLayer(iSLayer);
          const auto& fieldLayerIn = geo.getFieldLayer(iSLayer - 1);
          rmin_sensitive_left = epLayerBwdIn.getRmax();
          rmax_sensitive_left = epLayerBwdOut.getRmax();
          zback_sensitive_left = senseLayer.getZbwd();
          zfor_sensitive_left = epLayerBwdIn.getZfwd();

          rmin_sensitive_middle = fieldLayerIn.getR();
          rmax_sensitive_middle = (geo.getOuterWall(0)).getRmin();
          zback_sensitive_middle = epLayerBwdIn.getZfwd();
          zfor_sensitive_middle = epLayerFwdIn.getZbwd();

          rmin_sensitive_right = epLayerFwdIn.getRmax();
          rmax_sensitive_right = epLayerFwdOut.getRmax();
          zback_sensitive_right = epLayerFwdIn.getZbwd();
          zfor_sensitive_right = senseLayer.getZfwd();

        } else {
          B2ERROR("Undefined sensitive layer : " << iSLayer);
          continue;
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
          // cppcheck-suppress zerodiv
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left * CLHEP::cm,
                                             rmax_sensitive_left * CLHEP::cm, length_feedthrough * CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed,
                                                          // cppcheck-suppress zerodiv
                                                          (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_left + length_feedthrough / 2.0)*CLHEP::cm), leftTube,
                            // cppcheck-suppress zerodiv
                            (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
          // Build left sensitive tube (area 2)
          // cppcheck-suppress zerodiv
          G4Tubs* leftSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_left") % iSLayer).str().c_str(),
                                                      rmin_sensitive_left * CLHEP::cm, rmax_sensitive_left * CLHEP::cm,
                                                      (zfor_sensitive_left - zback_sensitive_left - length_feedthrough)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* leftSensitiveTube = new G4LogicalVolume(leftSensitiveTubeShape, cdcMed,
                                                                   // cppcheck-suppress zerodiv
                                                                   (format("logicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), 0, 0, 0);
          leftSensitiveTube->SetSensitiveDetector(m_sensitive);
          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left + length_feedthrough)*CLHEP::cm / 2.0),
                            // cppcheck-suppress zerodiv
                            leftSensitiveTube, (format("physicalSD_CDCLayer_%1%_left") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);
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
          // cppcheck-suppress zerodiv
          G4Tubs* leftTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), rmin_sensitive_left * CLHEP::cm,
                                             rmax_sensitive_left * CLHEP::cm, (zfor_sensitive_left - zback_sensitive_left)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* leftTube = new G4LogicalVolume(leftTubeShape, cdcMed,
                                                          // cppcheck-suppress zerodiv
                                                          (format("logicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), 0, 0, 0);
          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_left + zback_sensitive_left)*CLHEP::cm / 2.0), leftTube,
                            // cppcheck-suppress zerodiv
                            (format("physicalCDCLayer_%1%_leftTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);


          // Build a tube with metarial cdcMed for area 2
          // cppcheck-suppress zerodiv
          G4Tubs* leftMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(),
                                                rmin_sensitive_middle * CLHEP::cm, rmax_sensitive_middle * CLHEP::cm,
                                                (length_feedthrough - zfor_sensitive_left + zback_sensitive_left)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* leftMidTube = new G4LogicalVolume(leftMidTubeShape, cdcMed,
                                                             // cppcheck-suppress zerodiv
                                                             (format("logicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), 0, 0, 0);

          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (length_feedthrough + zfor_sensitive_left + zback_sensitive_left)*CLHEP::cm / 2.0),
                            // cppcheck-suppress zerodiv
                            leftMidTube, (format("physicalCDCLayer_%1%_leftMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

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
          // cppcheck-suppress zerodiv
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(),
                                              rmin_sensitive_right * CLHEP::cm, rmax_sensitive_right * CLHEP::cm, length_feedthrough * CLHEP::cm / 2.0, 0 * CLHEP::deg,
                                              360.*CLHEP::deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed,
                                                           // cppcheck-suppress zerodiv
                                                           (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);

          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right - length_feedthrough / 2.0)*CLHEP::cm), rightTube,
                            // cppcheck-suppress zerodiv
                            (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);


          // Build right sensitive tube (area 2)
          // cppcheck-suppress zerodiv
          G4Tubs* rightSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_right") % iSLayer).str().c_str(),
                                                       rmin_sensitive_right * CLHEP::cm, rmax_sensitive_right * CLHEP::cm,
                                                       (zfor_sensitive_right - zback_sensitive_right - length_feedthrough)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* rightSensitiveTube = new G4LogicalVolume(rightSensitiveTubeShape, cdcMed,
                                                                    // cppcheck-suppress zerodiv
                                                                    (format("logicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), 0, 0, 0);
          rightSensitiveTube->SetSensitiveDetector(m_sensitive);

          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right - length_feedthrough)*CLHEP::cm / 2.0),
                            // cppcheck-suppress zerodiv
                            rightSensitiveTube, (format("physicalSD_CDCLayer_%1%_right") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

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
          // cppcheck-suppress zerodiv
          G4Tubs* rightTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightTube") % iSLayer).str().c_str(),
                                              rmin_sensitive_right * CLHEP::cm, rmax_sensitive_right * CLHEP::cm, (zfor_sensitive_right - zback_sensitive_right)*CLHEP::cm / 2.0,
                                              0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* rightTube = new G4LogicalVolume(rightTubeShape, cdcMed,
                                                           // cppcheck-suppress zerodiv
                                                           (format("logicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), 0, 0, 0);

          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_right + zback_sensitive_right)*CLHEP::cm / 2.0), rightTube,
                            // cppcheck-suppress zerodiv
                            (format("physicalCDCLayer_%1%_rightTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);


          // Build a tube with metarial cdcMed for area 2
          // cppcheck-suppress zerodiv
          G4Tubs* rightMidTubeShape = new G4Tubs((format("solidCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(),
                                                 rmin_sensitive_middle * CLHEP::cm, rmax_sensitive_middle * CLHEP::cm,
                                                 (length_feedthrough - zfor_sensitive_right + zback_sensitive_right)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* rightMidTube = new G4LogicalVolume(rightMidTubeShape, cdcMed,
                                                              // cppcheck-suppress zerodiv
                                                              (format("logicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), 0, 0, 0);
          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zback_sensitive_right - length_feedthrough + zfor_sensitive_right)*CLHEP::cm / 2.0),
                            // cppcheck-suppress zerodiv
                            rightMidTube, (format("physicalCDCLayer_%1%_rightMidTube") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

          // Reset zback_sensitive_middle
          zfor_sensitive_middle = zfor_sensitive_right - length_feedthrough;
        }


        // Middle sensitive tube
        // cppcheck-suppress zerodiv
        G4Tubs* middleSensitiveTubeShape = new G4Tubs((format("solidSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(),
                                                      rmin_sensitive_middle * CLHEP::cm, rmax_sensitive_middle * CLHEP::cm,
                                                      (zfor_sensitive_middle - zback_sensitive_middle)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);
        G4LogicalVolume* middleSensitiveTube = new G4LogicalVolume(middleSensitiveTubeShape, cdcMedGas,
                                                                   // cppcheck-suppress zerodiv
                                                                   (format("logicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), 0, 0, 0);
        //hard-coded temporarily
        //need to create an object per layer ??? to be checked later
        G4UserLimits* uLimits = new G4UserLimits(8.5 * CLHEP::cm);
        m_userLimits.push_back(uLimits);
        middleSensitiveTube->SetUserLimits(uLimits);
        middleSensitiveTube->SetSensitiveDetector(m_sensitive);

        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfor_sensitive_middle + zback_sensitive_middle)*CLHEP::cm / 2.0), middleSensitiveTube,
                          (format("physicalSD_CDCLayer_%1%_middle") % iSLayer).str().c_str(), logical_cdc, false, iSLayer);

        //        if (cdcgp.getMaterialDefinitionMode() == 2) {
        if (gcp.getMaterialDefinitionMode() == 2) {
          G4String sName = "sWire";
          const G4int jc = 0;
          TVector3 wb0 = cdcgp.wireBackwardPosition(iSLayer, jc);
          //    G4double rsense0 = wb0.Perp();
          TVector3 wf0 = cdcgp.wireForwardPosition(iSLayer, jc);
          G4double tAtZ0 = -wb0.Z() / (wf0.Z() - wb0.Z()); //t: param. along the wire
          TVector3 wAtZ0 = wb0 + tAtZ0 * (wf0 - wb0);
          //additional chop of wire; must be larger than 126um*(1/10), where 126um is the field wire diameter; 1/10: approx. stereo angle
          const G4double epsl = 25.e-4; // (in cm);
          G4double reductionBwd = (zback_sensitive_middle + epsl) / wb0.Z();
          //chop the wire at zback_sensitive_middle for avoiding overlap; this is because the wire length defined by wb0 and wf0 is larger than the length of middle sensitive tube
          wb0 = reductionBwd * (wb0 - wAtZ0) + wAtZ0;
          //chop wire at  zfor_sensitive_middle for avoiding overlap
          G4double reductionFwd = (zfor_sensitive_middle - epsl) / wf0.Z();
          wf0 = reductionFwd * (wf0 - wAtZ0) + wAtZ0;

          const G4double wireHalfLength = 0.5 * (wf0 - wb0).Mag() * CLHEP::cm;
          const G4double sWireRadius = 0.5 * cdcgp.senseWireDiameter() * CLHEP::cm;
          //    const G4double sWireRadius = 15.e-4 * CLHEP::cm;
          G4Tubs* middleSensitiveSwireShape = new G4Tubs(sName, 0., sWireRadius, wireHalfLength, 0., 360. * CLHEP::deg);
          G4LogicalVolume* middleSensitiveSwire = new G4LogicalVolume(middleSensitiveSwireShape, medTungsten, sName);
          //    middleSensitiveSwire->SetSensitiveDetector(m_sensitive);
          middleSensitiveSwire->SetVisAttributes(m_VisAttributes.front()); // <- to speed up visualization

          G4String fName = "fWire";
          const G4double fWireRadius  = 0.5 * cdcgp.fieldWireDiameter() * CLHEP::cm;
          G4Tubs* middleSensitiveFwireShape = new G4Tubs(fName, 0., fWireRadius, wireHalfLength, 0., 360. * CLHEP::deg);
          G4LogicalVolume* middleSensitiveFwire = new G4LogicalVolume(middleSensitiveFwireShape, medAluminum, fName);
          //    middleSensitiveFwire->SetSensitiveDetector(m_sensitive);
          middleSensitiveFwire->SetVisAttributes(m_VisAttributes.front()); // <- to speed up visualization

          const G4double diameter = cdcgp.fieldWireDiameter();

          const G4int nCells = cdcgp.nWiresInLayer(iSLayer);
          const G4double dphi = M_PI / nCells;
          const TVector3 unitZ(0., 0., 1.);

          for (int ic = 0; ic < nCells; ++ic) {
            //define sense wire
            TVector3 wb = cdcgp.wireBackwardPosition(iSLayer, ic);
            TVector3 wf = cdcgp.wireForwardPosition(iSLayer, ic);
            G4double tAtZ02 = -wb.Z() / (wf.Z() - wb.Z());
            TVector3 wAtZ02 = wb + tAtZ02 * (wf - wb);
            G4double reductionBwd2 = (zback_sensitive_middle + epsl) / wb.Z();
            wb = reductionBwd2 * (wb - wAtZ02) + wAtZ02;
            G4double reductionFwd2 = (zfor_sensitive_middle - epsl) / wf.Z();
            wf = reductionFwd2 * (wf - wAtZ02) + wAtZ02;

            G4double thetaYZ = -asin((wf - wb).Y() / (wf - wb).Mag());

            TVector3 fMinusBInZX((wf - wb).X(), 0., (wf - wb).Z());
            G4double thetaZX = asin((unitZ.Cross(fMinusBInZX)).Y() / fMinusBInZX.Mag());
            G4RotationMatrix rotM;
            //      std::cout <<"deg,rad= " << CLHEP::deg <<" "<< CLHEP::rad << std::endl;
            rotM.rotateX(thetaYZ * CLHEP::rad);
            rotM.rotateY(thetaZX * CLHEP::rad);

            G4ThreeVector xyz(0.5 * (wb.X() + wf.X()) * CLHEP::cm,
                              0.5 * (wb.Y() + wf.Y()) * CLHEP::cm, 0.);

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
            rotM1.rotateX(thetaYZ * CLHEP::rad);
            rotM1.rotateY(thetaZX * CLHEP::rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * CLHEP::cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * CLHEP::cm);

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
            rotM2.rotateX(thetaYZ * CLHEP::rad);
            rotM2.rotateY(thetaZX * CLHEP::rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * CLHEP::cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * CLHEP::cm);

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
            rotM3.rotateX(thetaYZ * CLHEP::rad);
            rotM3.rotateY(thetaZX * CLHEP::rad);

            xyz.setX(0.5 * (wbF.X() + wfF.X()) * CLHEP::cm);
            xyz.setY(0.5 * (wbF.Y() + wfF.Y()) * CLHEP::cm);

            if (iSLayer != nSLayer - 1) {
              new G4PVPlacement(G4Transform3D(rotM3, xyz), middleSensitiveFwire, fName, middleSensitiveTube, false, ic + 2 * nCells);
            }
          }  // end of wire loop
        }  // end of wire definitions

      }
      //
      // Endplates.
      //

      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(1., 1., 0.)));
      for (const auto& endplate : geo.getEndPlates()) {
        for (const auto& epLayer : endplate.getEndPlateLayers()) {
          const int iEPLayer = epLayer.getILayer();
          const string name = epLayer.getName();
          const double rmin = epLayer.getRmin();
          const double rmax = epLayer.getRmax();
          const double zbwd = epLayer.getZbwd();
          const double zfwd = epLayer.getZfwd();
          const double length = (zfwd - zbwd) / 2.0;

          G4Tubs* tube = new G4Tubs("solidCDCEndplate" + name, rmin * CLHEP::cm,
                                    rmax * CLHEP::cm, length * CLHEP::cm, 0 * CLHEP::deg, 360.*CLHEP::deg);
          G4LogicalVolume* logical = new G4LogicalVolume(tube, Materials::get("G4_Al"),
                                                         "logicalCDCEndplate" + name, 0, 0);
          logical->SetVisAttributes(m_VisAttributes.back());
          new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (zfwd + zbwd)*CLHEP::cm / 2.0), logical,
                            "physicalCDCEndplate" + name, logical_cdc, false, iEPLayer);

        }
      }


      // Construct electronics boards
      for (const auto& frontend : geo.getFrontends()) {
        const int iEB = frontend.getId();
        const double ebInnerR = frontend.getRmin();
        const double ebOuterR = frontend.getRmax();
        const double ebBZ = frontend.getZbwd();
        const double ebFZ = frontend.getZfwd();

        G4Tubs* ebTubeShape = new G4Tubs((format("solidSD_ElectronicsBoard_Layer%1%") % iEB).str().c_str(), ebInnerR * CLHEP::cm,
                                         ebOuterR * CLHEP::cm, (ebFZ - ebBZ)*CLHEP::cm / 2.0, 0 * CLHEP::deg, 360.*CLHEP::deg);

        G4LogicalVolume* ebTube = new G4LogicalVolume(ebTubeShape, medNEMA_G10_Plate,
                                                      (format("logicalSD_ElectronicsBoard_Layer%1%") % iEB).str().c_str(), 0, 0, 0);
        if (!m_bkgsensitive) m_bkgsensitive = new BkgSensitiveDetector("CDC", iEB);
        ebTube->SetSensitiveDetector(m_bkgsensitive);
        ebTube->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (ebFZ + ebBZ)*CLHEP::cm / 2.0), ebTube,
                          (format("physicalSD_ElectronicsBoard_Layer%1%") % iEB).str().c_str(), logical_cdc, false, iEB);
      }

      //
      // Construct neutron shield.
      //
      createNeutronShields(geo);

      //
      // construct covers.
      //
      createCovers(geo);

      //
      // construct covers.
      //
      createCover2s(geo);

      //
      // Construct ribs.
      //
      for (const auto& rib : geo.getRibs()) {

        const int id = rib.getId();
        const double length = rib.getLength();
        const double width = rib.getWidth();
        const double thick = rib.getThick();
        const double rotx = rib.getRotX();
        const double roty = rib.getRotY();
        const double rotz = rib.getRotZ();
        const double x = rib.getX();
        const double y = rib.getY();
        const double z = rib.getZ();
        const int offset = rib.getOffset();
        const int ndiv = rib.getNDiv();

        const string solidName = "solidRib" + to_string(id);
        const string logicalName = "logicalRib" + to_string(id);
        G4Box* boxShape = new G4Box(solidName, 0.5 * length * CLHEP::cm,
                                    0.5 * width * CLHEP::cm,
                                    0.5 * thick * CLHEP::cm);

        const double rmax = 0.5 * length;
        const double rmin = max((rmax - thick), 0.);
        G4Tubs* tubeShape = new G4Tubs(solidName,
                                       rmin * CLHEP::cm,
                                       rmax * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.,
                                       360. * CLHEP::deg);

        //G4LogicalVolume* logicalV = new G4LogicalVolume(boxShape, medAluminum,
        //                                                logicalName, 0, 0, 0);
        // ID depndent material definition, Aluminum is default : Nakano
        G4LogicalVolume* logicalV = new G4LogicalVolume(boxShape, medAluminum, logicalName, 0, 0, 0);
        if (id > 39 && id < 78) // Cu
          logicalV = new G4LogicalVolume(boxShape, medCopper, logicalName, 0, 0, 0);
        if ((id > 77 && id < 94) || (id > 131 && id < 146))   // G10
          logicalV = new G4LogicalVolume(boxShape, medNEMA_G10_Plate, logicalName, 0, 0, 0);
        if (id > 93 && id < 110) // Cu
          logicalV = new G4LogicalVolume(tubeShape, medCopper, logicalName, 0, 0, 0);
        if (id > 109 && id < 126) // H2O
          logicalV = new G4LogicalVolume(tubeShape, medH2O, logicalName, 0, 0, 0);
        if (id > 127 && id < 132) // HV
          logicalV = new G4LogicalVolume(boxShape, medHV, logicalName, 0, 0, 0);
        /*if( id > 145 && id < 149 )// Fiber
          logicalV = new G4LogicalVolume(boxShape, medFiber, logicalName, 0, 0, 0);
        if( id > 148 && id < 158 )// Fiber
          logicalV = new G4LogicalVolume(boxShape, medCAT7, logicalName, 0, 0, 0);
        if( id > 157 && id < 164 )// Fiber
        logicalV = new G4LogicalVolume(boxShape, medTRG, logicalName, 0, 0, 0);*/

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / ndiv;

        G4RotationMatrix rot = G4RotationMatrix();
        double dz = thick;
        if (id > 93 && id < 126) dz = 0;

        G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - dz * CLHEP::cm / 2.0);
        rot.rotateX(rotx);
        rot.rotateY(roty);
        rot.rotateZ(rotz);
        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < ndiv; ++i) {
          const string physicalName = "physicalRib_" + to_string(id) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, id);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }

      //
      // Construct rib2s.
      //
      for (const auto& rib2 : geo.getRib2s()) {

        const int id = rib2.getId();
        const double length = rib2.getLength();
        const double width = rib2.getWidth();
        const double thick = rib2.getThick();
        const double width2 = rib2.getWidth2();
        const double thick2 = rib2.getThick2();
        const double rotx = rib2.getRotX();
        const double roty = rib2.getRotY();
        const double rotz = rib2.getRotZ();
        const double x = rib2.getX();
        const double y = rib2.getY();
        const double z = rib2.getZ();
        const int ndiv = rib2.getNDiv();

        const string solidName = "solidRib2" + to_string(id);
        const string logicalName = "logicalRib2" + to_string(id);
        G4Trd* trdShape = new G4Trd(solidName,
                                    0.5 * thick * CLHEP::cm,
                                    0.5 * thick2 * CLHEP::cm,
                                    0.5 * width * CLHEP::cm,
                                    0.5 * width2 * CLHEP::cm,
                                    0.5 * length * CLHEP::cm);

        G4LogicalVolume* logicalV = new G4LogicalVolume(trdShape, medAluminum,  logicalName, 0, 0, 0);

        if (id > 0)
          logicalV = new G4LogicalVolume(trdShape, medCopper,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / ndiv;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - thick * CLHEP::cm / 2.0);

        rot.rotateX(rotx);
        rot.rotateY(roty);
        rot.rotateZ(rotz);
        for (int i = 0; i < ndiv; ++i) {
          const string physicalName = "physicalRib2_" + to_string(id) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, id);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }

      //
      // Construct rib3s.
      //
      for (const auto& rib3 : geo.getRib3s()) {

        const int id = rib3.getId();
        const double length = rib3.getLength();
        const double width = rib3.getWidth();
        const double thick = rib3.getThick();
        const double r = rib3.getR();
        const double x = rib3.getX();
        const double y = rib3.getY();
        const double z = rib3.getZ();
        const double rx = rib3.getRx();
        const double ry = rib3.getRy();
        const double rz = rib3.getRz();
        const int offset = rib3.getOffset();
        const int ndiv = rib3.getNDiv();

        const string solidName = "solidRib3" + to_string(id);
        const string logicalName = "logicalRib3" + to_string(id);
        G4VSolid* boxShape = new G4Box("Block",
                                       0.5 * length * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.5 * thick * CLHEP::cm);
        G4VSolid* tubeShape = new G4Tubs("Hole",
                                         0.,
                                         r * CLHEP::cm,
                                         width * CLHEP::cm,
                                         0. * CLHEP::deg,
                                         360. * CLHEP::deg);

        G4RotationMatrix rotsub = G4RotationMatrix();
        rotsub.rotateX(90. * CLHEP::deg);
        G4ThreeVector trnsub(rx * CLHEP::cm - x * CLHEP::cm,  ry * CLHEP::cm - y * CLHEP::cm,
                             rz * CLHEP::cm - z * CLHEP::cm + 0.5 * thick * CLHEP::cm);
        G4VSolid* coolingBlock = new G4SubtractionSolid("Block-Hole",
                                                        boxShape,
                                                        tubeShape,
                                                        G4Transform3D(rotsub,
                                                            trnsub));

        G4LogicalVolume* logicalV = new G4LogicalVolume(coolingBlock, medCopper,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / ndiv;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - thick * CLHEP::cm / 2.0);

        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < ndiv; ++i) {
          const string physicalName = "physicalRib3_" + to_string(id) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, id);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }

      //
      // Construct rib4s.
      //
      for (const auto& rib4 : geo.getRib4s()) {

        const int id = rib4.getId();
        const double length = rib4.getLength();
        const double width = rib4.getWidth();
        const double thick = rib4.getThick();
        const double length2 = rib4.getLength2();
        const double width2 = rib4.getWidth2();
        const double thick2 = rib4.getThick2();
        const double x = rib4.getX();
        const double y = rib4.getY();
        const double z = rib4.getZ();
        const double x2 = rib4.getX2();
        const double y2 = rib4.getY2();
        const double z2 = rib4.getZ2();
        const int offset = rib4.getOffset();
        const int ndiv = rib4.getNDiv();

        const string solidName = "solidRib4" + to_string(id);
        const string logicalName = "logicalRib4" + to_string(id);
        G4VSolid* baseShape = new G4Box("Base",
                                        0.5 * length * CLHEP::cm,
                                        0.5 * width * CLHEP::cm,
                                        0.5 * thick * CLHEP::cm);
        G4VSolid* sqShape = new G4Box("Sq",
                                      0.5 * length2 * CLHEP::cm,
                                      0.5 * width2 * CLHEP::cm,
                                      0.5 * thick2 * CLHEP::cm);

        G4RotationMatrix rotsub = G4RotationMatrix();
        double dzc = (z2 - thick2 / 2.) - (z - thick / 2.);
        G4ThreeVector trnsub(x2 * CLHEP::cm - x * CLHEP::cm,
                             y2 * CLHEP::cm - y * CLHEP::cm,
                             dzc * CLHEP::cm);
        G4VSolid* sqHoleBase = new G4SubtractionSolid("Box-Sq",
                                                      baseShape,
                                                      sqShape,
                                                      G4Transform3D(rotsub,
                                                                    trnsub)
                                                     );

        G4LogicalVolume* logicalV = new G4LogicalVolume(sqHoleBase, medCopper,  logicalName, 0, 0, 0);
        if (id < 19)
          logicalV = new G4LogicalVolume(sqHoleBase, medNEMA_G10_Plate,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / ndiv;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - thick * CLHEP::cm / 2.0);

        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < ndiv; ++i) {
          const string physicalName = "physicalRib4_" + to_string(id) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, id);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }

      //
      // Construct rib5s.
      //
      for (const auto& rib5 : geo.getRib5s()) {

        const int id = rib5.getId();
        const double dr = rib5.getDr();
        const double dz = rib5.getDz();
        const double width = rib5.getWidth();
        const double thick = rib5.getThick();
        const double rin = rib5.getRin();
        const double x = rib5.getX();
        const double y = rib5.getY();
        const double z = rib5.getZ();
        const double rotx = rib5.getRotx();
        const double roty = rib5.getRoty();
        const double rotz = rib5.getRotz();
        const int offset = rib5.getOffset();
        const int ndiv = rib5.getNDiv();

        const string solidName = "solidRib5" + to_string(id);
        const string logicalName = "logicalRib5" + to_string(id);

        const double rmax = rin + thick;
        const double rmin = rin;
        const double dphi = 2. * atan2(dz, dr);
        const double ddphi = thick * tan(dphi) / rin;
        const double ddphi2 = width / 2. * width / 2. / (x + dr) / rin;
        const double cphi = dphi - ddphi - ddphi2;
        G4Tubs* tubeShape = new G4Tubs(solidName,
                                       rmin * CLHEP::cm,
                                       rmax * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.,
                                       cphi);

        G4LogicalVolume* logicalV = new G4LogicalVolume(tubeShape, medAluminum, logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / ndiv;

        G4RotationMatrix rot = G4RotationMatrix();

        //G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - thick * CLHEP::cm / 2.0);
        G4ThreeVector arm(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - rin * CLHEP::cm - thick * CLHEP::cm);
        rot.rotateX(rotx);
        rot.rotateY(roty);
        rot.rotateZ(rotz);
        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < ndiv; ++i) {
          const string physicalName = "physicalRib5_" + to_string(id) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, id);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }

      //Create B-field mapper (here tentatively)
      createMapper(topVolume);
    }


    void GeoCDCCreator::createNeutronShields(const GearDir& content)
    {

      G4Material* C2H4 = geometry::Materials::get("G4_POLYETHYLENE");
      G4Material* elB   = geometry::Materials::get("G4_B");

      // 5% borated polyethylene = SWX201
      // http://www.deqtech.com/Shieldwerx/Products/swx201hd.htm
      G4Material* boratedpoly05 = new G4Material("BoratedPoly05", 1.06 * CLHEP::g / CLHEP::cm3, 2);
      boratedpoly05->AddMaterial(elB, 0.05);
      boratedpoly05->AddMaterial(C2H4, 0.95);
      // 30% borated polyethylene = SWX210
      G4Material* boratedpoly30 = new G4Material("BoratedPoly30", 1.19 * CLHEP::g / CLHEP::cm3, 2);
      boratedpoly30->AddMaterial(elB, 0.30);
      boratedpoly30->AddMaterial(C2H4, 0.70);

      G4Material* shieldMat = C2H4;

      const int nShields = content.getNumberNodes("Shields/Shield");

      for (int iShield = 0; iShield < nShields; ++iShield) {
        GearDir shieldContent(content);
        shieldContent.append((format("/Shields/Shield[%1%]/") % (iShield + 1)).str());
        const string sShieldID = shieldContent.getString("@id");
        const int shieldID = atoi(sShieldID.c_str());
        //        const string shieldName = shieldContent.getString("Name");
        const double shieldInnerR1 = shieldContent.getLength("InnerR1");
        const double shieldInnerR2 = shieldContent.getLength("InnerR2");
        const double shieldOuterR1 = shieldContent.getLength("OuterR1");
        const double shieldOuterR2 = shieldContent.getLength("OuterR2");
        const double shieldThick = shieldContent.getLength("Thickness");
        const double shieldPosZ = shieldContent.getLength("PosZ");

        G4Cons* shieldConsShape = new G4Cons((format("solidShield%1%") % shieldID).str().c_str(),
                                             shieldInnerR1 * CLHEP::cm, shieldOuterR1 * CLHEP::cm,
                                             shieldInnerR2 * CLHEP::cm, shieldOuterR2 * CLHEP::cm,
                                             shieldThick * CLHEP::cm / 2.0,
                                             0.*CLHEP::deg, 360.*CLHEP::deg);

        G4LogicalVolume* shieldCons = new G4LogicalVolume(shieldConsShape, shieldMat, (format("logicalShield%1%") % shieldID).str().c_str(),
                                                          0, 0, 0);
        shieldCons->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (shieldPosZ - shieldThick / 2.0) * CLHEP::cm), shieldCons,
                          (format("physicalShield%1%") % shieldID).str().c_str(), logical_cdc, false, 0);

      }

    }


    void GeoCDCCreator::createNeutronShields(const CDCGeometry& geom)
    {

      G4Material* C2H4 = geometry::Materials::get("G4_POLYETHYLENE");
      G4Material* shieldMat = C2H4;

      for (const auto& shield : geom.getNeutronShields()) {
        const int shieldID = shield.getId();
        const double shieldInnerR1 = shield.getRmin1();
        const double shieldInnerR2 = shield.getRmin2();
        const double shieldOuterR1 = shield.getRmax1();
        const double shieldOuterR2 = shield.getRmax2();
        const double shieldThick = shield.getThick();
        const double shieldPosZ = shield.getZ();

        G4Cons* shieldConsShape = new G4Cons("solidShield" + to_string(shieldID),
                                             shieldInnerR1 * CLHEP::cm, shieldOuterR1 * CLHEP::cm,
                                             shieldInnerR2 * CLHEP::cm, shieldOuterR2 * CLHEP::cm,
                                             shieldThick * CLHEP::cm / 2.0,
                                             0.*CLHEP::deg, 360.*CLHEP::deg);

        G4LogicalVolume* shieldCons = new G4LogicalVolume(shieldConsShape, shieldMat, "logicalShield" + to_string(shieldID),
                                                          0, 0, 0);
        shieldCons->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, (shieldPosZ - shieldThick / 2.0) * CLHEP::cm), shieldCons,
                          "physicalShield" + to_string(shieldID), logical_cdc, false, 0);

      }

    }

    void GeoCDCCreator::createCovers(const GearDir& content)
    {
      string Aluminum  = content.getString("Aluminum");
      G4Material* medAluminum = geometry::Materials::get(Aluminum);
      G4Material* medNEMA_G10_Plate = geometry::Materials::get("NEMA_G10_Plate");
      // Nakano
      G4double density = 1.000 * CLHEP::g / CLHEP::cm3;
      G4double a = 1.01 * CLHEP::g / CLHEP::mole;
      G4Element* elH = new G4Element("Hydrogen", "H", 1., a);
      a = 16.00 * CLHEP::g / CLHEP::mole;
      G4Element* elO = new G4Element("Oxygen", "O", 8., a);
      G4Material* medH2O = new G4Material("Water", density, 2);
      medH2O->AddElement(elH, 2);
      medH2O->AddElement(elO, 1);
      G4Material* medCopper = geometry::Materials::get("Cu");
      G4Material* medLV = geometry::Materials::get("CDCLVCable");
      G4Material* medFiber = geometry::Materials::get("CDCOpticalFiber");
      G4Material* medCAT7 = geometry::Materials::get("CDCCAT7");
      G4Material* medTRG = geometry::Materials::get("CDCOpticalFiberTRG");
      G4Material* medHV = geometry::Materials::get("CDCHVCable");

      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      const int nCover = content.getNumberNodes("Covers/Cover");
      for (int iCover = 0; iCover < nCover; ++iCover) {
        GearDir coverContent(content);
        coverContent.append((format("/Covers/Cover[%1%]/") % (iCover + 1)).str());
        const string scoverID = coverContent.getString("@id");
        const int coverID = atoi(scoverID.c_str());
        const string coverName = coverContent.getString("Name");
        const double coverInnerR1 = coverContent.getLength("InnerR1");
        const double coverInnerR2 = coverContent.getLength("InnerR2");
        const double coverOuterR1 = coverContent.getLength("OuterR1");
        const double coverOuterR2 = coverContent.getLength("OuterR2");
        const double coverThick = coverContent.getLength("Thickness");
        const double coverPosZ = coverContent.getLength("PosZ");

        const double rmin1 = coverInnerR1;
        const double rmax1 = coverOuterR1;
        const double rmin2 = coverInnerR2;
        const double rmax2 = coverOuterR2;

        /*
              if (coverID == 7 || coverID == 10) {
                createCone(rmin1, rmax1, rmin2, rmax2, coverThick, coverPosZ, coverID, medAluminum, coverName);
              } else {
            createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medAluminum, coverName);

            }*/
        // ID dependent material definition
        if (coverID < 23) {
          if (coverID == 7 || coverID == 10) {// cones
            createCone(rmin1, rmax1, rmin2, rmax2, coverThick, coverPosZ, coverID, medAluminum, coverName);
          } else {// covers
            createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medAluminum, coverName);
          }
        }
        if (coverID > 22 && coverID < 29)// cooling plate
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medCopper, coverName);
        if (coverID > 28 && coverID < 35)// cooling Pipe
          createTorus(rmin1, rmax1, coverThick, coverPosZ, coverID, medCopper, coverName);
        if (coverID > 34 && coverID < 41)// cooling water
          createTorus(rmin1, rmax1, coverThick, coverPosZ, coverID, medH2O, coverName);
        if (coverID == 45 || coverID == 46)
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medLV, coverName);
        if (coverID == 47 || coverID == 48)
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medFiber, coverName);
        if (coverID == 49 || coverID == 50)
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medCAT7, coverName);
        if (coverID == 51 || coverID == 52)
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medTRG, coverName);
        if (coverID == 53)
          createTube(rmin1, rmax1, coverThick, coverPosZ, coverID, medHV, coverName);
      }

      const int nCover2 = content.getNumberNodes("Covers/Cover2");
      for (int iCover2 = 0; iCover2 < nCover2; ++iCover2) {
        GearDir cover2Content(content);
        cover2Content.append((format("/Cover2s/Cover2[%1%]/") % (iCover2 + 1)).str());
        const string scover2ID = cover2Content.getString("@id");
        const int cover2ID = atoi(scover2ID.c_str());
        const string cover2Name = cover2Content.getString("Name");
        const double cover2InnerR = cover2Content.getLength("InnerR");
        const double cover2OuterR = cover2Content.getLength("OuterR");
        const double cover2StartPhi = cover2Content.getLength("StartPhi");
        const double cover2DeltaPhi = cover2Content.getLength("DeltaPhi");
        const double cover2Thick = cover2Content.getLength("Thickness");
        const double cover2PosZ = cover2Content.getLength("PosZ");

        if (cover2ID < 11)
          createTube2(cover2InnerR, cover2OuterR, cover2StartPhi, cover2DeltaPhi, cover2Thick, cover2PosZ, cover2ID, medHV, cover2Name);
        if (cover2ID > 10 && cover2ID < 14)
          createTube2(cover2InnerR, cover2OuterR, cover2StartPhi, cover2DeltaPhi, cover2Thick, cover2PosZ, cover2ID, medFiber, cover2Name);
        if (cover2ID > 13 && cover2ID < 23)
          createTube2(cover2InnerR, cover2OuterR, cover2StartPhi, cover2DeltaPhi, cover2Thick, cover2PosZ, cover2ID, medCAT7, cover2Name);
        if (cover2ID > 22 && cover2ID < 29)
          createTube2(cover2InnerR, cover2OuterR, cover2StartPhi, cover2DeltaPhi, cover2Thick, cover2PosZ, cover2ID, medTRG, cover2Name);
      }

      const int nRibs = content.getNumberNodes("Covers/Rib");
      for (int iRib = 0; iRib < nRibs; ++iRib) {
        GearDir ribContent(content);
        ribContent.append((format("/Covers/Rib[%1%]/") % (iRib + 1)).str());
        const string sribID = ribContent.getString("@id");
        const int ribID = atoi(sribID.c_str());
        //        const string ribName = ribContent.getString("Name");
        const double length = ribContent.getLength("Length");
        const double width = ribContent.getLength("Width");
        const double thick = ribContent.getLength("Thickness");
        const double rotX = ribContent.getLength("RotX");
        const double rotY = ribContent.getLength("RotY");
        const double rotZ = ribContent.getLength("RotZ");
        const double cX = ribContent.getLength("PosX");
        const double cY = ribContent.getLength("PosY");
        const double cZ = ribContent.getLength("PosZ");
        const int offset = atoi((ribContent.getString("Offset")).c_str());
        const int number = atoi((ribContent.getString("NDiv")).c_str());

        const string solidName = "solidRib" + to_string(ribID);
        const string logicalName = "logicalRib" + to_string(ribID);
        G4Box* boxShape = new G4Box(solidName, 0.5 * length * CLHEP::cm,
                                    0.5 * width * CLHEP::cm,
                                    0.5 * thick * CLHEP::cm);
        const double rmax = 0.5 * length;
        const double rmin = max((rmax - thick), 0.);
        G4Tubs* tubeShape = new G4Tubs(solidName,
                                       rmin * CLHEP::cm,
                                       rmax * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.,
                                       360. * CLHEP::deg);

        //G4LogicalVolume* logicalV = new G4LogicalVolume(boxShape, medAluminum,
        //                                                logicalName, 0, 0, 0);
        // ID dependent material definition Aluminum is default: Nakano
        G4LogicalVolume* logicalV = new G4LogicalVolume(boxShape, medAluminum,  logicalName, 0, 0, 0);
        if (ribID > 39 && ribID < 78) // Cu box
          logicalV = new G4LogicalVolume(boxShape, medCopper,  logicalName, 0, 0, 0);
        if ((ribID > 77 && ribID < 94) || (ribID > 131 && ribID < 146)) // G10 box
          logicalV = new G4LogicalVolume(boxShape, medNEMA_G10_Plate,  logicalName, 0, 0, 0);
        if (ribID > 93 && ribID < 110) // Cu tube
          logicalV = new G4LogicalVolume(tubeShape, medCopper,  logicalName, 0, 0, 0);
        if (ribID > 109 && ribID < 126) // H2O tube (rmin = 0)
          logicalV = new G4LogicalVolume(tubeShape, medH2O,  logicalName, 0, 0, 0);
        if (ribID > 127 && ribID < 132) // HV bundle
          logicalV = new G4LogicalVolume(boxShape, medHV,  logicalName, 0, 0, 0);
        /*if( ribID > 145 && ribID < 149 )// Fiber box
          logicalV = new G4LogicalVolume(boxShape, medFiber,  logicalName, 0, 0, 0);
        if( ribID > 148 && ribID < 158 )// Fiber box
          logicalV = new G4LogicalVolume(boxShape, medCAT7,  logicalName, 0, 0, 0);
        if( ribID > 157 && ribID < 164 )// Fiber box
        logicalV = new G4LogicalVolume(boxShape, medTRG,  logicalName, 0, 0, 0);*/

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / number;

        G4RotationMatrix rot = G4RotationMatrix();

        double dz = thick;
        if (ribID > 93 && ribID < 126) dz = 0;
        G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - dz * CLHEP::cm / 2.0);

        rot.rotateX(rotX);
        rot.rotateY(rotY);
        rot.rotateZ(rotZ);
        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < number; ++i) {
          const string physicalName = "physicalRib_" + to_string(ribID) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, ribID);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }// rib

      const int nRib2s = content.getNumberNodes("Covers/Rib2");
      for (int iRib2 = 0; iRib2 < nRib2s; ++iRib2) {
        GearDir rib2Content(content);
        rib2Content.append((format("/Covers/Rib2[%1%]/") % (iRib2 + 1)).str());
        const string srib2ID = rib2Content.getString("@id");
        const int rib2ID = atoi(srib2ID.c_str());
        //        const string rib2Name = rib2Content.getString("Name");
        const double length = rib2Content.getLength("Length");
        const double width = rib2Content.getLength("Width");
        const double thick = rib2Content.getLength("Thickness");
        const double width2 = rib2Content.getLength("Width2");
        const double thick2 = rib2Content.getLength("Thickness2");
        const double rotX = rib2Content.getLength("RotX");
        const double rotY = rib2Content.getLength("RotY");
        const double rotZ = rib2Content.getLength("RotZ");
        const double cX = rib2Content.getLength("PosX");
        const double cY = rib2Content.getLength("PosY");
        const double cZ = rib2Content.getLength("PosZ");
        const int number = atoi((rib2Content.getString("NDiv")).c_str());

        const string solidName = "solidRib2" + to_string(rib2ID);
        const string logicalName = "logicalRib2" + to_string(rib2ID);
        G4Trd* trdShape = new G4Trd(solidName,
                                    0.5 * thick * CLHEP::cm,
                                    0.5 * thick2 * CLHEP::cm,
                                    0.5 * width * CLHEP::cm,
                                    0.5 * width2 * CLHEP::cm,
                                    0.5 * length * CLHEP::cm);

        G4LogicalVolume* logicalV = new G4LogicalVolume(trdShape, medAluminum,  logicalName, 0, 0, 0);
        if (rib2ID > 0)
          logicalV = new G4LogicalVolume(trdShape, medCopper,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / number;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - thick * CLHEP::cm / 2.0);

        rot.rotateX(rotX);
        rot.rotateY(rotY);
        rot.rotateZ(rotZ);
        for (int i = 0; i < number; ++i) {
          const string physicalName = "physicalRib2_" + to_string(rib2ID) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, rib2ID);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }// rib2

      const int nRib3s = content.getNumberNodes("Covers/Rib3");
      for (int iRib3 = 0; iRib3 < nRib3s; ++iRib3) {
        GearDir rib3Content(content);
        rib3Content.append((format("/Covers/Rib3[%1%]/") % (iRib3 + 1)).str());
        const string srib3ID = rib3Content.getString("@id");
        const int rib3ID = atoi(srib3ID.c_str());
        //        const string rib3Name = rib3Content.getString("Name");
        const double length = rib3Content.getLength("Length");
        const double width = rib3Content.getLength("Width");
        const double thick = rib3Content.getLength("Thickness");
        const double r = rib3Content.getLength("HoleR");
        const double cX = rib3Content.getLength("PosX");
        const double cY = rib3Content.getLength("PosY");
        const double cZ = rib3Content.getLength("PosZ");
        const double hX = rib3Content.getLength("HoleX");
        const double hY = rib3Content.getLength("HoleY");
        const double hZ = rib3Content.getLength("HoleZ");
        const int offset = atoi((rib3Content.getString("Offset")).c_str());
        const int number = atoi((rib3Content.getString("NDiv")).c_str());

        const string solidName = "solidRib3" + to_string(rib3ID);
        const string logicalName = "logicalRib3" + to_string(rib3ID);
        G4VSolid* boxShape = new G4Box("Block",
                                       0.5 * length * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.5 * thick * CLHEP::cm);
        G4VSolid* tubeShape = new G4Tubs("Hole",
                                         0.,
                                         r * CLHEP::cm,
                                         length * CLHEP::cm,
                                         0.,
                                         360. * CLHEP::deg);
        G4RotationMatrix rotsub = G4RotationMatrix();
        G4ThreeVector trnsub(cX * CLHEP::cm - hX * CLHEP::cm,  cY * CLHEP::cm - hY * CLHEP::cm,
                             cZ * CLHEP::cm - hZ * CLHEP::cm + 0.5 * thick * CLHEP::cm);
        G4VSolid* coolingBlock = new G4SubtractionSolid("Block-Hole",
                                                        boxShape,
                                                        tubeShape,
                                                        G4Transform3D(rotsub,
                                                            trnsub));

        G4LogicalVolume* logicalV = new G4LogicalVolume(coolingBlock, medCopper,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / number;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - thick * CLHEP::cm / 2.0);

        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < number; ++i) {
          const string physicalName = "physicalRib3_" + to_string(rib3ID) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, rib3ID);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }// rib3

      const int nRib4s = content.getNumberNodes("Covers/Rib4");
      for (int iRib4 = 0; iRib4 < nRib4s; ++iRib4) {
        GearDir rib4Content(content);
        rib4Content.append((format("/Covers/Rib4[%1%]/") % (iRib4 + 1)).str());
        const string srib4ID = rib4Content.getString("@id");
        const int rib4ID = atoi(srib4ID.c_str());
        //        const string rib4Name = rib4Content.getString("Name");
        const double length = rib4Content.getLength("Length");
        const double width = rib4Content.getLength("Width");
        const double thick = rib4Content.getLength("Thickness");
        const double length2 = rib4Content.getLength("Length2");
        const double width2 = rib4Content.getLength("Width2");
        const double thick2 = rib4Content.getLength("Thickness2");
        const double cX = rib4Content.getLength("PosX");
        const double cY = rib4Content.getLength("PosY");
        const double cZ = rib4Content.getLength("PosZ");
        const double hX = rib4Content.getLength("HoleX");
        const double hY = rib4Content.getLength("HoleY");
        const double hZ = rib4Content.getLength("HoleZ");
        const int offset = atoi((rib4Content.getString("Offset")).c_str());
        const int number = atoi((rib4Content.getString("NDiv")).c_str());

        const string solidName = "solidRib4" + to_string(rib4ID);
        const string logicalName = "logicalRib4" + to_string(rib4ID);
        G4VSolid* baseShape = new G4Box("Base",
                                        0.5 * length * CLHEP::cm,
                                        0.5 * width * CLHEP::cm,
                                        0.5 * thick * CLHEP::cm);
        G4VSolid* sqShape = new G4Box("Sq",
                                      0.5 * length2 * CLHEP::cm,
                                      0.5 * width2 * CLHEP::cm,
                                      0.5 * thick2 * CLHEP::cm);
        G4RotationMatrix rotsub = G4RotationMatrix();
        double dzc = (hZ - thick2 / 2.) - (cZ - thick / 2.);
        G4ThreeVector trnsub(hX * CLHEP::cm - cX * CLHEP::cm,
                             hY * CLHEP::cm - cY * CLHEP::cm,
                             dzc * CLHEP::cm);
        G4VSolid* sqHoleBase = new G4SubtractionSolid("Base-Sq",
                                                      baseShape,
                                                      sqShape,
                                                      G4Transform3D(rotsub,
                                                                    trnsub)
                                                     );

        G4LogicalVolume* logicalV = new G4LogicalVolume(sqHoleBase, medCopper,  logicalName, 0, 0, 0);
        if (rib4ID < 19)
          logicalV = new G4LogicalVolume(sqHoleBase, medNEMA_G10_Plate,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / number;

        G4RotationMatrix rot = G4RotationMatrix();
        G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - thick * CLHEP::cm / 2.0);

        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < number; ++i) {
          const string physicalName = "physicalRib4_" + to_string(rib4ID) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, rib4ID);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }

      }// rib4

      const int nRib5s = content.getNumberNodes("Covers/Rib5");
      for (int iRib5 = 0; iRib5 < nRib5s; ++iRib5) {
        GearDir rib5Content(content);
        rib5Content.append((format("/Covers/Rib5[%1%]/") % (iRib5 + 1)).str());
        const string srib5ID = rib5Content.getString("@id");
        const int rib5ID = atoi(srib5ID.c_str());
        //        const string rib5Name = rib5Content.getString("Name");
        const double dr = rib5Content.getLength("DeltaR");
        const double dz = rib5Content.getLength("DeltaZ");
        const double width = rib5Content.getLength("Width");
        const double thick = rib5Content.getLength("Thickness");
        const double rin = rib5Content.getLength("Rin");
        const double rotX = rib5Content.getLength("RotX");
        const double rotY = rib5Content.getLength("RotY");
        const double rotZ = rib5Content.getLength("RotZ");
        const double cX = rib5Content.getLength("PosX");
        const double cY = rib5Content.getLength("PosY");
        const double cZ = rib5Content.getLength("PosZ");
        const int offset = atoi((rib5Content.getString("Offset")).c_str());
        const int number = atoi((rib5Content.getString("NDiv")).c_str());

        const string solidName = "solidRib5" + to_string(rib5ID);
        const string logicalName = "logicalRib5" + to_string(rib5ID);
        const double rmax = rin + thick;
        const double rmin = rin;
        const double dphi = 2. * atan2(dz, dr);
        const double ddphi = thick * tan(dphi) / rin;
        const double ddphi2 = width / 2. * width / 2. / (cX + dr) / rin;
        const double cphi = dphi - ddphi - ddphi2;
        G4Tubs* tubeShape = new G4Tubs(solidName,
                                       rmin * CLHEP::cm,
                                       rmax * CLHEP::cm,
                                       0.5 * width * CLHEP::cm,
                                       0.,
                                       cphi);

        G4LogicalVolume* logicalV = new G4LogicalVolume(tubeShape, medAluminum,  logicalName, 0, 0, 0);

        logicalV->SetVisAttributes(m_VisAttributes.back());

        const double phi = 360.0 / number;

        G4RotationMatrix rot = G4RotationMatrix();

        //G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - thick * CLHEP::cm / 2.0);
        G4ThreeVector arm(cX * CLHEP::cm, cY * CLHEP::cm, cZ * CLHEP::cm - rin * CLHEP::cm - thick * CLHEP::cm);

        rot.rotateX(rotX);
        rot.rotateY(rotY);
        rot.rotateZ(rotZ);
        if (offset) {
          rot.rotateZ(0.5 * phi * CLHEP::deg);
          arm.rotateZ(0.5 * phi * CLHEP::deg);
        }
        for (int i = 0; i < number; ++i) {
          const string physicalName = "physicalRib5_" + to_string(rib5ID) + " " + to_string(i);
          new G4PVPlacement(G4Transform3D(rot, arm), logicalV,
                            physicalName.c_str(), logical_cdc, false, rib5ID);
          rot.rotateZ(phi * CLHEP::deg);
          arm.rotateZ(phi * CLHEP::deg);
        }
      }//rib5

    }


    void GeoCDCCreator::createCovers(const CDCGeometry& geom)
    {
      G4Material* medAl = geometry::Materials::get("Al");
      // Nakano
      G4double density = 1.000 * CLHEP::g / CLHEP::cm3;
      G4double a = 1.01 * CLHEP::g / CLHEP::mole;
      G4Element* elH = new G4Element("Hydrogen", "H", 1., a);
      a = 16.00 * CLHEP::g / CLHEP::mole;
      G4Element* elO = new G4Element("Oxygen", "O", 8., a);
      G4Material* medH2O = new G4Material("water", density, 2);
      medH2O->AddElement(elH, 2);
      medH2O->AddElement(elO, 1);
      G4Material* medCu = geometry::Materials::get("Cu");
      G4Material* medLV = geometry::Materials::get("CDCLVCable");
      G4Material* medFiber = geometry::Materials::get("CDCOpticalFiber");
      G4Material* medCAT7 = geometry::Materials::get("CDCCAT7");
      G4Material* medTRG = geometry::Materials::get("CDCOpticalFiberTRG");
      G4Material* medHV = geometry::Materials::get("CDCHVCable");

      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      for (const auto& cover : geom.getCovers()) {
        const int coverID = cover.getId();
        const string coverName = "cover" + to_string(coverID);
        const double rmin1 = cover.getRmin1();
        const double rmin2 = cover.getRmin2();
        const double rmax1 = cover.getRmax1();
        const double rmax2 = cover.getRmax2();
        const double thick = cover.getThick();
        const double posZ = cover.getZ();

        /*if (coverID == 7 || coverID == 10) {
          createCone(rmin1, rmax1, rmin2, rmax2, thick, posZ, coverID, medAl, coverName);
        } else {
          createTube(rmin1, rmax1, thick, posZ, coverID, medAl, coverName);
        }*/
        // ID dependent material definition
        if (coverID < 23) {
          if (coverID == 7 || coverID == 10) {
            createCone(rmin1, rmax1, rmin2, rmax2, thick, posZ, coverID, medAl, coverName);
          } else {
            createTube(rmin1, rmax1, thick, posZ, coverID, medAl, coverName);
          }
        }
        if (coverID > 22 && coverID < 29)
          createTube(rmin1, rmax1, thick, posZ, coverID, medCu, coverName);
        if (coverID > 28 && coverID < 35)
          createTorus(rmin1, rmax1, thick, posZ, coverID, medCu, coverName);
        if (coverID > 34 && coverID < 41)
          createTorus(rmin1, rmax1, thick, posZ, coverID, medH2O, coverName);
        if (coverID == 45 || coverID == 46)
          createTube(rmin1, rmax1, thick, posZ, coverID, medLV, coverName);
        if (coverID == 47 || coverID == 48)
          createTube(rmin1, rmax1, thick, posZ, coverID, medFiber, coverName);
        if (coverID == 49 || coverID == 50)
          createTube(rmin1, rmax1, thick, posZ, coverID, medCAT7, coverName);
        if (coverID == 51 || coverID == 52)
          createTube(rmin1, rmax1, thick, posZ, coverID, medTRG, coverName);
        if (coverID == 53)
          createTube(rmin1, rmax1, thick, posZ, coverID, medHV, coverName);
      }
    }

    void GeoCDCCreator::createCover2s(const CDCGeometry& geom)
    {
      G4Material* medHV = geometry::Materials::get("CDCHVCable");
      G4Material* medFiber = geometry::Materials::get("CDCOpticalFiber");
      G4Material* medCAT7 = geometry::Materials::get("CDCCAT7");
      G4Material* medTRG = geometry::Materials::get("CDCOpticalFiberTRG");

      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      for (const auto& cover2 : geom.getCover2s()) {
        const int cover2ID = cover2.getId();
        const string cover2Name = "cover2" + to_string(cover2ID);
        const double rmin = cover2.getRmin();
        const double rmax = cover2.getRmax();
        const double phis = cover2.getPhis();
        const double dphi = cover2.getDphi();
        const double thick = cover2.getThick();
        const double posZ = cover2.getZ();

        if (cover2ID < 11)
          createTube2(rmin, rmax, phis, dphi, thick, posZ, cover2ID, medHV, cover2Name);
        if (cover2ID > 10 && cover2ID < 14)
          createTube2(rmin, rmax, phis, dphi, thick, posZ, cover2ID, medFiber, cover2Name);
        if (cover2ID > 13 && cover2ID < 23)
          createTube2(rmin, rmax, phis, dphi, thick, posZ, cover2ID, medCAT7, cover2Name);
        if (cover2ID > 22 && cover2ID < 29)
          createTube2(rmin, rmax, phis, dphi, thick, posZ, cover2ID, medTRG, cover2Name);
      }
    }

    void GeoCDCCreator::createCone(const double rmin1, const double rmax1,
                                   const double rmin2, const double rmax2,
                                   const double thick, const double posZ,
                                   const int id, G4Material* med,
                                   const string& name)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      G4Cons* coverConeShape = new G4Cons(solidName.c_str(), rmin1 * CLHEP::cm, rmax1 * CLHEP::cm,
                                          rmin2 * CLHEP::cm, rmax2 * CLHEP::cm, thick * CLHEP::cm / 2.0, 0.*CLHEP::deg, 360.*CLHEP::deg);
      G4LogicalVolume* coverCone = new G4LogicalVolume(coverConeShape, med,
                                                       logicalName.c_str(), 0, 0, 0);
      coverCone->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::cm - thick * CLHEP::cm / 2.0), coverCone,
                        physicalName.c_str(), logical_cdc, false, id);

    }

    void GeoCDCCreator::createTube(const double rmin, const double rmax,
                                   const double thick, const double posZ,
                                   const int id, G4Material* med,
                                   const string& name)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      G4Tubs* solidV = new G4Tubs(solidName.c_str(),
                                  rmin * CLHEP::cm,
                                  rmax * CLHEP::cm,
                                  thick * CLHEP::cm / 2.0,
                                  0.*CLHEP::deg,
                                  360.*CLHEP::deg);
      G4LogicalVolume* logicalV = new G4LogicalVolume(solidV, med,
                                                      logicalName.c_str(), 0, 0, 0);
      logicalV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::cm - thick * CLHEP::cm / 2.0), logicalV,
                        physicalName.c_str(), logical_cdc, false, id);

    }

    void GeoCDCCreator::createBox(const double length, const double height,
                                  const double thick, const double x,
                                  const double y, const double z,
                                  const int id, G4Material* med,
                                  const string& name)
    {
      const string solidName = (format("solid%1%%2%") % name % id).str();
      const string logicalName = (format("logical%1%%2%") % name % id).str();
      const string physicalName = (format("physical%1%%2%") % name % id).str();
      G4Box* boxShape = new G4Box(solidName.c_str(), 0.5 * length * CLHEP::cm,
                                  0.5 * height * CLHEP::cm,
                                  0.5 * thick * CLHEP::cm);
      G4LogicalVolume* logicalV = new G4LogicalVolume(boxShape, med,
                                                      logicalName.c_str(), 0, 0, 0);
      logicalV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(x * CLHEP::cm, y * CLHEP::cm, z * CLHEP::cm - thick * CLHEP::cm / 2.0), logicalV,
                        physicalName.c_str(), logical_cdc, false, id);

    }

    void GeoCDCCreator::createTorus(const double rmin1, const double rmax1,
                                    const double thick, const double posZ,
                                    const int id, G4Material* med,
                                    const string& name)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      const double rtor = (rmax1 + rmin1) / 2.;
      const double rmax = rmax1 - rtor;
      const double rmin = max((rmax - thick), 0.);

      G4Torus* solidV = new G4Torus(solidName.c_str(),
                                    rmin * CLHEP::cm,
                                    rmax * CLHEP::cm,
                                    rtor * CLHEP::cm,
                                    0.*CLHEP::deg,
                                    360.*CLHEP::deg);
      G4LogicalVolume* logicalV = new G4LogicalVolume(solidV, med,
                                                      logicalName.c_str(), 0, 0, 0);
      logicalV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::cm), logicalV,
                        physicalName.c_str(), logical_cdc, false, id);

    }

    void GeoCDCCreator::createTube2(const double rmin, const double rmax,
                                    const double phis, const double phie,
                                    const double thick, const double posZ,
                                    const int id, G4Material* med,
                                    const string& name)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      G4Tubs* solidV = new G4Tubs(solidName.c_str(),
                                  rmin * CLHEP::cm,
                                  rmax * CLHEP::cm,
                                  thick * CLHEP::cm / 2.0,
                                  phis,
                                  phie);
      G4LogicalVolume* logicalV = new G4LogicalVolume(solidV, med,
                                                      logicalName.c_str(), 0, 0, 0);
      logicalV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::cm - thick * CLHEP::cm / 2.0), logicalV,
                        physicalName.c_str(), logical_cdc, false, id);

    }

    void GeoCDCCreator::createMapper(G4LogicalVolume& topVolume)
    {
      CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();
      if (!gcp.getMapperGeometry()) return;

      const double xc = 0.5 * (-0.0002769 +  0.0370499) * CLHEP::cm;
      const double yc = 0.5 * (-0.0615404 + -0.108948) * CLHEP::cm;
      const double zc = 0.5 * (-35.3   +    48.5) * CLHEP::cm;
      //3 plates
      //        const double plateWidth = 13.756 * CLHEP::cm;
      //        const double plateThick =  1.203 * CLHEP::cm;
      //        const double plateLength = 83.706 * CLHEP::cm;
      const double plateWidth  = 13.8 * CLHEP::cm;
      const double plateThick  =  1.2 * CLHEP::cm;
      const double plateLength = 83.8 * CLHEP::cm;
      const double phi = gcp.getMapperPhiAngle() * CLHEP::deg; //phi-angle in lab.
      //  std::cout << "phi= " << phi << std::endl;
      //        const double endRingRmin =  4.1135 * CLHEP::cm;
      //        const double endRingRmax = 15.353 * CLHEP::cm;
      //        const double endRingThick =  2.057 * CLHEP::cm;
      const double endPlateRmin     =  4.0 * CLHEP::cm;
      const double endPlateRmax     = 15.5 * CLHEP::cm;
      const double bwdEndPlateThick =  1.7 * CLHEP::cm;
      const double fwdEndPlateThick =  2.0 * CLHEP::cm;

      G4Material* medAluminum = geometry::Materials::get("Al");

      string name = "Plate";
      int pID = 0;
      G4Box* plateShape = new G4Box("solid" + name, .5 * plateWidth, .5 * plateThick, .5 * plateLength);
      G4LogicalVolume* logical0 = new G4LogicalVolume(plateShape, medAluminum, "logical" + name, 0, 0, 0);
      logical0->SetVisAttributes(m_VisAttributes.back());
      //        const double x = .5 * plateWidth;
      const double x = xc + 0.5 * plateWidth;
      //        const double y = endRingRmin;
      const double y = yc + endPlateRmin + 0.1 * CLHEP::cm;
      //        double z = 2.871 * CLHEP::cm;
      G4ThreeVector xyz(x, y, zc);
      G4RotationMatrix rotM3 = G4RotationMatrix();
      xyz.rotateZ(phi);
      rotM3.rotateZ(phi);
      new G4PVPlacement(G4Transform3D(rotM3, xyz), logical0, "physical" + name, &topVolume, false, pID);

      const double alf = 120. * CLHEP::deg;
      xyz.rotateZ(alf);
      rotM3.rotateZ(alf);
      new G4PVPlacement(G4Transform3D(rotM3, xyz), logical0, "physical" + name, &topVolume, false, pID + 1);

      xyz.rotateZ(alf);
      rotM3.rotateZ(alf);
      new G4PVPlacement(G4Transform3D(rotM3, xyz), logical0, "physical" + name, &topVolume, false, pID + 2);

      //Define 2 end-plates
      //bwd
      name = "BwdEndPlate";
      G4Tubs* BwdEndPlateShape = new G4Tubs("solid" + name, endPlateRmin, endPlateRmax, 0.5 * bwdEndPlateThick, 0., 360.*CLHEP::deg);
      G4LogicalVolume* logical1 = new G4LogicalVolume(BwdEndPlateShape, medAluminum, "logical" + name, 0, 0, 0);
      logical1->SetVisAttributes(m_VisAttributes.back());
      //        z = -40.0105 * CLHEP::cm;
      double z = -35.3 * CLHEP::cm - 0.5 * bwdEndPlateThick;
      pID = 0;
      new G4PVPlacement(0, G4ThreeVector(xc, yc, z), logical1, "physical" + name, &topVolume, false, pID);

      //fwd
      //        z = 45.7525 * CLHEP::cm;
      //        new G4PVPlacement(0, G4ThreeVector(0., 0., z), logical1, "physical" + name, &topVolume, false, pID + 1);
      name = "FwdEndPlate";
      G4Tubs* FwdEndPlateShape = new G4Tubs("solid" + name, endPlateRmin, endPlateRmax, 0.5 * fwdEndPlateThick, 0., 360.*CLHEP::deg);
      G4LogicalVolume* logical2 = new G4LogicalVolume(FwdEndPlateShape, medAluminum, "logical" + name, 0, 0, 0);
      logical2->SetVisAttributes(m_VisAttributes.back());
      z = 48.5 * CLHEP::cm + 0.5 * fwdEndPlateThick;
      new G4PVPlacement(0, G4ThreeVector(xc, yc, z), logical2, "physical" + name, &topVolume, false, pID);
    }
  }
}
