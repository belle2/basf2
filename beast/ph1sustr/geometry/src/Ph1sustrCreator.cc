/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/ph1sustr/geometry/Ph1sustrCreator.h>
#include <beast/ph1sustr/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"

//Visualization
#include "G4Colour.hh"
#include <G4VisAttributes.hh>

using namespace std;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PH1SUSTR detector */
  namespace ph1sustr {

    // Register the creator
    /** Creator creates the phase 1 support structure geometry */
    geometry::CreatorFactory<Ph1sustrCreator> Ph1sustrFactory("PH1SUSTRCreator");

    Ph1sustrCreator::Ph1sustrCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    Ph1sustrCreator::~Ph1sustrCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1sustrCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //lets get the stepsize parameter with a default value of 5 µm
      //double stepSize = content.getLength("stepSize", 5 * CLHEP::um);


      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      for (double value : bar) {
        B2INFO("value: " << value);
      }
      /*double x_tpcbeamR = 0;
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
      double z_tpcbeamB = 0;*/
      //Lets loop over all the Active nodes
      for (const GearDir& activeParams : content.getNodes("Active")) {

        //plate positions
        double x_tpcbeamR = activeParams.getLength("x_tpcbeamR") * CLHEP::cm;
        double y_tpcbeamR = activeParams.getLength("y_tpcbeamR") * CLHEP::cm;
        double z_tpcbeamR = activeParams.getLength("z_tpcbeamR") * CLHEP::cm;
        double x_tpcbeamL = activeParams.getLength("x_tpcbeamL") * CLHEP::cm;
        double y_tpcbeamL = activeParams.getLength("y_tpcbeamL") * CLHEP::cm;
        double z_tpcbeamL = activeParams.getLength("z_tpcbeamL") * CLHEP::cm;
        double x_tpcbeamT = activeParams.getLength("x_tpcbeamT") * CLHEP::cm;
        double y_tpcbeamT = activeParams.getLength("y_tpcbeamT") * CLHEP::cm;
        double z_tpcbeamT = activeParams.getLength("z_tpcbeamT") * CLHEP::cm;
        double x_tpcbeamB = activeParams.getLength("x_tpcbeamB") * CLHEP::cm;
        double y_tpcbeamB = activeParams.getLength("y_tpcbeamB") * CLHEP::cm;
        double z_tpcbeamB = activeParams.getLength("z_tpcbeamB") * CLHEP::cm;

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
        //G4double dx_plate = 2.54 * 0.35 / 2.*CLHEP::cm;

        //G4double dx_plate_short = 0.4765 * CLHEP::cm;
        G4double dx_plate_short = 0.5 / 2. * CLHEP::cm;
        G4double dy_plate_short = 27.47788 / 2.*CLHEP::cm;
        G4double dz_plate_short = 40. / 2.*CLHEP::cm;

        G4double dx_plate =  0.5 / 2. * CLHEP::cm;
        G4double dy_plate = 32.  / 2.*CLHEP::cm;
        G4double dz_plate = 50.  / 2.*CLHEP::cm;

        //Right from e^-~--~beam
        //create plate volume
        G4VSolid* s_plate = new G4Box("s_plate", dx_plate, dy_plate, dz_plate);
        G4VSolid* s_plate_short = new G4Box("s_plate_short", dx_plate_short, dy_plate_short, dz_plate_short);

        //place plate volume
        G4LogicalVolume* l_plate = new G4LogicalVolume(s_plate,  geometry::Materials::get("Al"), "l_plate", 0, 0);
        G4LogicalVolume* l_plate_short = new G4LogicalVolume(s_plate_short,  geometry::Materials::get("Al"), "l_plate_short", 0, 0);
        G4VisAttributes* white = new G4VisAttributes(G4Colour(1, 1, 1));
        white->SetForceAuxEdgeVisible(true);
        l_plate->SetVisAttributes(white);
        l_plate_short->SetVisAttributes(white);

        G4VSolid* s_tpcbeam_a = new G4Box("s_tpcbeam_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeam);
        G4VSolid* s_tpcbeam_b = new G4Box("s_tpcbeam_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeam);
        G4VSolid* s_tpcbeampos = new G4SubtractionSolid("s_tpcbeampos", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamneg = new G4SubtractionSolid("s_tpcbeamneg", s_tpcbeam_a, s_tpcbeam_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_tpcbeam = new G4UnionSolid("s_tpcbeam", s_tpcbeampos, s_tpcbeamneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));

        //create tpc beam volumes
        G4LogicalVolume* l_tpcbeam = new G4LogicalVolume(s_tpcbeam,  geometry::Materials::get("FG_Epoxy"), "l_tpcbeam", 0, 0);

        //place plate volume
        G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                                      x_tpcbeamR,
                                      y_tpcbeamR,
                                      z_tpcbeamR
                                    );
        new G4PVPlacement(0, PH1SUSTRpos, l_plate, "p_plateR", &topVolume, false, 1);

        //offsets hori
        //G4double offset_h = /*110.*CLHEP::cm*/dz_tpcbeam + /*80.*CLHEP::cm*/ dz_plate - 2. * dz_tpcbeam;
        G4double offset_h = /*110.*CLHEP::cm*/dz_tpcbeam + 80.*CLHEP::cm - 2. * dz_tpcbeam;

        //place 1st tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamR + dx_plate + dx_tpcbeam,
                  y_tpcbeamR + betpcbeam + 2. * dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        G4Transform3D TransForm = G4Translate3D(x_tpcbeamR + dx_plate + dx_tpcbeam,
                                                y_tpcbeamR + betpcbeam + 2. * dy_tpcbeam,
                                                -offset_h / 2.) * G4RotateZ3D(90.*CLHEP::deg);
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamR1", &topVolume, false, 1);

        //place 2nd tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamR + dx_plate + dx_tpcbeam,
                  y_tpcbeamR - betpcbeam - 2. * dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamR + dx_plate + dx_tpcbeam,
                                  y_tpcbeamR - betpcbeam - 2. * dy_tpcbeam,
                                  -offset_h / 2.) * G4RotateZ3D(90.*CLHEP::deg);
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamR2", &topVolume, false, 1);

        //Left from e^-~--~beam
        //place plate volume
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamL,
                        y_tpcbeamL,
                        z_tpcbeamL
                      );
        new G4PVPlacement(0, PH1SUSTRpos, l_plate, "p_plateL", &topVolume, false, 1);

        //place 1st tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamL - dx_plate - dx_tpcbeam,
                  y_tpcbeamL + betpcbeam + 2. * dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamL - dx_plate - 3. * dx_tpcbeam,
                                  y_tpcbeamL + betpcbeam + 2. * dy_tpcbeam,
                                  -offset_h / 2.) * G4RotateZ3D(90.*CLHEP::deg);
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamL1", &topVolume, false, 1);

        //place 2nd tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamL - dx_plate - dx_tpcbeam,
                  y_tpcbeamL - betpcbeam - 2. * dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(0, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamL - dx_plate - 3. * dx_tpcbeam,
                                  y_tpcbeamL - betpcbeam - 2. * dy_tpcbeam,
                                  -offset_h / 2.) * G4RotateZ3D(90.*CLHEP::deg);
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamL2", &topVolume, false, 1);

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
        new G4PVPlacement(rotXx, PH1SUSTRpos, l_plate_short, "p_plateB", &topVolume, false, 1);

        //place 1st tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamB + betpcbeam + 2. * dx_tpcbeam,
                  y_tpcbeamB - dx_plate - dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamB + betpcbeam + 1.5 * dx_tpcbeam,
                                  y_tpcbeamB - dx_plate - 1.5 * dy_tpcbeam,
                                  -offset_h / 2.) /* G4RotateZ3D(90.*CLHEP::deg)*/;
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamB1", &topVolume, false, 1);

        //place 2nd tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamB - betpcbeam - 2. * dx_tpcbeam,
                  y_tpcbeamB - dx_plate - dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamB - betpcbeam - 1.5 * dx_tpcbeam,
                                  y_tpcbeamB - dx_plate - 1.5 * dy_tpcbeam,
                                  -offset_h / 2.) /* G4RotateZ3D(90.*CLHEP::deg)*/;
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamB2", &topVolume, false, 1);

        //Top
        //rotXx = new G4RotationMatrix();
        //rotXx->rotateZ( 90. );
        //place top plate
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamT,
                        y_tpcbeamT,
                        z_tpcbeamT
                      );
        new G4PVPlacement(rotXx, PH1SUSTRpos, l_plate, "p_plateT", &topVolume, false, 1);

        //place 1st tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamT + betpcbeam + 2. * dx_tpcbeam,
                  y_tpcbeamT + dx_plate + dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamT + betpcbeam + 1.5 * dx_tpcbeam,
                                  y_tpcbeamT + dx_plate + 3. * dy_tpcbeam,
                                  -offset_h / 2.) /* G4RotateZ3D(90.*CLHEP::deg)*/;
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamT1", &topVolume, false, 1);

        //place 2nd tpc beam volume
        /*PH1SUSTRpos = G4ThreeVector(
                  x_tpcbeamT - betpcbeam - 2. * dx_tpcbeam,
                  y_tpcbeamT + dx_plate + dy_tpcbeam,
                  -offset_h / 2.
                  );
                  new G4PVPlacement(rotXx, PH1SUSTRpos, l_tpcbeam, "p_tpcbeam", &topVolume, false, 1);*/
        TransForm = G4Translate3D(x_tpcbeamT - betpcbeam - 1.5 * dx_tpcbeam,
                                  y_tpcbeamT + dx_plate + 3 * dy_tpcbeam,
                                  -offset_h / 2.) /* G4RotateZ3D(90.*CLHEP::deg)*/;
        new G4PVPlacement(TransForm, l_tpcbeam, "p_tpcbeamT2", &topVolume, false, 1);

        //vertical beams
        //G4double dz_tpcbeamv = 1537.37 / 2.*CLHEP::mm;
        G4double dz_tpcbeamv = 1583. / 2.*CLHEP::mm;
        G4VSolid* s_tpcbeamv_a = new G4Box("s_tpcbeamv_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeamv);
        G4VSolid* s_tpcbeamv_b = new G4Box("s_tpcbeamv_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeamv);
        G4VSolid* s_tpcbeamvpos = new G4SubtractionSolid("s_tpcbeamvpos", s_tpcbeamv_a, s_tpcbeamv_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamvneg = new G4SubtractionSolid("s_tpcbeamvneg", s_tpcbeamv_a, s_tpcbeamv_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamv = new G4UnionSolid("s_tpcbeamv", s_tpcbeamvpos, s_tpcbeamvneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_tpcbeamv = new G4LogicalVolume(s_tpcbeamv,  geometry::Materials::get("FG_Epoxy"), "l_tpcbeamv", 0, 0);

        //offset verti
        //G4double offset_v = fabs(76.*CLHEP::cm - 2. * dz_tpcbeamv) / 2.;
        G4double offset_v = fabs(110.*CLHEP::cm - 2. * dz_tpcbeamv) / 2.;

        //place 1st vertical TPC beam
        G4RotationMatrix* rotX = new G4RotationMatrix();
        rotX->rotateX(90.*CLHEP::deg);
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamL - dx_plate - 3 * dx_tpcbeam - 2.*dx_tpcbeam,
                        offset_v / 2.,
                        -800.*CLHEP::mm
                      );
        new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv1", &topVolume, false, 0);
        //place 2nd vertical TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamR + dx_plate + 3.*dx_tpcbeam + 2.*dx_tpcbeam,
                        offset_v / 2.,
                        -800.*CLHEP::mm
                      );
        new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv2", &topVolume, false, 0);
        //place 3rd vertical TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamL - dx_plate - 3.*dx_tpcbeam - 2.*dx_tpcbeam,
                        offset_v / 2.,
                        1100.*CLHEP::mm
                      );
        new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv3", &topVolume, false, 0);
        //place 4th vertical TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        x_tpcbeamR + dx_plate + 3.*dx_tpcbeam + 2.*dx_tpcbeam,
                        offset_v / 2.,
                        1100.*CLHEP::mm
                      );
        new G4PVPlacement(rotX, PH1SUSTRpos, l_tpcbeamv, "p_tpcbeamv4", &topVolume, false, 0);

        //horizontal beams
        //G4double dz_tpcbeamh = 1792. / 2.*CLHEP::mm;
        G4double dz_tpcbeamh = 1614. / 2.*CLHEP::mm;
        G4VSolid* s_tpcbeamh_a = new G4Box("s_tpcbeamh_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeamh);
        G4VSolid* s_tpcbeamh_b = new G4Box("s_tpcbeamh_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeamh);
        G4VSolid* s_tpcbeamhpos = new G4SubtractionSolid("s_tpcbeamhpos", s_tpcbeamh_a, s_tpcbeamh_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamhneg = new G4SubtractionSolid("s_tpcbeanhneg", s_tpcbeamh_a, s_tpcbeamh_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamh = new G4UnionSolid("s_tpcbeamh", s_tpcbeamhpos, s_tpcbeamhneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));

        G4LogicalVolume* l_tpcbeamh = new G4LogicalVolume(s_tpcbeamh,  geometry::Materials::get("FG_Epoxy"), "l_tpcbeamh", 0, 0);

        //place 1st horizontal TPC beam
        G4RotationMatrix* rotY = new G4RotationMatrix();
        rotY->rotateY(90.*CLHEP::deg);
        PH1SUSTRpos = G4ThreeVector(
                        0 * CLHEP::mm,
                        y_tpcbeamB - dx_plate - 4. * dy_tpcbeam - 2. * dy_tpcbeam,
                        1100.*CLHEP::mm - 2. * dy_tpcbeam
                      );
        new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh1", &topVolume, false, 0);

        //place 2nd horizontal TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        0 * CLHEP::mm,
                        y_tpcbeamT + dx_plate + 5. * dy_tpcbeam + 2. * dy_tpcbeam,
                        1100.*CLHEP::mm - 2. * dy_tpcbeam
                      );
        new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh2", &topVolume, false, 0);

        //place 3rd horizontal TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        0 * CLHEP::mm,
                        y_tpcbeamB - dx_plate - 4. * dy_tpcbeam - 2. * dy_tpcbeam,
                        -800.*CLHEP::mm + 4. * dy_tpcbeam
                      );
        new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh3", &topVolume, false, 0);

        //place 4th horizontal TPC beam
        PH1SUSTRpos = G4ThreeVector(
                        0 * CLHEP::mm,
                        y_tpcbeamT + dx_plate + 5. * dy_tpcbeam + 2. * dy_tpcbeam,
                        -800.*CLHEP::mm + 4. * dy_tpcbeam
                      );
        new G4PVPlacement(rotY, PH1SUSTRpos, l_tpcbeamh, "p_tpcbeamh4", &topVolume, false, 0);

        G4VisAttributes* brown = new G4VisAttributes(G4Colour(.5, .5, 0));
        brown->SetForceAuxEdgeVisible(true);
        l_tpcbeam->SetVisAttributes(brown);
        l_tpcbeamv->SetVisAttributes(brown);
        l_tpcbeamh->SetVisAttributes(brown);

        //CsI box beams
        G4double dz_csibeamh = activeParams.getLength("lcsiBeamh") * CLHEP::cm / 2.;
        G4VSolid* s_csibeamh_a = new G4Box("s_csibeamh_a", dx_tpcbeam, dy_tpcbeam, dz_csibeamh);
        G4VSolid* s_csibeamh_b = new G4Box("s_csibeamh_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_csibeamh);
        G4VSolid* s_csibeamhpos = new G4SubtractionSolid("s_csibeamhpos", s_csibeamh_a, s_csibeamh_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_csibeamhneg = new G4SubtractionSolid("s_csibeamhneg", s_csibeamh_a, s_csibeamh_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        //G4VSolid* s_csibeamhpos = new G4SubtractionSolid("s_csibeamhpos", s_csibeamh_a, s_csibeamh_b, 0, G4ThreeVector(dw_tpcbeam, 0, 0));
        //G4VSolid* s_csibeamhneg = new G4SubtractionSolid("s_csibeamhneg", s_csibeamh_a, s_csibeamh_b, 0, G4ThreeVector(-dw_tpcbeam, 0, 0));
        G4VSolid* s_csibeamh = new G4UnionSolid("s_csibeamh", s_csibeamhpos, s_csibeamhneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_csibeamh = new G4LogicalVolume(s_csibeamh,  geometry::Materials::get("FG_Epoxy"), "l_csibeamh", 0, 0);

        double x_offset = activeParams.getLength("x_offset") * CLHEP::cm;
        double y_offset = activeParams.getLength("y_offset") * CLHEP::cm;

        std::vector<double> xcsiBeamh = activeParams.getArray("xcsiBeamh", {0});
        std::vector<double> ycsiBeamh = activeParams.getArray("ycsiBeamh", {0});
        std::vector<double> zcsiBeamh = activeParams.getArray("zcsiBeamh", {0});
        for (unsigned int i = 0; i < xcsiBeamh.size(); ++i) {
          xcsiBeamh[i] = xcsiBeamh[i] - x_offset;
          ycsiBeamh[i] = ycsiBeamh[i] - y_offset;
          G4Transform3D transform = G4Translate3D(xcsiBeamh[i], ycsiBeamh[i] - dy_tpcbeam,
                                                  zcsiBeamh[i] + 2. * dy_tpcbeam) * G4RotateY3D(90.*CLHEP::deg) * G4RotateZ3D(90.*CLHEP::deg);
          new G4PVPlacement(transform, l_csibeamh, TString::Format("p_csibeamh1_%d", i).Data(), &topVolume, false, 1);
          transform = G4Translate3D(xcsiBeamh[i], ycsiBeamh[i] + dy_tpcbeam,
                                    zcsiBeamh[i] + 2. * dy_tpcbeam) * G4RotateY3D(90.*CLHEP::deg) * G4RotateZ3D(90.*CLHEP::deg);
          new G4PVPlacement(transform, l_csibeamh, TString::Format("p_csibeamh2_%d", i).Data(), &topVolume, false, 1);
        }

        G4double dz_csibeamvF = activeParams.getLength("lcsiBeamvF") * CLHEP::cm / 2.;
        G4VSolid* s_csibeamvF_a = new G4Box("s_csibeamvF_a", dx_tpcbeam, dy_tpcbeam, dz_csibeamvF);
        G4VSolid* s_csibeamvF_b = new G4Box("s_csibeamvF_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_csibeamvF);
        G4VSolid* s_csibeamvFpos = new G4SubtractionSolid("s_csibeamvFpos", s_csibeamvF_a, s_csibeamvF_b, 0, G4ThreeVector(0, dw_tpcbeam,
                                                          0));
        G4VSolid* s_csibeamvFneg = new G4SubtractionSolid("s_csibeamvFneg", s_csibeamvF_a, s_csibeamvF_b, 0, G4ThreeVector(0, -dw_tpcbeam,
                                                          0));
        G4VSolid* s_csibeamvF = new G4UnionSolid("s_csibeamvF", s_csibeamvFpos, s_csibeamvFneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_csibeamvF = new G4LogicalVolume(s_csibeamvF,  geometry::Materials::get("FG_Epoxy"), "l_csibeamvF", 0, 0);

        std::vector<double> xcsiBeamvF =
          activeParams.getArray("xcsiBeamvF", {0});
        std::vector<double> ycsiBeamvF =
          activeParams.getArray("ycsiBeamvF", {0});
        std::vector<double> zcsiBeamvF =
          activeParams.getArray("zcsiBeamvF", {0});
        for (unsigned int i = 0; i < xcsiBeamvF.size(); ++i) {
          xcsiBeamvF[i] = xcsiBeamvF[i] - x_offset;
          ycsiBeamvF[i] = ycsiBeamvF[i] - y_offset;
          PH1SUSTRpos = G4ThreeVector(xcsiBeamvF[i], ycsiBeamvF[i], zcsiBeamvF[i]);
          new G4PVPlacement(rotX, PH1SUSTRpos, l_csibeamvF, TString::Format("p_csibeamvF_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_csibeamvB = activeParams.getLength("lcsiBeamvB") * CLHEP::cm / 2.;
        G4VSolid* s_csibeamvB_a = new G4Box("s_csibeamvB_a", dx_tpcbeam, dy_tpcbeam, dz_csibeamvB);
        G4VSolid* s_csibeamvB_b = new G4Box("s_csibeamvB_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_csibeamvB);
        G4VSolid* s_csibeamvBpos = new G4SubtractionSolid("s_csibeamvBpos", s_csibeamvB_a, s_csibeamvB_b, 0, G4ThreeVector(0, dw_tpcbeam,
                                                          0));
        G4VSolid* s_csibeamvBneg = new G4SubtractionSolid("s_csibeamvBneg", s_csibeamvB_a, s_csibeamvB_b, 0, G4ThreeVector(0, -dw_tpcbeam,
                                                          0));
        G4VSolid* s_csibeamvB = new G4UnionSolid("s_csibeamvB", s_csibeamvBpos, s_csibeamvBneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_csibeamvB = new G4LogicalVolume(s_csibeamvB,  geometry::Materials::get("FG_Epoxy"), "l_csibeamvB", 0, 0);

        std::vector<double> xcsiBeamvB =
          activeParams.getArray("xcsiBeamvB", {0});
        std::vector<double> ycsiBeamvB =
          activeParams.getArray("ycsiBeamvB", {0});
        std::vector<double> zcsiBeamvB =
          activeParams.getArray("zcsiBeamvB", {0});
        for (unsigned int i = 0; i < xcsiBeamvB.size(); ++i) {
          xcsiBeamvB[i] = xcsiBeamvB[i] - x_offset;
          ycsiBeamvB[i] = ycsiBeamvB[i] - y_offset;
          PH1SUSTRpos = G4ThreeVector(xcsiBeamvB[i], ycsiBeamvB[i], zcsiBeamvB[i]);
          new G4PVPlacement(rotX, PH1SUSTRpos, l_csibeamvB, TString::Format("p_csibeamvB_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_base = activeParams.getLength("lBase") * CLHEP::cm / 2.;
        G4VSolid* s_base_a = new G4Box("s_base_a", dx_tpcbeam, dy_tpcbeam, dz_base);
        G4VSolid* s_base_b = new G4Box("s_base_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_base);
        G4VSolid* s_base = new G4SubtractionSolid("s_base", s_base_a, s_base_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4LogicalVolume* l_base = new G4LogicalVolume(s_base,  geometry::Materials::get("FG_Epoxy"), "l_base", 0, 0);

        std::vector<double> xBase = activeParams.getArray("xBase", {0});
        std::vector<double> yBase = activeParams.getArray("yBase", {0});
        std::vector<double> zBase = activeParams.getArray("zBase", {0});
        for (unsigned int i = 0; i < xBase.size(); ++i) {
          xBase[i] = xBase[i] - x_offset;
          yBase[i] = yBase[i] - y_offset;
          PH1SUSTRpos = G4ThreeVector(xBase[i], yBase[i], zBase[i]);
          new G4PVPlacement(0, PH1SUSTRpos, l_base, TString::Format("p_base_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_bgobeamv = activeParams.getLength("lbgoBeamv") * CLHEP::cm / 2.;
        G4VSolid* s_bgobeamv_a = new G4Box("s_bgobeamv_a", dx_tpcbeam, dy_tpcbeam, dz_bgobeamv);
        G4VSolid* s_bgobeamv_b = new G4Box("s_bgobeamv_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_bgobeamv);
        G4VSolid* s_bgobeamvpos = new G4SubtractionSolid("s_bgobeamvpos", s_bgobeamv_a, s_bgobeamv_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_bgobeamvneg = new G4SubtractionSolid("s_bgobeamvneg", s_bgobeamv_a, s_bgobeamv_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_bgobeamv = new G4UnionSolid("s_bgobeamv", s_bgobeamvpos, s_bgobeamvneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_bgobeamv = new G4LogicalVolume(s_bgobeamv,  geometry::Materials::get("FG_Epoxy"), "l_bgobeamv", 0, 0);

        std::vector<double> xbgoBeamv = activeParams.getArray("xbgoBeamv", {0});
        std::vector<double> ybgoBeamv = activeParams.getArray("ybgoBeamv", {0});
        std::vector<double> zbgoBeamv = activeParams.getArray("zbgoBeamv", {0});
        for (unsigned int i = 0; i < xbgoBeamv.size(); ++i) {
          ybgoBeamv[i] = ybgoBeamv[i] - y_offset;
          PH1SUSTRpos = G4ThreeVector(xbgoBeamv[i], ybgoBeamv[i], zbgoBeamv[i]);
          new G4PVPlacement(rotX, PH1SUSTRpos, l_bgobeamv, TString::Format("p_bgobeamv_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_bgobeamh = activeParams.getLength("lbgoBeamh") * CLHEP::cm / 2.;
        G4VSolid* s_bgobeamh_a = new G4Box("s_bgobeamh_a", dx_tpcbeam, dy_tpcbeam, dz_bgobeamh);
        G4VSolid* s_bgobeamh_b = new G4Box("s_bgobeamh_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_bgobeamh);
        G4VSolid* s_bgobeamhpos = new G4SubtractionSolid("s_bgobeamhpos", s_bgobeamh_a, s_bgobeamh_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_bgobeamhneg = new G4SubtractionSolid("s_bgobeamhneg", s_bgobeamh_a, s_bgobeamh_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_bgobeamh = new G4UnionSolid("s_bgobeamh", s_bgobeamhpos, s_bgobeamhneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_bgobeamh = new G4LogicalVolume(s_bgobeamh,  geometry::Materials::get("FG_Epoxy"), "l_bgobeamh", 0, 0);

        std::vector<double> xbgoBeamh = activeParams.getArray("xbgoBeamh", {0});
        std::vector<double> ybgoBeamh = activeParams.getArray("ybgoBeamh", {0});
        std::vector<double> zbgoBeamh = activeParams.getArray("zbgoBeamh", {0});
        for (unsigned int i = 0; i < xbgoBeamh.size(); ++i) {
          ybgoBeamh[i] = ybgoBeamh[i] - y_offset;
          G4Transform3D transform = G4Translate3D(xbgoBeamh[i], ybgoBeamh[i],
                                                  zbgoBeamh[i] + 2. * dy_tpcbeam) * G4RotateY3D(90.*CLHEP::deg) * G4RotateZ3D(90.*CLHEP::deg);
          new G4PVPlacement(transform, l_bgobeamh, TString::Format("p_bgobeamh_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_bgobeamt = activeParams.getLength("lbgoBeamt") * CLHEP::cm / 2.;
        G4VSolid* s_bgobeamt_a = new G4Box("s_bgobeamt_a", dx_tpcbeam, dy_tpcbeam, dz_bgobeamt);
        G4VSolid* s_bgobeamt_b = new G4Box("s_bgobeamt_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_bgobeamt);
        G4VSolid* s_bgobeamt = new G4SubtractionSolid("s_bgobeamt", s_bgobeamt_a, s_bgobeamt_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4LogicalVolume* l_bgobeamt = new G4LogicalVolume(s_bgobeamt,  geometry::Materials::get("FG_Epoxy"), "l_bgobeamt", 0, 0);

        std::vector<double> xbgoBeamt = activeParams.getArray("xbgoBeamt", {0});
        std::vector<double> ybgoBeamt = activeParams.getArray("ybgoBeamt", {0});
        std::vector<double> zbgoBeamt = activeParams.getArray("zbgoBeamt", {0});
        for (unsigned int i = 0; i < xbgoBeamt.size(); ++i) {
          ybgoBeamt[i] = ybgoBeamt[i] - y_offset;
          PH1SUSTRpos = G4ThreeVector(xbgoBeamt[i], ybgoBeamt[i], zbgoBeamt[i]);
          new G4PVPlacement(0, PH1SUSTRpos, l_bgobeamt, TString::Format("p_bgobeamt_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_bgobeamb = activeParams.getLength("lbgoBeamb") * CLHEP::cm / 2.;
        G4VSolid* s_bgobeamb_a = new G4Box("s_bgobeamb_a", dx_tpcbeam, dy_tpcbeam, dz_bgobeamb);
        G4VSolid* s_bgobeamb_b = new G4Box("s_bgobeamb_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_bgobeamb);
        G4VSolid* s_bgobeambpos = new G4SubtractionSolid("s_bgobeambpos", s_bgobeamb_a, s_bgobeamb_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_bgobeambneg = new G4SubtractionSolid("s_bgobeambneg", s_bgobeamb_a, s_bgobeamb_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_bgobeamb = new G4UnionSolid("s_bgobeamb", s_bgobeambpos, s_bgobeambneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_bgobeamb = new G4LogicalVolume(s_bgobeamb,  geometry::Materials::get("FG_Epoxy"), "l_bgobeamb", 0, 0);
        std::vector<double> xbgoBeamb = activeParams.getArray("xbgoBeamb", {0});
        std::vector<double> ybgoBeamb = activeParams.getArray("ybgoBeamb", {0});
        std::vector<double> zbgoBeamb = activeParams.getArray("zbgoBeamb", {0});
        for (unsigned int i = 0; i < xbgoBeamb.size(); ++i) {
          xbgoBeamb[i] = xbgoBeamb[i] - x_offset;
          ybgoBeamb[i] = ybgoBeamb[i] - y_offset;
          G4Transform3D transform = G4Translate3D(xbgoBeamb[i], ybgoBeamb[i],
                                                  zbgoBeamb[i] + 2. * dy_tpcbeam) * G4RotateY3D(90.*CLHEP::deg) * G4RotateZ3D(90.*CLHEP::deg);
          new G4PVPlacement(transform, l_bgobeamb, TString::Format("p_bgobeamb_%d", i).Data(), &topVolume, false, 1);
        }

        G4double dz_tpcbeamb = activeParams.getLength("ltpcBeamb") * CLHEP::cm / 2.;
        G4VSolid* s_tpcbeamb_a = new G4Box("s_tpcbeamb_a", dx_tpcbeam, dy_tpcbeam, dz_tpcbeamb);
        G4VSolid* s_tpcbeamb_b = new G4Box("s_tpcbeamb_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_tpcbeamb);
        G4VSolid* s_tpcbeambpos = new G4SubtractionSolid("s_tpcbeambpos", s_tpcbeamb_a, s_tpcbeamb_b, 0, G4ThreeVector(0, dw_tpcbeam, 0));
        G4VSolid* s_tpcbeambneg = new G4SubtractionSolid("s_tpcbeambneg", s_tpcbeamb_a, s_tpcbeamb_b, 0, G4ThreeVector(0, -dw_tpcbeam, 0));
        G4VSolid* s_tpcbeamb = new G4UnionSolid("s_tpcbeamb", s_tpcbeambpos, s_tpcbeambneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam, 0));
        G4LogicalVolume* l_tpcbeamb = new G4LogicalVolume(s_tpcbeamb,  geometry::Materials::get("FG_Epoxy"), "l_tpcbeamb", 0, 0);
        std::vector<double> xtpcBeamb = activeParams.getArray("xtpcBeamb", {0});
        std::vector<double> ytpcBeamb = activeParams.getArray("ytpcBeamb", {0});
        std::vector<double> ztpcBeamb = activeParams.getArray("ztpcBeamb", {0});
        for (unsigned int i = 0; i < xtpcBeamb.size(); ++i) {
          xtpcBeamb[i] = xtpcBeamb[i] - x_offset;
          ytpcBeamb[i] = ytpcBeamb[i] - y_offset;
          G4Transform3D transform =
            G4Translate3D(xtpcBeamb[i], ytpcBeamb[i], ztpcBeamb[i] + 2. * dy_tpcbeam) *
            G4RotateY3D(90.*CLHEP::deg) * G4RotateZ3D(90.*CLHEP::deg);
          new G4PVPlacement(transform, l_tpcbeamb, TString::Format("p_tpcbeamb_%d", i).Data(), &topVolume, false, 1);
        }

        G4double dz_fangsbeamhf = activeParams.getLength("lfangsBeamhf") * CLHEP::cm / 2.;
        G4VSolid* s_fangsbeamhf_a = new G4Box("s_fangsbeamhf_a", dx_tpcbeam, dy_tpcbeam, dz_fangsbeamhf);
        G4VSolid* s_fangsbeamhf_b = new G4Box("s_fangsbeamhf_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_fangsbeamhf);
        G4VSolid* s_fangsbeamhf = new G4SubtractionSolid("s_fangsbeamhf", s_fangsbeamhf_a, s_fangsbeamhf_b, 0, G4ThreeVector(0, dw_tpcbeam,
                                                         0));
        G4LogicalVolume* l_fangsbeamhf = new G4LogicalVolume(s_fangsbeamhf,  geometry::Materials::get("FG_Epoxy"), "l_fangsbeamhf", 0, 0);
        double xfangsbeamhf = activeParams.getLength("xfangsBeamhf") * CLHEP::cm;
        double yfangsbeamhf = activeParams.getLength("yfangsBeamhf") * CLHEP::cm;
        double zfangsbeamhf = activeParams.getLength("zfangsBeamhf") * CLHEP::cm;
        PH1SUSTRpos = G4ThreeVector(xfangsbeamhf, yfangsbeamhf, zfangsbeamhf);
        new G4PVPlacement(rotY, PH1SUSTRpos, l_fangsbeamhf, "p_fangsbeamhf", &topVolume, false, 0);

        G4double dz_fangsbeamhb = activeParams.getLength("lfangsBeamhb") * CLHEP::cm / 2.;
        G4VSolid* s_fangsbeamhb_a = new G4Box("s_fangsbeamhb_a", dx_tpcbeam, dy_tpcbeam, dz_fangsbeamhb);
        G4VSolid* s_fangsbeamhb_b = new G4Box("s_fangsbeamhb_b", dx_tpcbeam - 2.*dw_tpcbeam, dy_tpcbeam - dw_tpcbeam, dz_fangsbeamhb);
        G4VSolid* s_fangsbeamhbpos = new G4SubtractionSolid("s_fangsbeamhbpos", s_fangsbeamhb_a, s_fangsbeamhb_b, 0, G4ThreeVector(0,
                                                            dw_tpcbeam, 0));
        G4VSolid* s_fangsbeamhbneg = new G4SubtractionSolid("s_fangsbeamhbneg", s_fangsbeamhb_a, s_fangsbeamhb_b, 0, G4ThreeVector(0,
                                                            -dw_tpcbeam, 0));
        G4VSolid* s_fangsbeamhb = new G4UnionSolid("s_fangsbeamhb", s_fangsbeamhbpos, s_fangsbeamhbneg, 0, G4ThreeVector(0, -2.*dy_tpcbeam,
                                                   0));
        G4LogicalVolume* l_fangsbeamhb = new G4LogicalVolume(s_fangsbeamhb,  geometry::Materials::get("FG_Epoxy"), "l_fangsbeamhb", 0, 0);
        double xfangsbeamhb = activeParams.getLength("xfangsBeamhb") * CLHEP::cm;
        double yfangsbeamhb = activeParams.getLength("yfangsBeamhb") * CLHEP::cm;
        double zfangsbeamhb = activeParams.getLength("zfangsBeamhb") * CLHEP::cm;
        TransForm = G4Translate3D(xfangsbeamhb, yfangsbeamhb, zfangsbeamhb) /*  G4RotateX3D(90.*CLHEP::deg)*/ * G4RotateY3D(90.*CLHEP::deg);
        new G4PVPlacement(TransForm, l_fangsbeamhb, "p_fangsbeamhb", &topVolume, false, 1);
      }
    }
  } // ph1sustr namespace
} // Belle2 namespace
