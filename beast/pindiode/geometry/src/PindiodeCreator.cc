/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/pindiode/geometry/PindiodeCreator.h>
#include <beast/pindiode/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include "G4Tubs.hh"

//Visualization
#include "G4Colour.hh"
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PINDIODE detector */
  namespace pindiode {

    // Register the creator
    /** Creator creates the PINDIODE geometry */
    geometry::CreatorFactory<PindiodeCreator> PindiodeFactory("PINDIODECreator");

    PindiodeCreator::PindiodeCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    PindiodeCreator::~PindiodeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void PindiodeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //Visualization Attributes
      //G4VisAttributes *invis = new G4VisAttributes(G4Colour(1,1,1));
      //invis->SetColor(0,0,0,0);
      //invis->SetForceWireframe(true);
      //invis->SetVisibility(false);
      G4VisAttributes* red = new G4VisAttributes(G4Colour(1, 0, 0));
      red->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *redwire = new G4VisAttributes(G4Colour(1,0,0));
      //redwire->SetForceAuxEdgeVisible(true);
      //redwire->SetForceWireframe(true);
      G4VisAttributes* green = new G4VisAttributes(G4Colour(0, 1, 0));
      green->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *blue = new G4VisAttributes(G4Colour(0,0,1));
      //blue->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *white = new G4VisAttributes(G4Colour(1,1,1));
      //white->SetForceAuxEdgeVisible(true);
      G4VisAttributes* gray = new G4VisAttributes(G4Colour(.5, .5, .5));
      gray->SetForceAuxEdgeVisible(true);
      G4VisAttributes* yellow = new G4VisAttributes(G4Colour(1, 1, 0));
      yellow->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *cyan = new G4VisAttributes(G4Colour(0,1,1));
      //cyan->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *magenta = new G4VisAttributes(G4Colour(1,0,1));
      //magenta->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *brown = new G4VisAttributes(G4Colour(.5,.5,0));
      //brown->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *orange = new G4VisAttributes(G4Colour(1,2,0));
      //orange->SetForceAuxEdgeVisible(true);
      //G4VisAttributes* coppercolor = new G4VisAttributes(G4Colour(218. / 255., 138. / 255., 103. / 255.));
      //coppercolor->SetForceAuxEdgeVisible(true);

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);
      /*
      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      */

      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        int phase = activeParams.getInt("phase");
        G4double dx_pins = activeParams.getLength("dx_pins") / 2.*CLHEP::cm;
        G4double dy_pins = activeParams.getLength("dy_pins") / 2.*CLHEP::cm;
        G4double dz_pins = activeParams.getLength("dz_pins") / 2.*CLHEP::cm;
        //Positioned PIN diodes
        double x_pos[100];
        double y_pos[100];
        double z_pos[100];
        double thetaX[100];
        double thetaZ[100];
        double thetaY[100];
        double ch_wAu[100];
        double ch_woAu[100];
        double phi[100];
        double r[100];
        //int dimr_pin = 0;
        if (phase == 1) {
          int dimwAu = 0;
          for (int wAu : activeParams.getArray("Ch_wAu", {0})) {
            ch_wAu[dimwAu] = wAu;
            dimwAu++;
          }
          int dimwoAu = 0;
          for (int woAu : activeParams.getArray("Ch_woAu", {0})) {
            ch_woAu[dimwoAu] = woAu;
            dimwoAu++;
          }
          int dimx = 0;
          for (double x : activeParams.getArray("x", {0})) {
            x *= CLHEP::cm;
            x_pos[dimx] = x;
            dimx++;
          }
          int dimy = 0;
          for (double y : activeParams.getArray("y", {0})) {
            y *= CLHEP::cm;
            y_pos[dimy] = y;
            r[dimy] = sqrt(x_pos[dimy] * x_pos[dimy] + y_pos[dimy] * y_pos[dimy]);
            double Phi = 0;
            if (x_pos[dimy] >= 0) Phi = TMath::ASin(y_pos[dimy] / r[dimy]) * TMath::RadToDeg();
            else Phi = -TMath::ASin(y_pos[dimy] / r[dimy]) * TMath::RadToDeg() + 180.;
            //else if (x_pos[dimy] < 0) Phi = -TMath::ASin(y_pos[dimy] / r[dimy]) * TMath::RadToDeg() + 180.;
            phi[dimy] = Phi * CLHEP::deg  - 90. * CLHEP::deg;
            dimy++;
          }
          int dimThetaX = 0;
          for (double ThetaX : activeParams.getArray("ThetaX", {0})) {
            thetaX[dimThetaX] = ThetaX;
            dimThetaX++;
          }
          int dimThetaY = 0;
          for (double ThetaY : activeParams.getArray("ThetaY", {0})) {
            thetaY[dimThetaY] = ThetaY;
            dimThetaY++;
          }
        }
        int dimPhi = 0;
        if (phase == 2) {
          for (int i = 0; i < 100; i++) {
            x_pos[i] = 0;
            y_pos[i] = 0;
            z_pos[i] = 0;
          }
          for (double Phi : activeParams.getArray("Phi", {0})) {
            phi[dimPhi] = Phi  - 90. * CLHEP::deg;
            dimPhi++;
          }
          int dimr_pin = 0;
          for (double r_pin : activeParams.getArray("r_pin", {0})) {
            r_pin *= CLHEP::cm;
            r[dimr_pin] = r_pin;
            dimr_pin++;
          }
        }
        int dimz = 0;
        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dimz] = z;
          dimz++;
        }
        int dimThetaZ = 0;
        for (double ThetaZ : activeParams.getArray("ThetaZ", {0})) {
          thetaZ[dimThetaZ] = ThetaZ;
          dimThetaZ++;
        }

        //inch to cm
        G4double InchtoCm = 2.54 * CLHEP::cm;

        //Create Air Box
        G4double dz_airbox = 0.563 / 2. * InchtoCm;
        G4double dx_airbox = 1. / 2. * InchtoCm;
        G4double dy_airbox = 0.315 / 2. * InchtoCm;
        /*
        G4double dz_airbox_e = 0.1 / 2. * CLHEP::cm;
        G4double dx_airbox_e = 0.1 / 2. * CLHEP::cm;
        G4double dy_airbox_e = 0.1 / 2. * CLHEP::cm;
        G4VSolid* s_airbox = new G4Box("s_airbox", dx_airbox + dx_airbox_e, dy_airbox + dy_airbox_e, dz_airbox + dz_airbox_e);
        G4LogicalVolume* l_airbox = new G4LogicalVolume(s_airbox, G4Material::GetMaterial("G4_AIR"), "l_airbox");
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            G4RotationMatrix* pRot = new G4RotationMatrix();
            pRot->rotateX(thetaX[i]);
            pRot->rotateY(thetaY[i]);
            pRot->rotateZ(thetaZ[i]);
            new G4PVPlacement(pRot, G4ThreeVector(x_pos[i], y_pos[i], z_pos[i]), l_airbox, TString::Format("p_airbox_%d", i).Data(), &topVolume,
                              false, 0);
          } else if (phase == 2) {
            G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]);
            new G4PVPlacement(transform, l_airbox, TString::Format("p_pin_airbox_%d", i).Data(), &topVolume, false, 0);
          }
        }
        */
        //Create PIN diode base box
        G4double dz_base = 0.5 / 2. * InchtoCm;
        G4double dx_base = dx_airbox;
        G4double dy_base = 0.25 / 2. * InchtoCm;
        G4VSolid* s_base = new G4Box("s_base", dx_base, dy_base, dz_base);

        G4double ir_hole = 0.;
        G4double or_hole = 5. / 2.*CLHEP::mm;
        G4double h_hole = 0.382 / 2. * InchtoCm;
        G4double sA_hole = 0.*CLHEP::deg;
        G4double spA_hole = 360.*CLHEP::deg;
        G4VSolid* s_hole = new G4Tubs("s_hole", ir_hole, or_hole, h_hole, sA_hole, spA_hole);
        G4double x_pos_hole = dx_base - 0.315 * InchtoCm;
        G4double y_pos_hole = (0.187 - 0.250 / 2.) * InchtoCm;
        G4double z_pos_hole = -(0.5 - 0.382) * InchtoCm;
        s_base = new G4SubtractionSolid("s_base_hole1", s_base, s_hole, 0, G4ThreeVector(x_pos_hole, y_pos_hole, z_pos_hole));
        s_base = new G4SubtractionSolid("s_base_hole2", s_base, s_hole, 0, G4ThreeVector(-x_pos_hole, y_pos_hole, z_pos_hole));
        /*s_base = new G4SubtractionSolid("s_base_hole1", s_base, s_hole, 0, G4ThreeVector((0.5 - 0.315)*InchtoCm,
                                              (0.187 - 0.250 / 2.)*InchtoCm, -(0.5 - 0.382)*InchtoCm));
              s_base = new G4SubtractionSolid("s_base_hole2", s_base, s_hole, 0, G4ThreeVector(-(0.5 - 0.315)*InchtoCm,
        (0.187 - 0.250 / 2.)*InchtoCm, -(0.5 - 0.382)*InchtoCm));*/
        double iTheta[4] = {0, 90, 180, 270};
        G4LogicalVolume* l_base = new G4LogicalVolume(s_base, G4Material::GetMaterial("Al6061"), "l_base");
        l_base->SetVisAttributes(yellow);
        G4Transform3D transform;
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]);
            //if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]);
            new G4PVPlacement(transform, l_base, TString::Format("p_pin_base_%d", i).Data(), &topVolume, false, 0);
            B2INFO("PIN base-" << (int)i / 4 << "-" << iTheta[i - ((int)i / 4) * 4] << " placed at: " << transform.getTranslation() << " mm ");
          }
        }
        /*
        for (int i = 0; i < dimz; i++) {
          new G4PVPlacement(0, G4ThreeVector(0, dy_base - dy_airbox, dz_airbox - dz_base), l_base, TString::Format("p_base_%d", i).Data(),
                            l_airbox, false, 0);
        }
        */
        //Create diode cover
        G4double dz_cover1 = dz_airbox;
        G4double dx_cover1 = dx_airbox;
        G4double dy_cover1 = dy_airbox - dy_base;
        G4VSolid* s_cover1 = new G4Box("s_cover1", dx_cover1, dy_cover1, dz_cover1);

        G4double dx_shole = (0.563 - 0.406) / 2. * InchtoCm;
        G4VSolid* s_shole = new G4Box("s_shole", dx_shole, dy_cover1, dx_shole);
        G4double x_pos_cover_hole = dx_base - 0.392 * InchtoCm + dx_shole;
        //G4double y_pos_cover_hole = 0;
        G4double z_pos_cover_hole = dz_airbox - 0.406 * InchtoCm + dx_shole;
        s_cover1 = new G4SubtractionSolid("s_cover1_hole1", s_cover1, s_shole, 0, G4ThreeVector(x_pos_cover_hole, 0, z_pos_cover_hole));
        s_cover1 = new G4SubtractionSolid("s_cover1_hole2", s_cover1, s_shole, 0, G4ThreeVector(-x_pos_cover_hole, 0, z_pos_cover_hole));
        /*
              s_cover1 = new G4SubtractionSolid("s_cover1_hole1", s_cover1, s_shole, 0, G4ThreeVector((0.5 - 0.392)*InchtoCm + dx_shole, 0,
                                                (0.563 / 2. - 0.406)*InchtoCm + dx_shole));
              s_cover1 = new G4SubtractionSolid("s_cover1_hole2", s_cover1, s_shole, 0, G4ThreeVector(-(0.5 - 0.392)*InchtoCm - dx_shole, 0,
                                                (0.563 / 2. - 0.406)*InchtoCm + dx_shole));
        */
        G4LogicalVolume* l_cover1 = new G4LogicalVolume(s_cover1, G4Material::GetMaterial("Al6061"), "l_cover1");
        l_cover1->SetVisAttributes(yellow);
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.);
            //if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
            //                            G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.);
            new G4PVPlacement(transform, l_cover1, TString::Format("p_pin_cover1_%d", i).Data(), &topVolume, false, 0);
          }
        }
        /*
        for (int i = 0; i < dimz; i++) {
                new G4PVPlacement(0,  G4ThreeVector(0, dy_airbox_e / 2. + dy_airbox - dy_cover1,  0), l_cover1, TString::Format("p_cover1_%d",
                                  i).Data(), l_airbox,
                                  false, 0);
              }
        */
        G4double dz_cover2 = dz_airbox - dz_base;
        G4double dx_cover2 = dx_airbox;
        G4double dy_cover2 = dy_base;
        G4VSolid* s_cover2 = new G4Box("s_cover2", dx_cover2, dy_cover2, dz_cover2);
        G4LogicalVolume* l_cover2 = new G4LogicalVolume(s_cover2, G4Material::GetMaterial("Al6061"), "l_cover2");
        l_cover2->SetVisAttributes(yellow);
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2);
            //if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
            //G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2);
            new G4PVPlacement(transform, l_cover2, TString::Format("p_pin_cover2_%d", i).Data(), &topVolume, false, 0);
          }
        }
        /*
        for (int i = 0; i < dimz; i++) {
                new G4PVPlacement(0, G4ThreeVector(0, dy_cover2 - dy_airbox, dz_cover2 - dz_airbox - dz_airbox_e / 2.), l_cover2,
                                  TString::Format("p_cover2_%d",
                                                  i).Data(), l_airbox, false, 0);
              }
        */
        //Create PIN plastic subtrate
        /*
        h_hole = 0.6 / 2.*CLHEP::cm;
        sA_hole = 0.*deg;
        spA_hole = 180.*deg;
        G4VSolid* s_pinsubtrate = new G4Tubs("s_pinsubtrate", ir_hole, or_hole, h_hole, sA_hole, spA_hole);
        G4LogicalVolume* l_pinsubtrate = new G4LogicalVolume(s_pinsubtrate, geometry::Materials::get("G4_POLYSTYRENE"), "l_pinsubtrate");
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D((0.5 - 0.315) * InchtoCm, (0.187 - 0.250 / 2.) * InchtoCm, -(0.5 - 0.382) * InchtoCm) * G4RotateZ3D(360.0);
        new G4PVPlacement(transform, l_pinsubtrate, "p_pinsubtrate_1", &topVolume, false, 0);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D(-(0.5 - 0.315) * InchtoCm, (0.187 - 0.250 / 2.) * InchtoCm, -(0.5 - 0.382) * InchtoCm) * G4RotateZ3D(360.0);
        new G4PVPlacement(transform, l_pinsubtrate, "p_pinsubtrate_2", &topVolume, false, 0);
        */

        //Sensitive area
        G4double dx_pin = dx_pins;
        G4double dz_pin = dz_pins;
        G4double dy_pin = dy_pins;

        G4VSolid* s_pin = new G4Box("s_pin", dx_pin, dy_pin, dz_pin);
        G4LogicalVolume* l_pin = new G4LogicalVolume(s_pin, geometry::Materials::get("G4_SILICON_DIOXIDE"), "l_pin", 0, m_sensitive);
        l_pin->SetVisAttributes(yellow);
        l_pin->SetUserLimits(new G4UserLimits(stepSize));
        int detID = 0;
        for (int i = 0; i < dimz; i++) {
          int detID1 = 2 * i;
          int detID2 = 2 * i + 1;
          /*if (phase == 1) {
            detID1 = ch_wAu[i];
            detID2 = ch_woAu[i];
          }*/
          if (phase == 1) {

            detID1 = ch_wAu[i];
            detID2 = ch_woAu[i];

            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                      (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin);
            /*
              if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
              G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
              (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin);
            */
            new G4PVPlacement(transform, l_pin, TString::Format("p_pin_1_%d", i).Data(), &topVolume, false, detID1);
            B2INFO("With Au PIN-" << detID1 << " placed at: " << transform.getTranslation() << " mm");

            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                      (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            new G4PVPlacement(transform, l_pin, TString::Format("p_pin_2_%d", i).Data(), &topVolume, false, detID2);
            B2INFO("        PIN-" << detID2 << " placed at: " << transform.getTranslation() << " mm");
          }
          /*if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                      (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            new G4PVPlacement(transform, l_pin, TString::Format("p_pin_2_%d", i).Data(), &topVolume, false, detID2);
            B2INFO("        PIN-" << detID2 << " placed at: " << transform.getTranslation() << " mm");
          }*/
          /*
            if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
            G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
            (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
          */

          if (phase == 2) {
            for (int j = 0; j < dimPhi; j++) {
              transform = G4RotateZ3D(phi[j]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[j]);
              new G4PVPlacement(transform, l_pin, TString::Format("p_pin_%d", i).Data(), &topVolume, false, detID);
              B2INFO("PIN-" << detID << " placed at: " << transform.getTranslation() << " mm");
              detID ++;
            }
          }
        }
        /*
              for (int i = 0; i < dimz; i++) {

        int detID1 = 2 * i;
                int detID2 = 2 * i + 1;
                if (phase == 1) {
                  detID1 = ch_wAu[i];
                  detID2 = ch_woAu[i];
                }

                      new G4PVPlacement(0, G4ThreeVector((0.5 - 0.392) * InchtoCm + dx_shole,
                                                         (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                                         (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin)
                                        , l_pin, TString::Format("p_pin_1_%d", i).Data(), l_airbox, false, detID1);
                      new G4PVPlacement(0,  G4ThreeVector(-(0.5 - 0.392) * InchtoCm - dx_shole,
                                                          (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                                          (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin)
                                        , l_pin, TString::Format("p_pin_2_%d", i).Data(), l_airbox, false, detID2);

                new G4PVPlacement(0, G4ThreeVector(x_pos_cover_hole,
                                                   y_pos_hole + dy_pin,
                                                   z_pos_cover_hole + dx_shole - dz_pin)
                                  , l_pin, TString::Format("p_pin_1_%d", i).Data(), l_airbox, false, detID1);
                new G4PVPlacement(0,  G4ThreeVector(-x_pos_cover_hole,
                                                    y_pos_hole + dy_pin,
                                                    z_pos_cover_hole + dx_shole - dz_pin)
                                  , l_pin, TString::Format("p_pin_2_%d", i).Data(), l_airbox, false, detID2);
              }
        */
        G4double dx_layer = 2.65 / 2.*CLHEP::mm;
        G4double dz_layer = 2.65 / 2.*CLHEP::mm;
        G4double dy_layer1 = 0.01 / 2.*CLHEP::mm;
        G4VSolid* s_layer1 = new G4Box("s_layer1", dx_layer, dy_layer1, dz_layer);
        G4LogicalVolume* l_layer1 = new G4LogicalVolume(s_layer1, geometry::Materials::get("G4_Au"), "l_layer1");
        l_layer1->SetVisAttributes(red);
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
                                      (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            //if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
            //G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
            //(0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            new G4PVPlacement(transform, l_layer1, TString::Format("p_pin_layer1_%d", i).Data(), &topVolume, false, 0);
          }
        }
        /*
        for (int i = 0; i < dimz; i++) {

                      new G4PVPlacement(0, G4ThreeVector((0.5 - 0.392) * InchtoCm + dx_shole,
                                                         (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
                                                         (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin),
                                        l_layer1, TString::Format("p_layer1_%d", i).Data(), l_airbox, false, 0);

                new G4PVPlacement(0, G4ThreeVector(x_pos_cover_hole,
                                                   y_pos_hole + dy_layer1 + 2.* dy_pin,
                                                   z_pos_cover_hole + dx_shole - dz_pin),
                                  l_layer1, TString::Format("p_layer1_%d", i).Data(), l_airbox, false, 1);
              }
        */
        G4double dy_layer2 = 0.001 / 2.*InchtoCm;
        G4VSolid* s_layer2 = new G4Box("s_layer1", dx_layer, dy_layer2, dz_layer);
        G4LogicalVolume* l_layer2 = new G4LogicalVolume(s_layer2, geometry::Materials::get("Al"), "l_layer2");
        l_layer2->SetVisAttributes(green);
        for (int i = 0; i < dimz; i++) {
          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]) *
                        G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
                                      (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            //if (phase == 2) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
            //G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
            //(0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
            new G4PVPlacement(transform, l_layer2, TString::Format("p_pin_layer2_%d", i).Data(), &topVolume, false, 0);
          }
        }
        /*
        for (int i = 0; i < dimz; i++) {

                      new G4PVPlacement(0, G4ThreeVector(-(0.5 - 0.392) * InchtoCm - dx_shole,
                                                         (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
                                                         (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin),
                                        l_layer2, TString::Format("p_layer2_%d", i).Data(), l_airbox, false, 0);

                new G4PVPlacement(0, G4ThreeVector(-x_pos_cover_hole,
                                                   y_pos_hole + dy_layer2 + 2. * dy_pin,
                                                   z_pos_cover_hole + dx_shole - dz_pin),
                                  l_layer2, TString::Format("p_layer2_%d", i).Data(), l_airbox, false, 1);
              }
        */
      }
    }
  } // pindiode namespace
} // Belle2 namespace
