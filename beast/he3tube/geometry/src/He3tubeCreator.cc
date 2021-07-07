/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/he3tube/geometry/He3tubeCreator.h>
#include <beast/he3tube/simulation/SensitiveDetector.h>

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
#include <G4UserLimits.hh>
#include "G4Tubs.hh"

//Visualization Attributes
#include <G4VisAttributes.hh>


using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the HE3TUBE detector */
  namespace he3tube {

    // Register the creator
    /** Creator creates the He-3 tube geometry */
    geometry::CreatorFactory<He3tubeCreator> He3tubeFactory("HE3TUBECreator");

    He3tubeCreator::He3tubeCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    He3tubeCreator::~He3tubeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void He3tubeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      G4String symbol;
      G4double a, z, n;
      G4double abundance, density;
      G4int ncomponents;

      G4Isotope* iHe3 = new G4Isotope("iHe3", z = 2, n = 3, a = 3.0160293 * CLHEP::g / CLHEP::mole);
      G4Element* eHe3 = new G4Element("eHe3", symbol = "eHe3", ncomponents = 1);
      eHe3->AddIsotope(iHe3, abundance = 100.);
      G4Material* gHe3 = new G4Material("gHe3", density = 0.00066 * CLHEP::g / CLHEP::cm3, ncomponents = 1);
      gHe3->AddElement(eHe3, 1);

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create he3tube
        G4double startAngle = 0.*CLHEP::deg;
        G4double spanningAngle = 360.*CLHEP::deg;
        G4Tubs* s_He3TUBE = new G4Tubs("s_He3TUBE",
                                       activeParams.getLength("tube_innerRadius")*CLHEP::cm,
                                       activeParams.getLength("tube_outerRadius")*CLHEP::cm,
                                       activeParams.getLength("tube_hz")*CLHEP::cm,
                                       startAngle, spanningAngle);

        string matTube = activeParams.getString("MaterialTube");
        G4LogicalVolume* l_He3TUBE = new G4LogicalVolume(s_He3TUBE, geometry::Materials::get(matTube), "l_He3TUBE");

        G4VisAttributes* l_He3TubeVisAtt = new G4VisAttributes(G4Colour(0, 1.0, 1.0, 1.0));
        l_He3TUBE->SetVisAttributes(l_He3TubeVisAtt);

        //position he3tube volume
        G4ThreeVector He3TUBEpos = G4ThreeVector(
                                     activeParams.getLength("x_he3tube") * CLHEP::cm,
                                     activeParams.getLength("y_he3tube") * CLHEP::cm,
                                     activeParams.getLength("z_he3tube") * CLHEP::cm
                                   );

        G4RotationMatrix* rot_he3tube = new G4RotationMatrix();
        rot_he3tube->rotateX(activeParams.getAngle("AngleX"));
        rot_he3tube->rotateY(activeParams.getAngle("AngleY"));
        rot_he3tube->rotateZ(activeParams.getAngle("AngleZ"));

        //geometry::setColor(*l_HE3TUBE, "#006699");

        new G4PVPlacement(rot_he3tube, He3TUBEpos, l_He3TUBE, "p_He3TUBE", &topVolume, false, 1);

        B2INFO("HE3-tube-" << detID << " placed at: " <<  He3TUBEpos << " mm");

        //create endcaps
        G4double EndcapinnerRadius = 0.;
        G4Tubs* s_He3endcap = new G4Tubs("s_He3endcap",
                                         EndcapinnerRadius,
                                         activeParams.getLength("tube_outerRadius")*CLHEP::cm,
                                         activeParams.getLength("endcap_hz")*CLHEP::cm,
                                         startAngle, spanningAngle);

        G4LogicalVolume* l_He3endcap = new G4LogicalVolume(s_He3endcap, geometry::Materials::get(matTube), "l_He3endcap");

        l_He3endcap->SetVisAttributes(l_He3TubeVisAtt);

        //position endcaps
        G4ThreeVector He3endcapposTop = G4ThreeVector(
                                          activeParams.getLength("x_he3tube") * CLHEP::cm,
                                          activeParams.getLength("y_he3tube") * CLHEP::cm,
                                          activeParams.getLength("z_he3tube") * CLHEP::cm + activeParams.getLength("tube_hz") * CLHEP::cm +
                                          activeParams.getLength("endcap_hz") * CLHEP::cm
                                        );

        G4ThreeVector He3endcapposBot = G4ThreeVector(
                                          activeParams.getLength("x_he3tube") * CLHEP::cm,
                                          activeParams.getLength("y_he3tube") * CLHEP::cm,
                                          activeParams.getLength("z_he3tube") * CLHEP::cm - activeParams.getLength("tube_hz") * CLHEP::cm -
                                          activeParams.getLength("endcap_hz") * CLHEP::cm
                                        );

        new G4PVPlacement(rot_he3tube, He3endcapposTop, l_He3endcap, "p_He3endcapTop", &topVolume, false, 1);
        new G4PVPlacement(rot_he3tube, He3endcapposBot, l_He3endcap, "p_He3endcapBot", &topVolume, false, 1);

        //create he3 inactif gas
        G4double GasinnerRadius = 0.;
        G4Tubs* s_iHe3Gas = new G4Tubs("s_iHe3Gas",
                                       GasinnerRadius,
                                       activeParams.getLength("tube_innerRadius")*CLHEP::cm,
                                       activeParams.getLength("tube_hz")*CLHEP::cm,
                                       startAngle, spanningAngle);

        //string matGas = activeParams.getString("MaterialGas");
        G4LogicalVolume* l_iHe3Gas = new G4LogicalVolume(s_iHe3Gas, gHe3, "l_iHe3Gas");
        l_iHe3Gas->SetVisAttributes(l_He3TubeVisAtt);

        new G4PVPlacement(rot_he3tube, He3TUBEpos, l_iHe3Gas, "p_iHe3Gas", &topVolume, false, 1);

        //create he3 actif gas
        G4Tubs* s_He3Gas = new G4Tubs("s_He3Gas",
                                      GasinnerRadius,
                                      activeParams.getLength("gas_outerRadius")*CLHEP::cm,
                                      activeParams.getLength("gas_hz")*CLHEP::cm,
                                      startAngle, spanningAngle);

        G4LogicalVolume* l_He3Gas = new G4LogicalVolume(s_He3Gas, gHe3, "l_He3Gas", 0, m_sensitive);
        l_He3Gas->SetVisAttributes(l_He3TubeVisAtt);
        //Lets limit the Geant4 stepsize inside the volume
        l_He3Gas->SetUserLimits(new G4UserLimits(stepSize));

        new G4PVPlacement(0, G4ThreeVector(0, 0, activeParams.getLength("SV_offset_inZ")*CLHEP::cm), l_He3Gas, "p_He3Gas", l_iHe3Gas, false,
                          detID);
        B2INFO("HE3-tube-Sensitive-Volume-" << detID << " placed at: (" <<  He3TUBEpos.getX() << "," << He3TUBEpos.getY() << "," <<
               He3TUBEpos.getZ() + activeParams.getLength("SV_offset_inZ")*CLHEP::cm << ")  mm");
        detID++;
      }
    }
  } // he3tube namespace
} // Belle2 namespace
