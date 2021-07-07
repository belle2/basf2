/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/srsensor/geometry/Fei4Creator.h>
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
    /** Creator creates the FEI4 geometry */
    geometry::CreatorFactory<Fei4Creator> Fei4Factory("FEI4Creator");

    Fei4Creator::Fei4Creator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    Fei4Creator::~Fei4Creator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Fei4Creator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar_fei4 = content.getArray("bar_fei4");
      B2INFO("Contents of bar_fei4: ");
      BOOST_FOREACH(double value, bar_fei4) {
        B2INFO("value: " << value);
      }
      int fei4Nb = 100;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create fei4 volume
        G4Box* s_FEI4 = new G4Box("s_FEI4",
                                  activeParams.getLength("fei4_dx")*CLHEP::cm,
                                  activeParams.getLength("fei4_dy")*CLHEP::cm,
                                  activeParams.getLength("fei4_dz")*CLHEP::cm);

        string matFEI4 = activeParams.getString("MaterialFEI4");
        G4LogicalVolume* l_FEI4 = new G4LogicalVolume(s_FEI4, geometry::Materials::get(matFEI4), "l_FEI4", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_FEI4->SetUserLimits(new G4UserLimits(stepSize));

        //position fei4 volume
        G4ThreeVector FEI4pos = G4ThreeVector(
                                  activeParams.getLength("x_fei4") * CLHEP::cm,
                                  activeParams.getLength("y_fei4") * CLHEP::cm,
                                  activeParams.getLength("z_fei4") * CLHEP::cm
                                );

        G4RotationMatrix* rot_fei4 = new G4RotationMatrix();
        rot_fei4->rotateX(activeParams.getAngle("AngleX"));
        rot_fei4->rotateY(activeParams.getAngle("AngleY"));
        rot_fei4->rotateZ(activeParams.getAngle("AngleZ"));
        //geometry::setColor(*l_FEI4, "#006699");

        new G4PVPlacement(rot_fei4, FEI4pos, l_FEI4, "p_FEI4", &topVolume, false, fei4Nb);

        fei4Nb++;
      }

    }
  } // srsensor namespace
} // Belle2 namespace
