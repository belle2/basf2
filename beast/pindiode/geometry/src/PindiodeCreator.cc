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
      double stepSize = content.getLength("stepSize", 5 * Unit::um);

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

        //create pindiode volume
        G4Trap* s_PINDIODE = new G4Trap("s_PINDIODE",
                                        activeParams.getLength("cDz") / 2.*Unit::mm , 0 , 0,
                                        activeParams.getLength("cDy1") / 2.*Unit::mm ,
                                        activeParams.getLength("cDx1") / 2.*Unit::mm ,
                                        activeParams.getLength("cDx2") / 2.*Unit::mm , 0,
                                        activeParams.getLength("cDy2") / 2.*Unit::mm ,
                                        activeParams.getLength("cDx3") / 2.*Unit::mm ,
                                        activeParams.getLength("cDx4") / 2.*Unit::mm , 0);


        G4LogicalVolume* l_PINDIODE = new G4LogicalVolume(s_PINDIODE, geometry::Materials::get("PINDIODE"), "l_PINDIODE", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_PINDIODE->SetUserLimits(new G4UserLimits(stepSize));

        //position pindiode volume
        G4ThreeVector PINDIODEpos = G4ThreeVector(
                                      activeParams.getLength("x_pindiode") * Unit::cm,
                                      activeParams.getLength("y_pindiode") * Unit::cm,
                                      activeParams.getLength("z_pindiode") * Unit::cm
                                    );

        G4RotationMatrix* rot_pindiode = new G4RotationMatrix();
        rot_pindiode->rotateX(activeParams.getLength("AngleX"));
        rot_pindiode->rotateY(activeParams.getLength("AngleY"));
        rot_pindiode->rotateZ(activeParams.getLength("AngleZ"));
        //geometry::setColor(*l_PINDIODE, "#006699");

        new G4PVPlacement(rot_pindiode, PINDIODEpos, l_PINDIODE, "p_PINDIODE", &topVolume, false, detID);

        detID++;
      }
    }
  } // pindiode namespace
} // Belle2 namespace
