/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/geometry/PindiodeCreator.h>
#include <beast/pindiode/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <pindiode/simulation/SensitiveDetector.h>

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

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PINDIODE detector */
  namespace pindiode {

    // Register the creator
    /** Creator creates the PINDIODE geometry */
    geometry::CreatorFactory<PindiodeCreator> PindiodeFactory("PINDIODECreator");

    PindiodeCreator::PindiodeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    PindiodeCreator::~PindiodeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void PindiodeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
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
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //Positioned PIN diodes
        double x_pos[100];
        double y_pos[100];
        double x_off[100];
        double y_off[100];
        double x_offp[100];
        double y_offp[100];
        double z_offp[100];
        double z_pos[100];
        double phi[100];
        double thetaZ[100];
        //double thetaY[100];
        double r[100];
        int dimx = 0;
        int dimy = 0;
        int dimx_offset = 0;
        int dimy_offset = 0;
        int dimx_offsetp = 0;
        int dimy_offsetp = 0;
        int dimz_offsetp = 0;
        int dimz = 0;
        int dimr_dia = 0;
        for (double x_offset : activeParams.getArray("x_offset", {0})) {
          x_offset *= CLHEP::cm;
          x_off[dimx_offset] = x_offset;
          dimx_offset++;
        }
        for (double y_offset : activeParams.getArray("y_offset", {0})) {
          y_offset *= CLHEP::cm;
          y_off[dimy_offset] = y_offset;
          dimy_offset++;
        }
        for (double x_offsetp : activeParams.getArray("x_offsetp", {0})) {
          x_offsetp *= CLHEP::cm;
          x_offp[dimx_offsetp] = x_offsetp;
          dimx_offsetp++;
        }
        for (double y_offsetp : activeParams.getArray("y_offsetp", {0})) {
          y_offsetp *= CLHEP::cm;
          y_offp[dimy_offsetp] = y_offsetp;
          dimy_offsetp++;
        }
        for (double z_offsetp : activeParams.getArray("z_offsetp", {0})) {
          z_offsetp *= CLHEP::cm;
          z_offp[dimz_offsetp] = z_offsetp;
          dimz_offsetp++;
        }
        for (double x : activeParams.getArray("x", {0})) {
          x *= CLHEP::cm;
          x_pos[dimx] = x + x_off[dimx];
          dimx++;
        }
        for (double y : activeParams.getArray("y", {0})) {
          y *= CLHEP::cm;
          y_pos[dimy] = y + y_off[dimy];
          dimy++;
        }
        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dimz] = z;
          r[dimz] = sqrt(x_pos[dimz] * x_pos[dimz] + y_pos[dimz] * y_pos[dimz]);
          dimz++;
        }
        int dimPhi = 0;
        for (double Phi : activeParams.getArray("Phi", {0})) {
          phi[dimPhi] = Phi  - 90. * CLHEP::deg;
          dimPhi++;
        }
        int dimThetaZ = 0;
        for (double ThetaZ : activeParams.getArray("ThetaZ", {0})) {
          thetaZ[dimThetaZ] = ThetaZ;
          dimThetaZ++;
        }
        /*int dimThetaY = 0;
        for (double ThetaY : activeParams.getArray("ThetaY", {0})) {
          thetaY[dimThetaY] = ThetaY;
          dimThetaY++;
          }*/
        if (dimx == 0 && dimy == 0) {
          for (double r_dia : activeParams.getArray("r_dia", {0})) {
            r_dia *= CLHEP::cm;
            r[dimr_dia] = r_dia;
            dimr_dia++;
          }
        } /*else {
    for (int i = 0; i < dimz; i++) {
      if(x_pos[i] > 0 && y_pos[i] >= 0)
        phi[i] = TMath::ATan(y_pos[i] / x_pos[i]) * TMath::RadToDeg() * CLHEP::deg - 90. * CLHEP::deg;
      else if(x_pos[i] > 0 && y_pos[i] < 0)
        phi[i] = TMath::ATan(y_pos[i] / x_pos[i]) * TMath::RadToDeg() * CLHEP::deg - 90. * CLHEP::deg + 360. * CLHEP::deg;
      else if(x_pos[i] < 0)
        phi[i] = TMath::ATan(y_pos[i] / x_pos[i]) * TMath::RadToDeg() * CLHEP::deg - 90. * CLHEP::deg + 180. * CLHEP::deg;
      else if(x_pos[i] == 0 && y_pos[i] > 0)
        phi[i] =  90. * CLHEP::deg - 90. * CLHEP::deg;
      else if(x_pos[i] == 0 && y_pos[i] < 0)
        phi[i] =  3. * 90. * CLHEP::deg - 90. * CLHEP::deg;
    }
    }*/
        //double r = activeParams.getLength("r_pindiode") * CLHEP::cm;
        //double z = activeParams.getLength("z_pindiode") * CLHEP::cm;
        //double phi = activeParams.getAngle("Phi") - 90. * CLHEP::deg;
        //double thetaZ = activeParams.getAngle("ThetaZ");

        //inch to cm
        G4double InchtoCm = 2.54 * CLHEP::cm;

        //Create PIN diode base box
        G4double dz_base = 0.5 / 2. * InchtoCm;
        G4double dx_base = 1. / 2. * InchtoCm;
        G4double dy_base = 0.25 / 2. * InchtoCm;
        G4VSolid* s_base = new G4Box("s_base", dx_base, dy_base, dz_base);

        G4double ir_hole = 0.;
        G4double or_hole = 5. / 2.*CLHEP::mm;
        G4double h_hole = 0.382 / 2. * InchtoCm;
        G4double sA_hole = 0.*CLHEP::deg;
        G4double spA_hole = 360.*CLHEP::deg;
        G4VSolid* s_hole = new G4Tubs("s_hole", ir_hole, or_hole, h_hole, sA_hole, spA_hole);
        s_base = new G4SubtractionSolid("s_base_hole1", s_base, s_hole, 0, G4ThreeVector((0.5 - 0.315)*InchtoCm,
                                        (0.187 - 0.250 / 2.)*InchtoCm, -(0.5 - 0.382)*InchtoCm));
        s_base = new G4SubtractionSolid("s_base_hole2", s_base, s_hole, 0, G4ThreeVector(-(0.5 - 0.315)*InchtoCm,
                                        (0.187 - 0.250 / 2.)*InchtoCm, -(0.5 - 0.382)*InchtoCm));

        G4LogicalVolume* l_base = new G4LogicalVolume(s_base, G4Material::GetMaterial("Al6061"), "l_base");
        l_base->SetVisAttributes(yellow);
        G4Transform3D transform;
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i],
                                    z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) * G4Translate3D(x_offp[i], y_offp[i], z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0)transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]);
            else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]);*/
          new G4PVPlacement(transform, l_base, TString::Format("p_base_%d", i).Data(), &topVolume, false, 0);
        }
        //Create diode cover
        G4double dz_cover1 = (0.563) / 2. * InchtoCm;
        G4double dx_cover1 = 1. / 2. * InchtoCm;
        G4double dy_cover1 = 1. / 16. / 2. * InchtoCm;
        G4VSolid* s_cover1 = new G4Box("s_cover1", dx_cover1, dy_cover1, dz_cover1);

        G4double dx_shole = (0.563 - 0.406) / 2. * InchtoCm;
        G4VSolid* s_shole = new G4Box("s_shole", dx_shole, dy_cover1, dx_shole);

        s_cover1 = new G4SubtractionSolid("s_cover1_hole1", s_cover1, s_shole, 0, G4ThreeVector((0.5 - 0.392)*InchtoCm + dx_shole, 0,
                                          (0.563 / 2. - 0.406)*InchtoCm + dx_shole));
        s_cover1 = new G4SubtractionSolid("s_cover1_hole2", s_cover1, s_shole, 0, G4ThreeVector(-(0.5 - 0.392)*InchtoCm - dx_shole, 0,
                                          (0.563 / 2. - 0.406)*InchtoCm + dx_shole));

        G4LogicalVolume* l_cover1 = new G4LogicalVolume(s_cover1, G4Material::GetMaterial("Al6061"), "l_cover1");
        l_cover1->SetVisAttributes(yellow);
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                                    G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.)  * G4Translate3D(x_offp[i], y_offp[i],
                                        z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0)transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
             G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
          G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.);*/
          new G4PVPlacement(transform, l_cover1, TString::Format("p_cover1_%d", i).Data(), &topVolume, false, 0);
        }

        G4double dz_cover2 = 1. / 16. / 2. * InchtoCm;
        G4double dx_cover2 = 1. / 2. * InchtoCm;
        G4double dy_cover2 = (0.315 - 1. / 16.) / 2. * InchtoCm;
        G4VSolid* s_cover2 = new G4Box("s_cover2", dx_cover2, dy_cover2, dz_cover2);
        G4LogicalVolume* l_cover2 = new G4LogicalVolume(s_cover2, G4Material::GetMaterial("Al6061"), "l_cover2");
        l_cover2->SetVisAttributes(yellow);
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                                    G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2)  * G4Translate3D(x_offp[i], y_offp[i],
                                        z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0)transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
             G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
          G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2);*/
          new G4PVPlacement(transform, l_cover2, TString::Format("p_cover2_%d", i).Data(), &topVolume, false, 0);
        }
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
        G4double dx_pin = 2.65 / 2.*CLHEP::mm;
        G4double dz_pin = 2.65 / 2.*CLHEP::mm;
        G4double dy_pin = 0.25 / 2.*CLHEP::mm;

        G4VSolid* s_pin = new G4Box("s_pin", dx_pin, dy_pin, dz_pin);
        G4LogicalVolume* l_pin = new G4LogicalVolume(s_pin, geometry::Materials::get("G4_SILICON_DIOXIDE"), "l_pin", 0, m_sensitive);
        l_pin->SetVisAttributes(yellow);
        l_pin->SetUserLimits(new G4UserLimits(stepSize));
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                                    G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin)  * G4Translate3D(x_offp[i], y_offp[i],
                                                      z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0)transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
             G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
             (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
           G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
           (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin);*/
          new G4PVPlacement(transform, l_pin, TString::Format("p_pin_1_%d", i).Data(), &topVolume, false, detID * 2);
          transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                      G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                    (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin) * G4Translate3D(x_offp[i], y_offp[i],
                                        z_offp[i]) /* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0)transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
             G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                     (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
           G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
           (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);*/
          new G4PVPlacement(transform, l_pin, TString::Format("p_pin_2_%d", i).Data(), &topVolume, false, detID * 2 + 1);
          detID++;
        }
        G4double dx_layer = 2.65 / 2.*CLHEP::mm;
        G4double dz_layer = 2.65 / 2.*CLHEP::mm;
        G4double dy_layer1 = 0.01 / 2.*CLHEP::mm;
        G4VSolid* s_layer1 = new G4Box("s_layer1", dx_layer, dy_layer1, dz_layer);
        G4LogicalVolume* l_layer1 = new G4LogicalVolume(s_layer1, geometry::Materials::get("G4_Au"), "l_layer1");
        l_layer1->SetVisAttributes(red);
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                                    G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
                                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin)  * G4Translate3D(x_offp[i], y_offp[i],
                                                      z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
              G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
                (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
           G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
           (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);*/
          new G4PVPlacement(transform, l_layer1, TString::Format("p_layer1_%d", i).Data(), &topVolume, false, 0);
        }
        G4double dy_layer2 = 0.001 / 2.*InchtoCm;
        G4VSolid* s_layer2 = new G4Box("s_layer1", dx_layer, dy_layer2, dz_layer);
        G4LogicalVolume* l_layer2 = new G4LogicalVolume(s_layer2, geometry::Materials::get("Aluminum"), "l_layer2");
        l_layer2->SetVisAttributes(green);
        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                                    G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
                                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin)  * G4Translate3D(x_offp[i], y_offp[i],
                                                      z_offp[i])/* G4RotateY3D(thetaY[i])*/;
          /*if(dimx == 0) transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
              G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
                (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
          else transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
           G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
           (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);*/
          new G4PVPlacement(transform, l_layer2, TString::Format("p_layer2_%d", i).Data(), &topVolume, false, 0);

        }
      }
    }
  } // pindiode namespace
} // Belle2 namespace
