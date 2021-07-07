/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/beamabort/geometry/BeamabortCreator.h>
#include <beast/beamabort/simulation/SensitiveDetector.h>

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
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>

//Visualization
#include "G4Colour.hh"
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the BEAMABORT detector */
  namespace beamabort {

    // Register the creator
    /** Creator creates the BEAMABORT geometry */
    geometry::CreatorFactory <BeamabortCreator> BeamabortFactory("BEAMABORTCreator");

    BeamabortCreator::BeamabortCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    BeamabortCreator::~BeamabortCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void BeamabortCreator::create(const GearDir& content, G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      B2INFO("BeamabortCreator phase2");
      //Visualization Attributes
      G4VisAttributes* orange = new G4VisAttributes(G4Colour(1, 2, 0));
      orange->SetForceAuxEdgeVisible(true);
      G4VisAttributes* magenta = new G4VisAttributes(G4Colour(1, 0, 1));
      magenta->SetForceAuxEdgeVisible(true);
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //Lets loop over all the Active nodes
      BOOST_FOREACH(
        const GearDir & activeParams, content.getNodes("Active")) {

        int phase = activeParams.getInt("phase");
        //Positioned PIN diodes
        std::vector<double> x_pos;
        std::vector<double> y_pos;
        std::vector<double> z_pos;
        std::vector<double> phi;
        std::vector<double> thetaX;
        std::vector<double> thetaY;
        std::vector<double> thetaZ;
        std::vector<double> svdAngle;
        std::vector<double> r;
        std::vector<double> deltaX;
        unsigned int dimz = 0;

        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos.push_back(z);
          dimz++;
        }

        for (double ThetaZ : activeParams.getArray("ThetaZ", {0})) {
          thetaZ.push_back(ThetaZ);
        }

        if (thetaZ.size() != dimz) { B2ERROR("Diamond data not consistent (i.e. not same number of all position parmeters)"); return;}

        if (phase == 2 || phase == 3) {

          for (double Phi : activeParams.getArray("Phi", {0})) {
            phi.push_back(Phi);
          }

          for (double r_dia : activeParams.getArray("r_dia", {0})) {
            r_dia *= CLHEP::cm;
            r.push_back(r_dia);
          }
          for (double dX : activeParams.getArray("deltaX", {0})) {
            dX *= CLHEP::cm;
            deltaX.push_back(dX);
          }
          for (double addAngle : activeParams.getArray("addAngle", {0})) {
            svdAngle.push_back(addAngle);
          }
          if (phi.size() != dimz || r.size() != dimz || deltaX.size() != dimz || svdAngle.size() != dimz) { B2ERROR("Diamond data not consistent (i.e. not same number of all position parmeters)"); return;}
        }
        if (phase == 1) {
          for (double x : activeParams.getArray("x", {0})) {
            x *= CLHEP::cm;
            x_pos.push_back(x);
          }
          for (double y : activeParams.getArray("y", {0})) {
            y *= CLHEP::cm;
            y_pos.push_back(y);
          }

          for (double ThetaX : activeParams.getArray("ThetaX", {0})) {
            thetaX.push_back(ThetaX);
          }
          for (double ThetaY : activeParams.getArray("ThetaY", {0})) {
            thetaY.push_back(ThetaY);
          }
          if (x_pos.size() != dimz || y_pos.size() != dimz || thetaX.size() != dimz || thetaY.size() != dimz) { B2ERROR("Diamond data not consistent (i.e. not same number of all position parmeters)"); return;}
        }


        //create beamabort package
        G4double dx_opa = 12. / 2. * CLHEP::mm;
        G4double dy_opa = 18. / 2. * CLHEP::mm;
        G4double dz_opa = 3.1 / 2. * CLHEP::mm;
        G4VSolid* s_pa = new G4Box("s_opa", dx_opa, dy_opa, dz_opa);
        G4double dx_ipa = 6. / 2. * CLHEP::mm;
        G4double dy_ipa = 6. / 2. * CLHEP::mm;
        G4double dz_ipa = 5.6 / 2. * CLHEP::mm;
        G4VSolid* s_ipa = new G4Box("s_ipa", dx_ipa, dy_ipa, dz_ipa);
        s_pa = new G4SubtractionSolid("s_pa", s_pa, s_ipa, 0, G4ThreeVector(0., 4.0, 0.));
        G4LogicalVolume* l_pa = new G4LogicalVolume(s_pa, G4Material::GetMaterial("Al6061"), "l_pa");
        l_pa->SetVisAttributes(magenta);
        G4Transform3D transform;
        for (unsigned int i = 0; i < dimz; i++) {
          B2INFO("DIA-" << i << "RotateZ3D phi: " << phi[i]);

          if (phase == 1) {
            transform = G4Translate3D(x_pos[i], y_pos[i], z_pos[i]) * G4RotateX3D(thetaX[i]) *
                        G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]);
          } else if (phase == 2 || phase == 3) {
            transform = G4Translate3D(deltaX[i], 0, 0) * G4RotateZ3D(phi[i]) *
                        G4Translate3D(r[i], 0, z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                        G4RotateZ3D(svdAngle[i]) * G4RotateY3D(M_PI / 2);
          }
          new G4PVPlacement(transform, l_pa, TString::Format("p_dia_pa_%d", i).Data(), &topVolume, false, 0);
          B2INFO("DIA-" << i << " placed at: " << transform.getTranslation() << " mm ");
        }

        //create beamabort volumes
        G4double dx_ba = 4.5 / 2. * CLHEP::mm;
        G4double dy_ba = 4.5 / 2. * CLHEP::mm;
        G4double dz_ba = 0.5 / 2. * CLHEP::mm;
        G4Box* s_BEAMABORT = new G4Box("s_BEAMABORT", dx_ba, dy_ba, dz_ba);
        G4LogicalVolume* l_BEAMABORT = new G4LogicalVolume(s_BEAMABORT, geometry::Materials::get("Diamond"),
                                                           "l_BEAMABORT", 0, m_sensitive);
        l_BEAMABORT->SetVisAttributes(orange);

        //Lets limit the Geant4 stepsize inside the volume
        l_BEAMABORT->SetUserLimits(new G4UserLimits(stepSize));
        l_BEAMABORT->SetVisAttributes(orange);

        for (unsigned int i = 0; i < dimz; i++) {

          if (phase == 1)
            transform = G4Translate3D(x_pos[i], y_pos[i],
                                      z_pos[i]) * G4RotateX3D(thetaX[i]) * G4RotateY3D(thetaY[i]) * G4RotateZ3D(thetaZ[i]);
          if (phase == 2 || phase == 3)
            transform = G4Translate3D(deltaX[i], 0, 0) * G4RotateZ3D(phi[i]) *
                        G4Translate3D(r[i], 0, z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]) *
                        G4RotateZ3D(svdAngle[i]) * G4RotateY3D(M_PI / 2) * G4Translate3D(0, 4.2, 0);

          new G4PVPlacement(transform, l_BEAMABORT, TString::Format("p_dia_%d", i).Data(), &topVolume, false, i);

          B2INFO("DIA-sensitive volume-" << i << " placed at: " << transform.getTranslation() << " mm "
                 << "  at phi angle = " << phi[i] << "   at theta angle = "
                 << thetaZ[i]);
          B2INFO("DIA-sensitive volume-" << i << " G4RotateZ3D of: phi= " << phi[i] << "  G4RotateX3D = "
                 << (-M_PI / 2 - thetaZ[i]));
        }
      }
    }
  }
}
