/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/geometry/BeamabortCreator.h>
#include <beast/beamabort/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <beamabort/simulation/SensitiveDetector.h>

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

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the BEAMABORT detector */
  namespace beamabort {

    // Register the creator
    /** Creator creates the BEAMABORT geometry */
    geometry::CreatorFactory<BeamabortCreator> BeamabortFactory("BEAMABORTCreator");

    BeamabortCreator::BeamabortCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    BeamabortCreator::~BeamabortCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void BeamabortCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
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

        //create beamabort volumes
        G4double dx_ba = 4.5 / 2.*CLHEP::mm;
        G4double dy_ba = 4.5 / 2.*CLHEP::mm;
        G4double dz_ba = 0.5 / 2.*CLHEP::mm;
        G4Box* s_BEAMABORT = new G4Box("s_BEAMABORT", dx_ba, dy_ba, dz_ba);
        G4LogicalVolume* l_BEAMABORT = new G4LogicalVolume(s_BEAMABORT, geometry::Materials::get("Diamond"), "l_BEAMABORT", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_BEAMABORT->SetUserLimits(new G4UserLimits(stepSize));

        //position beamabort assembly
        G4ThreeVector BEAMABORTpos = G4ThreeVector(
                                       activeParams.getLength("x_beamabort") * CLHEP::cm,
                                       activeParams.getLength("y_beamabort") * CLHEP::cm,
                                       activeParams.getLength("z_beamabort") * CLHEP::cm
                                     );

        G4RotationMatrix* rot_beamabort = new G4RotationMatrix();
        rot_beamabort->rotateX(activeParams.getLength("AngleX"));
        rot_beamabort->rotateY(activeParams.getLength("AngleY"));
        rot_beamabort->rotateZ(activeParams.getLength("AngleZ"));

        new G4PVPlacement(rot_beamabort, BEAMABORTpos + BEAMABORTpos, l_BEAMABORT, "p_BEAMABORT", &topVolume, false, detID);

        detID++;
      }
    }
  } // beamabort namespace
} // Belle2 namespace
