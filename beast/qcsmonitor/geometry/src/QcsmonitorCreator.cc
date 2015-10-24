/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/qcsmonitor/geometry/QcsmonitorCreator.h>
#include <beast/qcsmonitor/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <qcsmonitor/simulation/SensitiveDetector.h>

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

//Visualization Attributes
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the QCSMONITOR detector */
  namespace qcsmonitor {

    // Register the creator
    /** Creator creates the QCSMONITOR geometry */
    geometry::CreatorFactory<QcsmonitorCreator> QcsmonitorFactory("QCSMONITORCreator");

    QcsmonitorCreator::QcsmonitorCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    QcsmonitorCreator::~QcsmonitorCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void QcsmonitorCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
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

      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        G4double dx_scint = activeParams.getLength("dx_scint") / 2.*CLHEP::cm;
        G4double dy_scint = activeParams.getLength("dy_scint") / 2.*CLHEP::cm;
        G4double dz_scint = activeParams.getLength("dz_scint") / 2.*CLHEP::cm;
        double thetaZ = activeParams.getAngle("ThetaZ");
        G4VSolid* s_scint = new G4Box("s_scint", dx_scint, dy_scint, dz_scint);
        G4LogicalVolume* l_scint = new G4LogicalVolume(s_scint, geometry::Materials::get("G4_POLYSTYRENE"), "l_scint", 0, m_sensitive);
        l_scint->SetVisAttributes(red);
        //Lets limit the Geant4 stepsize inside the volume
        l_scint->SetUserLimits(new G4UserLimits(stepSize));
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
          r_pos[dim] = r + dz_scint;
          dim++;
        }
        for (double phi : activeParams.getArray("Phi", {M_PI / 2})) {
          //phi  *= CLHEP::deg;
          for (int i = 0; i < dim; i++) {
            cout << " r " << r_pos[i] << " width " << dz_scint << " z " << z_pos[i] << " phi " << phi << endl;
            G4Transform3D transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(0, r_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ);
            new G4PVPlacement(transform, l_scint, "p_scint", &topVolume, false, detID);
            detID++;
          }
        }
        cout << " Nb of detector " << detID << endl;
      }
    }
  } // qcsmonitor namespace
} // Belle2 namespace
