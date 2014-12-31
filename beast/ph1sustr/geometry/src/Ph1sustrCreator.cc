/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/ph1sustr/geometry/Ph1sustrCreator.h>
#include <beast/ph1sustr/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <ph1sustr/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

//Visualization
#include "G4Colour.hh"
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PH1SUSTR detector */
  namespace ph1sustr {

    // Register the creator
    /** Creator creates the phase 1 support structure geometry */
    geometry::CreatorFactory<Ph1sustrCreator> Ph1sustrFactory("PH1SUSTRCreator");

    Ph1sustrCreator::Ph1sustrCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    Ph1sustrCreator::~Ph1sustrCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1sustrCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      //double stepSize = content.getLength("stepSize", 5 * CLHEP::um);


      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      double x_tpcbeamR = 0;
      double y_tpcbeamR = 0;
      double z_tpcbeamR = 0;
      double x_tpcbeamL = 0;
      double y_tpcbeamL = 0;
      double z_tpcbeamL = 0;
      double x_tpcbeamT = 0;
      double y_tpcbeamT = 0;
      double z_tpcbeamT = 0;
      double x_tpcbeamB = 0;
      double y_tpcbeamB = 0;
      double z_tpcbeamB = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //plate positions
        x_tpcbeamR = activeParams.getLength("x_tpcbeamR") * CLHEP::cm;
        y_tpcbeamR = activeParams.getLength("y_tpcbeamR") * CLHEP::cm;
        z_tpcbeamR = activeParams.getLength("z_tpcbeamR") * CLHEP::cm;
        x_tpcbeamL = activeParams.getLength("x_tpcbeamL") * CLHEP::cm;
        y_tpcbeamL = activeParams.getLength("y_tpcbeamL") * CLHEP::cm;
        z_tpcbeamL = activeParams.getLength("z_tpcbeamL") * CLHEP::cm;
        x_tpcbeamT = activeParams.getLength("x_tpcbeamT") * CLHEP::cm;
        y_tpcbeamT = activeParams.getLength("y_tpcbeamT") * CLHEP::cm;
        z_tpcbeamT = activeParams.getLength("z_tpcbeamT") * CLHEP::cm;
        x_tpcbeamB = activeParams.getLength("x_tpcbeamB") * CLHEP::cm;
        y_tpcbeamB = activeParams.getLength("y_tpcbeamB") * CLHEP::cm;
        z_tpcbeamB = activeParams.getLength("z_tpcbeamB") * CLHEP::cm;
        //plate orientation
        /*
        G4RotationMatrix* rotXx = new G4RotationMatrix();
              //G4double AngleX = activeParams.getAngle("AngleX");
        //G4double AngleY = activeParams.getAngle("AngleY");
              //G4double AngleZ = activeParams.getAngle("AngleZ");
        rotXx->rotateX( activeParams.getAngle("AngleX") );
        rotXx->rotateY( activeParams.getAngle("AngleY") );
              rotXx->rotateZ( activeParams.getAngle("AngleZ") );

        //define tpc beam and plate dimensions
        double betpcbeam = 190.8/2. * CLHEP::mm;
        G4double dx_tpcbeam = 2.54 * 1.63 / 2.*CLHEP::cm;
        G4double dy_tpcbeam = 2.54 * 1.63 / 2.*CLHEP::cm;
        G4double dz_tpcbeam = 2200. / 2.*CLHEP::mm;
        G4double dw_tpcbeam = 2.54 * 0.25 / 2.*CLHEP::cm;
        G4double dx_plate = 2.54 * 0.35 / 2.*CLHEP::cm;
        G4double dy_plate = betpcbeam + 4. * dy_tpcbeam;
        G4double dz_plate = 400. / 2.*CLHEP::mm;

        //create plate volume
        G4VSolid* s_plate = new G4Box("s_plate", dx_plate, dy_plate, dz_plate);

        //place plate volume
        G4LogicalVolume* l_plate = new G4LogicalVolume(s_plate,  geometry::Materials::get("MetalCopper") , "l_plate", 0, 0);


        G4VSolid* s_tpcbeam_a = new G4Box("s_tpcbeam_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeam);
        G4VSolid* s_tpcbeam_b = new G4Box("s_tpcbeam_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeam);
        G4VSolid* s_tpcbeampos = new G4SubtractionSolid("s_tpcbeampos", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamneg = new G4SubtractionSolid("s_tpcbeamneg", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_tpcbeam = new G4UnionSolid("s_tpcbeam", s_tpcbeampos, s_tpcbeamneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));

        //create tpc beam volumes
        G4LogicalVolume* l_tpcbeam = new G4LogicalVolume(s_tpcbeam,  geometry::Materials::get("MetalCopper") , "l_tpcbeam", 0, 0);

        //place plate volume
        G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                    activeParams.getLength("x_tpcbeam") * CLHEP::cm,
                    activeParams.getLength("y_tpcbeam") * CLHEP::cm,
                    activeParams.getLength("z_tpcbeam") * CLHEP::cm
                    );
        new G4PVPlacement(rotXx, PH1SUSTRpos, l_plate, "p_plate", &topVolume, false, 1);

        //place 1st tpc beam volume
        PH1SUSTRpos = G4ThreeVector(
                  activeParams.getLength("x_tpcbeam") * CLHEP::cm + dx_plate + dx_tpcbeam,
                  activeParams.getLength("y_tpcbeam") * CLHEP::cm + betpcbeam + 2. * dy_tpcbeam,
                  0.
                  );
        new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam1", &topVolume, false, 1);

        //place 2nd tpc beam volume
        PH1SUSTRpos = G4ThreeVector(
                  activeParams.getLength("x_tpcbeam") * CLHEP::cm + dx_plate + dx_tpcbeam,
                  activeParams.getLength("y_tpcbeam") * CLHEP::cm - betpcbeam - 2. * dy_tpcbeam,
                  0.
                  );
        new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam2", &topVolume, false, 1);
        detID++;
        */
      }

      //TPC vertical: 4x @ 1614/ea
      //TPC horizontal + BGO base: 8x @ 1583/ea
      //TPC railroad: 8x @ 2200/ea
      //BGO vertical: 4x @ 928/ea
      //BGO horizontal: 4x @ 318/ea
      //G4double dz_20V2100bgov = 843.72 / 2.*CLHEP::mm;
      //G4double dz_20V2100bgoh = 280.00 / 2.*CLHEP::mm;

      //Beam supporting the TPC-Tube-plate
      //define tpc beam and plate dimensions
      double betpcbeam = 190.8 / 2. * CLHEP::mm;
      G4double dx_tpcbeam = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dy_tpcbeam = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dz_tpcbeam = 2200. / 2.*CLHEP::mm;
      G4double dw_tpcbeam = 2.54 * 0.25 / 2.*CLHEP::cm;
      G4double dx_plate = 2.54 * 0.35 / 2.*CLHEP::cm;
      G4double dy_plate = betpcbeam + 4. * dy_tpcbeam;
      G4double dz_plate = 400. / 2.*CLHEP::mm;


      //Right from e^-~--~beam
      //create plate volume
      G4VSolid* s_plate = new G4Box("s_plate", dx_plate, dy_plate, dz_plate);

      //place plate volume
      G4LogicalVolume* l_plate = new G4LogicalVolume(s_plate,  geometry::Materials::get("Aluminum") , "l_plate", 0, 0);
      G4VisAttributes* white = new G4VisAttributes(G4Colour(1, 1, 1));
      white->SetForceAuxEdgeVisible(true);
      l_plate->SetVisAttributes(white);

      G4VSolid* s_tpcbeam_a = new G4Box("s_tpcbeam_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeam);
      G4VSolid* s_tpcbeam_b = new G4Box("s_tpcbeam_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeam);
      G4VSolid* s_tpcbeampos = new G4SubtractionSolid("s_tpcbeampos", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
      G4VSolid* s_tpcbeamneg = new G4SubtractionSolid("s_tpcbeamneg", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
      G4VSolid* s_tpcbeam = new G4UnionSolid("s_tpcbeam", s_tpcbeampos, s_tpcbeamneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));

      //create tpc beam volumes
      G4LogicalVolume* l_tpcbeam = new G4LogicalVolume(s_tpcbeam,  geometry::Materials::get("FG_Epoxy") , "l_tpcbeam", 0, 0);

      //place plate volume
      G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                                    x_tpcbeamR,
                                    y_tpcbeamR,
                                    z_tpcbeamR
                                  );
      new G4PVPlacement(0, PH1SUSTRpos, l_plate, "p_plate", &topVolume, false, 1);

      //place 1st tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamR + dx_plate + dx_tpcbeam,
                      y_tpcbeamR + betpcbeam + 2. * dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //place 2nd tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamR + dx_plate + dx_tpcbeam,
                      y_tpcbeamR - betpcbeam - 2. * dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //Left from e^-~--~beam
      //place plate volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamL,
                      y_tpcbeamL,
                      z_tpcbeamL
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_plate, "p_plate", &topVolume, false, 1);

      //place 1st tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamL - dx_plate - dx_tpcbeam,
                      y_tpcbeamL + betpcbeam + 2. * dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //place 2nd tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamL - dx_plate - dx_tpcbeam,
                      y_tpcbeamL - betpcbeam - 2. * dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //Bottom
      G4RotationMatrix* rotXx = new G4RotationMatrix();
      double Angle = 90. * CLHEP::deg;
      rotXx->rotateZ(Angle);
      //place bottom plate
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamB,
                      y_tpcbeamB,
                      z_tpcbeamB
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_plate, "p_plate", &topVolume, false, 1);

      //place 1st tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamB + betpcbeam + 2. * dx_tpcbeam,
                      y_tpcbeamB - dx_plate - dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //place 2nd tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamB - betpcbeam - 2. * dx_tpcbeam,
                      y_tpcbeamB - dx_plate - dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //Top
      //rotXx = new G4RotationMatrix();
      //rotXx->rotateZ( 90. );
      //place top plate
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamT,
                      y_tpcbeamT,
                      z_tpcbeamT
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_plate, "p_plate", &topVolume, false, 1);

      //place 1st tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamT + betpcbeam + 2. * dx_tpcbeam,
                      y_tpcbeamT + dx_plate + dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //place 2nd tpc beam volume
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamT - betpcbeam - 2. * dx_tpcbeam,
                      y_tpcbeamT + dx_plate + dy_tpcbeam,
                      0.
                    );
      new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);

      //vertical beams
      //G4double dz_tpcbeamv = 1537.37 / 2.*CLHEP::mm;
      G4double dz_tpcbeamv = 1583. / 2.*CLHEP::mm;
      G4VSolid* s_tpcbeamv_a = new G4Box("s_tpcbeamv_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeamv);
      G4VSolid* s_tpcbeamv_b = new G4Box("s_tpcbeamv_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeamv);
      G4VSolid* s_tpcbeamvpos = new G4SubtractionSolid("s_tpcbeamvpos", s_tpcbeamv_a, s_tpcbeamv_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
      G4VSolid* s_tpcbeamvneg = new G4SubtractionSolid("s_tpcbeamvneg", s_tpcbeamv_a, s_tpcbeamv_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
      G4VSolid* s_tpcbeamv = new G4UnionSolid("s_tpcbeamv", s_tpcbeamvpos, s_tpcbeamvneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
      G4LogicalVolume* l_tpcbeamv = new G4LogicalVolume(s_tpcbeamv,  geometry::Materials::get("FG_Epoxy") , "l_tpcbeamv", 0, 0);

      //place 1st vertical TPC beam
      G4RotationMatrix* rotX = new G4RotationMatrix();
      rotX->rotateX(90.*CLHEP::deg);
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamL - dx_plate - dx_tpcbeam - 2.*dx_tpcbeam,
                      0.,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv", &topVolume, false, 0);
      //place 2nd vertical TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamR + dx_plate + dx_tpcbeam + 2.*dx_tpcbeam,
                      0,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv", &topVolume, false, 0);
      //place 3rd vertical TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamL - dx_plate - dx_tpcbeam - 2.*dx_tpcbeam,
                      0.,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv", &topVolume, false, 0);
      //place 4th vertical TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      x_tpcbeamR + dx_plate + dx_tpcbeam + 2.*dx_tpcbeam,
                      0.,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv", &topVolume, false, 0);

      //horizontal beams
      //G4double dz_tpcbeamh = 1792. / 2.*CLHEP::mm;
      G4double dz_tpcbeamh = 1614. / 2.*CLHEP::mm;
      G4VSolid* s_tpcbeamh_a = new G4Box("s_tpcbeamh_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeamh);
      G4VSolid* s_tpcbeamh_b = new G4Box("s_tpcbeamh_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeamh);
      G4VSolid* s_tpcbeamhpos = new G4SubtractionSolid("s_tpcbeamhpos", s_tpcbeamh_a, s_tpcbeamh_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
      G4VSolid* s_tpcbeamhneg = new G4SubtractionSolid("s_tpcbeanhneg", s_tpcbeamh_a, s_tpcbeamh_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
      G4VSolid* s_tpcbeamh = new G4UnionSolid("s_tpcbeamh", s_tpcbeamhpos, s_tpcbeamhneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
      G4LogicalVolume* l_tpcbeamh = new G4LogicalVolume(s_tpcbeamh,  geometry::Materials::get("FG_Epoxy") , "l_tpcbeamh", 0, 0);

      //place 1st horizontal TPC beam
      G4RotationMatrix* rotY = new G4RotationMatrix();
      rotY->rotateY(90.*CLHEP::deg);
      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      y_tpcbeamB - dx_plate - dy_tpcbeam - 2. * dy_tpcbeam,
                      1100.*CLHEP::mm - 2. * dy_tpcbeam
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh", &topVolume, false, 0);

      //place 2nd horizontal TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      y_tpcbeamT + dx_plate + dy_tpcbeam + 2. * dy_tpcbeam,
                      1100.*CLHEP::mm - 2. * dy_tpcbeam
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh", &topVolume, false, 0);

      //place 3rd horizontal TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      y_tpcbeamB - dx_plate - dy_tpcbeam - 2. * dy_tpcbeam,
                      -800.*CLHEP::mm + 4. * dy_tpcbeam
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh", &topVolume, false, 0);

      //place 4th horizontal TPC beam
      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      y_tpcbeamT + dx_plate + dy_tpcbeam + 2. * dy_tpcbeam,
                      -800.*CLHEP::mm + 4. * dy_tpcbeam
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh", &topVolume, false, 0);
      G4VisAttributes* brown = new G4VisAttributes(G4Colour(.5, .5, 0));
      brown->SetForceAuxEdgeVisible(true);
      l_tpcbeam->SetVisAttributes(brown);
      l_tpcbeamv->SetVisAttributes(brown);
      l_tpcbeamh->SetVisAttributes(brown);
      /*
      //Bottom beams
      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 4);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 5);

      //Top beams
      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 6);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 7);
      */
      /*
      G4double dx_20V2000 = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dy_20V2000 = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dz_20V2000 = 2200. / 2.*CLHEP::mm;
      G4double dz_20V2100v = 1537.37 / 2.*CLHEP::mm;
      G4double dz_20V2100h = 1792. / 2.*CLHEP::mm;
      G4double dz_20V2100bgov = 843.72 / 2.*CLHEP::mm;
      G4double dz_20V2100bgoh = 280.00 / 2.*CLHEP::mm;
      G4double dw_20V2000 = 2.54 * 0.25 / 2.*CLHEP::cm;
      G4VSolid* s_20V2000_a = new G4Box("s_20V2000_a", dx_20V2000, dy_20V2000, dz_20V2000);
      G4VSolid* s_20V2000_b = new G4Box("s_20V2000_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2000);
      G4VSolid* s_20V2100v_a = new G4Box("s_20V1000v_a", dx_20V2000, dy_20V2000, dz_20V2100v);
      G4VSolid* s_20V2100v_b = new G4Box("s_20V1000v_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100v);
      G4VSolid* s_20V2100bgov_a = new G4Box("s_20V1000bgov_a", dx_20V2000, dy_20V2000, dz_20V2100bgov);
      G4VSolid* s_20V2100bgov_b = new G4Box("s_20V1000bgov_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100bgov);
      G4VSolid* s_20V2100h_a = new G4Box("s_20V1000h_a", dx_20V2000, dy_20V2000, dz_20V2100h);
      G4VSolid* s_20V2100h_b = new G4Box("s_20V1000h_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100h);
      G4VSolid* s_20V2100bgoh_a = new G4Box("s_20V1000bgoh_a", dx_20V2000, dy_20V2000, dz_20V2100bgoh);
      G4VSolid* s_20V2100bgoh_b = new G4Box("s_20V1000bgoh_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100bgoh);

      G4VSolid* s_20V2000 = new G4SubtractionSolid("s_20V2000", s_20V2000_a, s_20V2000_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100vpos = new G4SubtractionSolid("s_20V2100vpos", s_20V2100v_a, s_20V2100v_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100vneg = new G4SubtractionSolid("s_20V2100vneg", s_20V2100v_a, s_20V2100v_b, 0, G4ThreeVector(0, -dw_20V2000, 0));
      G4VSolid* s_20V2100bgovpos = new G4SubtractionSolid("s_20V2100bgovpos", s_20V2100bgov_a, s_20V2100bgov_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100bgovneg = new G4SubtractionSolid("s_20V2100bgovneg", s_20V2100bgov_a, s_20V2100bgov_b, 0, G4ThreeVector(0, -dw_20V2000, 0));

      G4VSolid* s_20V2100hpos = new G4SubtractionSolid("s_20V2100hpos", s_20V2100h_a, s_20V2100h_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100hneg = new G4SubtractionSolid("s_20V2100hneg", s_20V2100h_a, s_20V2100h_b, 0, G4ThreeVector(0, -dw_20V2000, 0));
      G4VSolid* s_20V2100bgohpos = new G4SubtractionSolid("s_20V2100bgohpos", s_20V2100bgoh_a, s_20V2100bgoh_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100bgohneg = new G4SubtractionSolid("s_20V2100bgohneg", s_20V2100bgoh_a, s_20V2100bgoh_b, 0, G4ThreeVector(0, -dw_20V2000, 0));

      G4double dx_20V2300 = 2.54 * 2.38 / 2.*CLHEP::cm;
      G4double dy_20V2300 = 2.54 * 0.25 / 2.*CLHEP::cm;
      G4double dz_20V2300 = 2200. / 2.*CLHEP::mm;
      G4VSolid* s_20V2300_a = new G4Box("s_20V2300_a", dx_20V2300, dy_20V2300, dz_20V2300);
      G4VSolid* s_20V2300 = new G4UnionSolid("s_20V2300", s_20V2000, s_20V2300_a, 0, G4ThreeVector(0, -dy_20V2000 - dw_20V2000, 0));

      G4LogicalVolume* l_20V2300 = new G4LogicalVolume(s_20V2300,  geometry::Materials::get("FG_Epoxy") , "l_20V2300", 0, m_sensitive);

      //Lets limit the Geant4 stepsize inside the volume
      l_20V2300->SetUserLimits(new G4UserLimits(stepSize));

      //Beam supporting the TPC-Tube-plate
      //Right from e^-~--~beam
      G4ThreeVector PH1SUSTRpos = G4ThreeVector(
            708.72 * CLHEP::mm,
                                    136.67 * CLHEP::mm,
                                    0.
                                  );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      708.72 * CLHEP::mm,
                      -136.67 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 1);
      //Left from e^-~--~beam
      PH1SUSTRpos = G4ThreeVector(
                      -707.81 * CLHEP::mm,
                      136.67 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 2);

      PH1SUSTRpos = G4ThreeVector(
                      -707.81 * CLHEP::mm,
                      -136.67 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 3);
      //Bottom beams
      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 4);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 5);

      //Top beams
      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 6);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      //new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 7);


      G4VSolid* s_20V2100v = new G4UnionSolid("s_20V2100v", s_20V2100vpos, s_20V2100vneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100v = new G4LogicalVolume(s_20V2100v,  geometry::Materials::get("FG_Epoxy") , "l_20V2100v", 0, 0);

      G4RotationMatrix* rotX = new G4RotationMatrix();
      rotX->rotateX(90.*CLHEP::deg);
      PH1SUSTRpos = G4ThreeVector(
                      -791.3 * CLHEP::mm,
                      0.,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      791.3 * CLHEP::mm,
                      0,
                      -800.*CLHEP::mm
                    );

      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -791.3 * CLHEP::mm,
                      0.,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      791.3 * CLHEP::mm,
                      0.,
                      1100.*CLHEP::mm
                    );

      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);


      G4VSolid* s_20V2100h = new G4UnionSolid("s_20V2100h", s_20V2100hpos, s_20V2100hneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100h = new G4LogicalVolume(s_20V2100h,  geometry::Materials::get("FG_Epoxy") , "l_20V2100h", 0, 0);

      G4RotationMatrix* rotY = new G4RotationMatrix();
      rotY->rotateY(90.*CLHEP::deg);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -896.7 * CLHEP::mm,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      895.3 * CLHEP::mm,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);


      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -896.7 * CLHEP::mm,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      895.3 * CLHEP::mm,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      G4VSolid* s_20V2100bgov = new G4UnionSolid("s_20V2100bgov", s_20V2100bgovpos, s_20V2100bgovneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100bgov = new G4LogicalVolume(s_20V2100bgov,  geometry::Materials::get("FG_Epoxy") , "l_20V2100bgov", 0, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -181.3 * CLHEP::mm,
                      0.,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      181.3 * CLHEP::mm,
                      0.,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -181.3 * CLHEP::mm,
                      0.,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      181.3 * CLHEP::mm,
                      0.,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      G4VSolid* s_20V2100bgoh = new G4UnionSolid("s_20V2100bgoh", s_20V2100bgohpos, s_20V2100bgohneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100bgoh = new G4LogicalVolume(s_20V2100bgoh,  geometry::Materials::get("FG_Epoxy") , "l_20V2100bgoh", 0, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -166.3 * CLHEP::mm,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      166.3 * CLHEP::mm,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -166.3 * CLHEP::mm,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      166.3 * CLHEP::mm,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);
      */
    }
  } // ph1sustr namespace
} // Belle2 namespace
