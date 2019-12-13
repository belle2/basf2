/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/geometry/BeamabortCreator.h>
#include <beast/beamabort/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

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

    BeamabortCreator::BeamabortCreator() : m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    BeamabortCreator::~BeamabortCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void BeamabortCreator::create(const GearDir& content, G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes /* type */)
    {

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
        double x_pos[100];
        double y_pos[100];
        double z_pos[100];
        double phi[100];
        double thetaX[100];
        double thetaY[100];
        double thetaZ[100];
        double svdAngle[100];
        double r[100];
        double deltaX[100];
        int dimz = 0;
        int dimr_dia = 0;
        int dimDeltaX = 0;
        if (phase == 2 || phase == 3) {
          for (int i = 0; i < 100; i++) {
            x_pos[i] = 0;
            y_pos[i] = 0;
            z_pos[i] = 0;
          }
          int dimPhi = 0;
          for (double Phi : activeParams.getArray("Phi", {0})) {
            phi[dimPhi] = Phi;
            dimPhi++;
          }
          for (double r_dia : activeParams.getArray("r_dia", {0})) {
            r_dia *= CLHEP::cm;
            r[dimr_dia] = r_dia;
            dimr_dia++;
          }
          for (double dX : activeParams.getArray("deltaX", {0})) {
            dX *= CLHEP::cm;
            deltaX[dimDeltaX] = dX;
            dimDeltaX++;
          }
          int dimAngle = 0;
          for (double addAngle : activeParams.getArray("addAngle", {0})) {
            svdAngle[dimAngle] = addAngle;
            dimAngle++;
          }
        }
        if (phase == 1) {
          int dimx = 0;
          int dimy = 0;
          for (double x : activeParams.getArray("x", {0})) {
            x *= CLHEP::cm;
            x_pos[dimx] = x;
            dimx++;
          }
          for (double y : activeParams.getArray("y", {0})) {
            y *= CLHEP::cm;
            y_pos[dimy] = y;
            dimy++;
          }
          int dimThetaX = 0;
          for (double ThetaX : activeParams.getArray("ThetaX", {0})) {
            thetaX[dimThetaX] = ThetaX;
            dimThetaX++;
          }
          int dimThetaY = 0;
          for (double ThetaY : activeParams.getArray("ThetaY", {0})) {
            thetaY[dimThetaY] = ThetaY;
            dimThetaY++;
          }
        }

        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dimz] = z;
          dimz++;
        }
        int dimThetaZ = 0;
        for (double ThetaZ : activeParams.getArray("ThetaZ", {0})) {
          thetaZ[dimThetaZ] = ThetaZ;
          dimThetaZ++;
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
        for (int i = 0; i < dimz; i++) {
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

        for (int i = 0; i < dimz; i++) {

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
