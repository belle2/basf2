/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <awesome/geometry/AwesomeCreator.h>
#include <awesome/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <awesome/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the AWESOME detector */
  namespace awesome {

    // Register the creator
    geometry::CreatorFactory<AwesomeCreator> AwesomeFactory("AWESOMECreator");

    AwesomeCreator::AwesomeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    AwesomeCreator::~AwesomeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void AwesomeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
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

      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
        //Lets create a Box with dimensions taken from the parameters
        //WARNING: Geant4 is in mm so you have to convert the parameters
        G4Box* activeShape = new G4Box(
          "ActiveBox",
          activeParams.getLength("width") / Unit::mm / 2.0,
          activeParams.getLength("length") / Unit::mm / 2.0,
          activeParams.getLength("height") / Unit::mm / 2.0
        );

        string material = activeParams.getString("Material");
        G4LogicalVolume* active = new G4LogicalVolume(activeShape,  geometry::Materials::get(material),
                                                      "ActiveVolume", 0, m_sensitive);
        //setColor is an utility function which allows to easily set the color for visualization
        geometry::setColor(*active, "#006699");
        //Lets limit the Geant4 stepsize inside the volume
        active->SetUserLimits(new G4UserLimits(stepSize));

        //And finally place the volume
        new G4PVPlacement(G4Translate3D(0, 0, activeParams.getLength("z") / Unit::mm),
                          active, "ActivePlacement", &topVolume, false, 1);
      }
    }
  } // awesome namespace
} // Belle2 namespace
