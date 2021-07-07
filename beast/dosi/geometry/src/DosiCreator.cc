/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/dosi/geometry/DosiCreator.h>
#include <beast/dosi/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4UserLimits.hh>

//Visualization Attributes
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the DOSI detector */
  namespace dosi {

    // Register the creator
    /** Creator creates the DOSI geometry */
    geometry::CreatorFactory<DosiCreator> DosiFactory("DOSICreator");

    DosiCreator::DosiCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    DosiCreator::~DosiCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void DosiCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      G4VisAttributes* red = new G4VisAttributes(G4Colour(1, 0, 0));
      red->SetForceAuxEdgeVisible(true);
      G4VisAttributes* green = new G4VisAttributes(G4Colour(0, 1, 0));
      green->SetForceAuxEdgeVisible(true);
      G4VisAttributes* gray = new G4VisAttributes(G4Colour(.5, .5, .5));
      gray->SetForceAuxEdgeVisible(true);
      G4VisAttributes* coppercolor = new G4VisAttributes(G4Colour(218. / 255., 138. / 255., 103. / 255.));
      coppercolor->SetForceAuxEdgeVisible(true);

      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
        G4double dx_dosi = activeParams.getLength("dx_dosi") / 2.*CLHEP::cm;
        G4double dy_dosi = activeParams.getLength("dy_dosi") / 2.*CLHEP::cm;
        G4double dz_dosi = activeParams.getLength("dz_dosi") / 2.*CLHEP::cm;
        double thetaZ = activeParams.getAngle("ThetaZ");
        G4VSolid* s_dosi = new G4Box("s_dosi", dx_dosi, dy_dosi, dz_dosi);
        //G4LogicalVolume* l_dosi = new G4LogicalVolume(s_dosi, geometry::Materials::get("BGO"), "l_dosi", 0, m_sensitive);
        G4LogicalVolume* l_dosi = new G4LogicalVolume(s_dosi, geometry::Materials::get("G4_SILICON_DIOXIDE"), "l_dosi", 0, m_sensitive);

        l_dosi->SetVisAttributes(green);
        //Lets limit the Geant4 stepsize inside the volume
        l_dosi->SetUserLimits(new G4UserLimits(stepSize));
        double z_pos[100];
        double r_pos[100];
        int dim = 0;
        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dim] = z;
          dim++;
        }
        dim = 0;
        for (double r : activeParams.getArray("r", {0})) {
          r *= CLHEP::cm;
          r_pos[dim] = r + dz_dosi;
          dim++;
        }

        int detID = 0;
        G4Transform3D transform;
        for (double phi : activeParams.getArray("Phi", {M_PI / 2})) {
          //phi  *= CLHEP::deg;
          for (int i = 0; i < dim; i++) {
            transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(0, r_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ);
            new G4PVPlacement(transform, l_dosi, TString::Format("p_dosi_%d", detID).Data() , &topVolume, false, detID);
            detID++;
          }
        }
      }
    }
  } // dosi namespace
} // Belle2 namespace
