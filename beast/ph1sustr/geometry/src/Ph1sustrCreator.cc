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

        //create ph1sustr volume
        G4Trap* s_PH1SUSTR = new G4Trap("s_PH1SUSTR",
                                        activeParams.getLength("cDz") / 2.*CLHEP::mm , 0 , 0,
                                        activeParams.getLength("cDy1") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx1") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx2") / 2.*CLHEP::mm , 0,
                                        activeParams.getLength("cDy2") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx3") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx4") / 2.*CLHEP::mm , 0);


        G4LogicalVolume* l_PH1SUSTR = new G4LogicalVolume(s_PH1SUSTR, geometry::Materials::get("PH1SUSTR"), "l_PH1SUSTR", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_PH1SUSTR->SetUserLimits(new G4UserLimits(stepSize));

        //position ph1sustr volume
        G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                                      activeParams.getLength("x_ph1sustr") * CLHEP::cm,
                                      activeParams.getLength("y_ph1sustr") * CLHEP::cm,
                                      activeParams.getLength("z_ph1sustr") * CLHEP::cm
                                    );

        G4RotationMatrix* rot_ph1sustr = new G4RotationMatrix();
        rot_ph1sustr->rotateX(activeParams.getLength("AngleX"));
        rot_ph1sustr->rotateY(activeParams.getLength("AngleY"));
        rot_ph1sustr->rotateZ(activeParams.getLength("AngleZ"));
        //geometry::setColor(*l_PH1SUSTR, "#006699");

        new G4PVPlacement(rot_ph1sustr, PH1SUSTRpos, l_PH1SUSTR, "p_PH1SUSTR", &topVolume, false, detID);

        detID++;
      }
    }
  } // ph1sustr namespace
} // Belle2 namespace
