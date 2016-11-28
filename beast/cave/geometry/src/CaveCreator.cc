/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/cave/geometry/CaveCreator.h>
#include <beast/cave/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <cave/simulation/SensitiveDetector.h>

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

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CAVE detector */
  namespace cave {

    // Register the creator
    /** Creator creates the cave geometry */
    geometry::CreatorFactory<CaveCreator> CaveFactory("CAVECreator");

    CaveCreator::CaveCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    CaveCreator::~CaveCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void CaveCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
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

        //create cave volume
        G4Box* s_CAVE = new G4Box("s_CAVE",
                                  activeParams.getLength("px")*CLHEP::cm ,
                                  activeParams.getLength("py")*CLHEP::cm ,
                                  activeParams.getLength("pz")*CLHEP::cm);

        //G4LogicalVolume* l_CAVE = new G4LogicalVolume(s_CAVE, geometry::Materials::get("CAVE"), "l_CAVE", 0, m_sensitive);
        G4LogicalVolume* l_CAVE = new G4LogicalVolume(s_CAVE, geometry::Materials::get("CAVE"), "l_CAVE", 0, 0);

        //Lets limit the Geant4 stepsize inside the volume
        l_CAVE->SetUserLimits(new G4UserLimits(stepSize));

        //position cave volume
        G4ThreeVector CAVEpos = G4ThreeVector(
                                  activeParams.getLength("x_cave") * CLHEP::cm,
                                  activeParams.getLength("y_cave") * CLHEP::cm,
                                  activeParams.getLength("z_cave") * CLHEP::cm
                                );

        G4RotationMatrix* rot_cave = new G4RotationMatrix();
        rot_cave->rotateX(activeParams.getLength("AngleX"));
        rot_cave->rotateY(activeParams.getLength("AngleY"));
        rot_cave->rotateZ(activeParams.getLength("AngleZ"));
        //geometry::setColor(*l_CAVE, "#006699");
        //double angle = activeParams.getDouble("angle");
        //double rx = activeParams.getDouble("rx");
        //double ry = activeParams.getDouble("ry");
        //double rz = activeParams.getDouble("rz");
        //rot_cave->rotate(-angle, G4ThreeVector(rx, ry, rz));
        new G4PVPlacement(rot_cave, CAVEpos, l_CAVE, "p_CAVE", &topVolume, false, detID);

        detID++;
      }
    }
  } // cave namespace
} // Belle2 namespace
