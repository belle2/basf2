/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/srsensor/geometry/DiamondCreator.h>
#include <beast/srsensor/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SRSENSOR detector */
  namespace srsensor {

    // Register the creator
    /** Creator creates the DIAMOND geometry */
    geometry::CreatorFactory<DiamondCreator> DiamondFactory("DIAMONDCreator");

    DiamondCreator::DiamondCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    DiamondCreator::~DiamondCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void DiamondCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar_diamond = content.getArray("bar_diamond");
      B2INFO("Contents of bar_diamond: ");
      BOOST_FOREACH(double value, bar_diamond) {
        B2INFO("value: " << value);
      }
      int diamondNb = 200;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create diamond volume
        G4Box* s_DIAMOND = new G4Box("s_DIAMOND",
                                     activeParams.getLength("diamond_dx")*CLHEP::cm,
                                     activeParams.getLength("diamond_dy")*CLHEP::cm,
                                     activeParams.getLength("diamond_dz")*CLHEP::cm);

        string matDIAMOND = activeParams.getString("MaterialDIAMOND");
        G4LogicalVolume* l_DIAMOND = new G4LogicalVolume(s_DIAMOND, geometry::Materials::get(matDIAMOND), "l_DIAMOND", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_DIAMOND->SetUserLimits(new G4UserLimits(stepSize));

        //position diamond volume
        G4ThreeVector DIAMONDpos = G4ThreeVector(
                                     activeParams.getLength("x_diamond") * CLHEP::cm,
                                     activeParams.getLength("y_diamond") * CLHEP::cm,
                                     activeParams.getLength("z_diamond") * CLHEP::cm
                                   );

        G4RotationMatrix* rot_diamond = new G4RotationMatrix();
        rot_diamond->rotateX(activeParams.getAngle("AngleX"));
        rot_diamond->rotateY(activeParams.getAngle("AngleY"));
        rot_diamond->rotateZ(activeParams.getAngle("AngleZ"));
        //geometry::setColor(*l_DIAMOND, "#006699");

        new G4PVPlacement(rot_diamond, DIAMONDpos, l_DIAMOND, "p_DIAMOND", &topVolume, false, diamondNb);

        diamondNb++;
      }

    }
  } // srsensor namespace
} // Belle2 namespace
