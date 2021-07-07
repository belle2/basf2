/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/srsensor/geometry/SddCreator.h>
#include <beast/srsensor/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4UserLimits.hh>
#include "G4Tubs.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SRSENSOR detector */
  namespace srsensor {

    // Register the creator
    /** Creator creates the SDD geometry */
    geometry::CreatorFactory<SddCreator> SddFactory("SDDCreator");

    SddCreator::SddCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    SddCreator::~SddCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void SddCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar_sdd = content.getArray("bar_sdd");
      B2INFO("Contents of bar_sdd: ");
      BOOST_FOREACH(double value, bar_sdd) {
        B2INFO("value: " << value);
      }
      int sddNb = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create sdd volume
        G4double startAngle = 0.*CLHEP::deg;
        G4double spanningAngle = 360.*CLHEP::deg;
        G4Tubs* s_SDD = new G4Tubs("s_SDD",
                                   activeParams.getLength("sdd_innerRadius")*CLHEP::cm,
                                   activeParams.getLength("sdd_outerRadius")*CLHEP::cm,
                                   activeParams.getLength("sdd_hz")*CLHEP::cm,
                                   startAngle, spanningAngle);

        string matSDD = activeParams.getString("MaterialSDD");
        G4LogicalVolume* l_SDD = new G4LogicalVolume(s_SDD, geometry::Materials::get(matSDD), "l_SDD", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_SDD->SetUserLimits(new G4UserLimits(stepSize));

        //position sdd volume
        G4ThreeVector SDDpos = G4ThreeVector(
                                 activeParams.getLength("x_sdd") * CLHEP::cm,
                                 activeParams.getLength("y_sdd") * CLHEP::cm,
                                 activeParams.getLength("z_sdd") * CLHEP::cm
                               );

        G4RotationMatrix* rot_sdd = new G4RotationMatrix();
        rot_sdd->rotateX(activeParams.getAngle("AngleX"));
        rot_sdd->rotateY(activeParams.getAngle("AngleY"));
        rot_sdd->rotateZ(activeParams.getAngle("AngleZ"));
        //geometry::setColor(*l_SDD, "#006699");

        new G4PVPlacement(rot_sdd, SDDpos, l_SDD, "p_SDD", &topVolume, false, sddNb);

        sddNb++;
      }
    }
  } // srsensor namespace
} // Belle2 namespace
