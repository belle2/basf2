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
        double r = activeParams.getLength("r_pindiode") * CLHEP::cm;
        double z = activeParams.getLength("z_pindiode") * CLHEP::cm;
        double phi = activeParams.getAngle("Phi") - 90. * CLHEP::deg;
        double thetaZ = activeParams.getAngle("ThetaZ");

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
        G4Transform3D transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_base, "p_base", &topVolume, false, 0);

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
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D(0., dy_base + dy_cover1, (dz_cover1 - dz_base) - dy_cover1 * 2.);
        new G4PVPlacement(transform, l_cover1, "p_cover1", &topVolume, false, 0);

        G4double dz_cover2 = 1. / 16. / 2. * InchtoCm;
        G4double dx_cover2 = 1. / 2. * InchtoCm;
        G4double dy_cover2 = (0.315 - 1. / 16.) / 2. * InchtoCm;
        G4VSolid* s_cover2 = new G4Box("s_cover2", dx_cover2, dy_cover2, dz_cover2);
        G4LogicalVolume* l_cover2 = new G4LogicalVolume(s_cover2, G4Material::GetMaterial("Al6061"), "l_cover2");
        l_cover2->SetVisAttributes(yellow);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D(0., -2.*dy_cover1, - dz_base - dz_cover2);
        new G4PVPlacement(transform, l_cover2, "p_cover2", &topVolume, false, 0);

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
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - 0. - dz_pin);
        new G4PVPlacement(transform, l_pin, "p_pin_1", &topVolume, false, detID * 2);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_pin,
                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
        new G4PVPlacement(transform, l_pin, "p_pin_2", &topVolume, false, detID * 2 + 1);

        G4double dx_layer = 2.65 / 2.*CLHEP::mm;
        G4double dz_layer = 2.65 / 2.*CLHEP::mm;
        G4double dy_layer1 = 0.01 / 2.*CLHEP::mm;
        G4VSolid* s_layer1 = new G4Box("s_layer1", dx_layer, dy_layer1, dz_layer);
        G4LogicalVolume* l_layer1 = new G4LogicalVolume(s_layer1, geometry::Materials::get("G4_Au"), "l_layer1");
        l_layer1->SetVisAttributes(red);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D((0.5 - 0.392) * InchtoCm + dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer1 + 2.* dy_pin,
                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
        new G4PVPlacement(transform, l_layer1, "p_layer1", &topVolume, false, 0);

        G4double dy_layer2 = 0.001 / 2.*InchtoCm;
        G4VSolid* s_layer2 = new G4Box("s_layer1", dx_layer, dy_layer2, dz_layer);
        G4LogicalVolume* l_layer2 = new G4LogicalVolume(s_layer2, geometry::Materials::get("Aluminum"), "l_layer2");
        l_layer2->SetVisAttributes(green);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ) *
                    G4Translate3D(-(0.5 - 0.392) * InchtoCm - dx_shole, (0.187 - 0.250 / 2.) * InchtoCm + dy_layer2 + 2. * dy_pin,
                                  (0.563 / 2. - 0.406) * InchtoCm + dx_shole * 2 - dz_pin);
        new G4PVPlacement(transform, l_layer2, "p_layer2", &topVolume, false, 0);

        detID++;
      }
    }
  } // pindiode namespace
} // Belle2 namespace
