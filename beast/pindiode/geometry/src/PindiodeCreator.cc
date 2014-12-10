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
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create pindiode container volume
        G4double dx_box = 2. / 2.*CLHEP::cm;
        G4double dy_box = 1. / 2.*CLHEP::cm;
        G4double dz_box = 1. / 2.*CLHEP::cm;
        G4VSolid* s_Box = new G4Box("s_box", dx_box, dy_box, dz_box);

        G4double dx_hole = (20. - 2.*2.05) / 2.*CLHEP::mm;
        G4double dy_hole = (10. - 2.*2.05) / 2.*CLHEP::mm;
        G4double dz_hole = 5.1 / 2.*CLHEP::mm;
        G4VSolid* s_Hole = new G4Box("s_Hole", dx_hole, dy_hole, dz_hole);
        s_Box = new G4SubtractionSolid("s_Box", s_Box, s_Hole, 0, G4ThreeVector(0, 0, dz_box - dz_hole));

        G4LogicalVolume* l_Box = new G4LogicalVolume(s_Box, geometry::Materials::get("Al6061"), "l_Box");

        //create pindiode volumes
        G4double dx_diode = 2.65 / 2.*CLHEP::mm;
        G4double dy_diode = 2.65 / 2.*CLHEP::mm;
        G4double dz_diode = 0.25 / 2.*CLHEP::mm;
        G4Box* s_PINDIODE1 = new G4Box("s_PINDIODE1", dx_diode, dy_diode, dz_diode);
        G4Box* s_PINDIODE2 = new G4Box("s_PINDIODE2", dx_diode, dy_diode, dz_diode);
        G4double dz_layer = 0.01 / 2.*CLHEP::mm;
        G4Box* s_layer = new G4Box("s_layer", dx_diode, dy_diode, dz_layer);

        G4LogicalVolume* l_PINDIODE1 = new G4LogicalVolume(s_PINDIODE1, geometry::Materials::get("G4_SILICON_DIOXIDE"), "l_PINDIODE1", 0, m_sensitive);
        G4LogicalVolume* l_PINDIODE2 = new G4LogicalVolume(s_PINDIODE2, geometry::Materials::get("G4_SILICON_DIOXIDE"), "l_PINDIODE2", 0, m_sensitive);
        G4LogicalVolume* l_layer = new G4LogicalVolume(s_layer,  geometry::Materials::get("G4_Au"), "l_layer");

        //Lets limit the Geant4 stepsize inside the volume
        l_PINDIODE1->SetUserLimits(new G4UserLimits(stepSize));
        l_PINDIODE2->SetUserLimits(new G4UserLimits(stepSize));

        //position pindiode assembly
        G4ThreeVector PINDIODEpos = G4ThreeVector(
                                      activeParams.getLength("x_pindiode") * CLHEP::cm,
                                      activeParams.getLength("y_pindiode") * CLHEP::cm,
                                      activeParams.getLength("z_pindiode") * CLHEP::cm
                                    );
        //pindiode 1 position
        G4ThreeVector PINDIODEpos1 = G4ThreeVector(2.*  dx_diode, 0, 0);
        //pindiode gold layer 1 position
        G4ThreeVector Layerpos = G4ThreeVector(2. * dx_diode, 0, dz_diode + dz_layer);
        //pindiode 2 position
        G4ThreeVector PINDIODEpos2 = G4ThreeVector(-2. * dx_diode, 0, 0);


        G4RotationMatrix* rot_pindiode = new G4RotationMatrix();
        rot_pindiode->rotateX(activeParams.getAngle("AngleX"));
        rot_pindiode->rotateY(activeParams.getAngle("AngleY"));
        rot_pindiode->rotateZ(activeParams.getAngle("AngleZ"));
        //geometry::setColor(*l_PINDIODE, "#006699");


        new G4PVPlacement(rot_pindiode, PINDIODEpos, l_Box, "p_Box", &topVolume, false, 1);
        new G4PVPlacement(rot_pindiode, PINDIODEpos + Layerpos, l_layer, "p_layer", &topVolume, false, 1);
        new G4PVPlacement(rot_pindiode, PINDIODEpos + PINDIODEpos1, l_PINDIODE1, "p_PINDIODE1", &topVolume, false, detID);
        new G4PVPlacement(rot_pindiode, PINDIODEpos + PINDIODEpos2, l_PINDIODE2, "p_PINDIODE2", &topVolume, false, detID++);

        detID++;
      }
    }
  } // pindiode namespace
} // Belle2 namespace
