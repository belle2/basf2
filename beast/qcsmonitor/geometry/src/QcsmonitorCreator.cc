/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/qcsmonitor/geometry/QcsmonitorCreator.h>
#include <beast/qcsmonitor/simulation/SensitiveDetector.h>

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
#include <G4Box.hh>
#include <G4UserLimits.hh>

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
      //m_sensitive = new SensitiveDetector();
    }

    QcsmonitorCreator::~QcsmonitorCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void QcsmonitorCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
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
        int phase = activeParams.getInt("phase");
        G4double dx_scint = activeParams.getLength("dx_scint") / 2.*CLHEP::cm;
        G4double dy_scint = activeParams.getLength("dy_scint") / 2.*CLHEP::cm;
        G4double dz_scint = activeParams.getLength("dz_scint") / 2.*CLHEP::cm;
        double thetaZ = activeParams.getAngle("ThetaZ");
        G4VSolid* s_scint = new G4Box("s_scint", dx_scint, dy_scint, dz_scint);
        G4LogicalVolume* l_scint = new G4LogicalVolume(s_scint, geometry::Materials::get("G4_POLYSTYRENE"), "l_scint", 0, m_sensitive);
        l_scint->SetVisAttributes(green);
        //Lets limit the Geant4 stepsize inside the volume
        l_scint->SetUserLimits(new G4UserLimits(stepSize));
        double x_pos[100];
        double y_pos[100];
        double z_pos[100];
        double r_pos[100];
        int dim = 0;
        if (phase == 1) {
          dim = 0;
          for (double x : activeParams.getArray("x", {0})) {
            x *= CLHEP::cm;
            x_pos[dim] = x;
            dim++;
          }
          dim = 0;
          for (double y : activeParams.getArray("y", {0})) {
            y *= CLHEP::cm;
            y_pos[dim] = y;
            dim++;
          }
        }
        dim = 0;
        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dim] = z;
          //cout << "QCSS z " << z << " zpos " << z_pos[dim] << endl;
          if (phase == 1) {
            r_pos[dim] = sqrt(x_pos[dim] * x_pos[dim] + y_pos[dim] * y_pos[dim]);
          }
          dim++;
        }
        //if (phase == 1) dim = 1;
        if (phase == 2) {
          dim = 0;
          for (double r : activeParams.getArray("r", {0})) {
            r *= CLHEP::cm;
            r_pos[dim] = r + dz_scint;
            dim++;
          }
          for (int i = 0; i < 100; i++) {
            x_pos[i] = 0;
            y_pos[i] = 0;
            //x_off[i] = 0;
            //y_off[i] = 0;
          }
        }
        int detID = 0;
        G4Transform3D transform;
        for (double phi : activeParams.getArray("Phi", {M_PI / 2})) {
          //phi  *= CLHEP::deg;
          for (int i = 0; i < dim; i++) {
            if (phase == 2) {
              transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(0, r_pos[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ);
              //cout << "phase 2" << endl;
            }
            if (phase == 1) {
              transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateZ3D(phi) * G4RotateX3D(thetaZ);
              //cout << "phase 1" << endl;
            }
            //cout << "QCS r " << r_pos[i] << " width " << dz_scint << " z " << z_pos[i] << " phi " << phi << " x " << x_pos[i] << " y " <<
            //     y_pos[i] << endl;
            new G4PVPlacement(transform, l_scint, TString::Format("p_scint_%d", detID).Data() , &topVolume, false, detID);
            B2INFO("QCSS-" << detID << " placed at: " << transform.getTranslation() << " mm ");
            //cout << " Nb of detector " << detID << endl;
            detID++;
          }
        }
      }
    }
  } // qcsmonitor namespace
} // Belle2 namespace
