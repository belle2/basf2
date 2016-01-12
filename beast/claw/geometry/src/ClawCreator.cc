/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claw/geometry/ClawCreator.h>
#include <beast/claw/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <claw/simulation/SensitiveDetector.h>

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

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CLAW detector */
  namespace claw {

    // Register the creator
    /** Creator creates the CLAW geometry */
    geometry::CreatorFactory<ClawCreator> ClawFactory("CLAWCreator");

    ClawCreator::ClawCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    ClawCreator::~ClawCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void ClawCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      /*
      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      */
      //Visualization Attributes
      //G4VisAttributes *invis = new G4VisAttributes(G4Colour(1,1,1));
      //invis->SetColor(0,0,0,0);
      //invis->SetForceWireframe(true);
      //invis->SetVisibility(false);
      G4VisAttributes* red = new G4VisAttributes(G4Colour(1, 0, 0));
      red->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *redwire = new G4VisAttributes(G4Colour(1,0,0));
      //redwire->SetForceAuxEdgeVisible(true);
      //redwire->SetForceWireframe(true);
      G4VisAttributes* green = new G4VisAttributes(G4Colour(0, 1, 0));
      green->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *blue = new G4VisAttributes(G4Colour(0,0,1));
      //blue->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *white = new G4VisAttributes(G4Colour(1,1,1));
      //white->SetForceAuxEdgeVisible(true);
      G4VisAttributes* gray = new G4VisAttributes(G4Colour(.5, .5, .5));
      gray->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *yellow = new G4VisAttributes(G4Colour(1,1,0));
      //yellow->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *cyan = new G4VisAttributes(G4Colour(0,1,1));
      //cyan->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *magenta = new G4VisAttributes(G4Colour(1,0,1));
      //magenta->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *brown = new G4VisAttributes(G4Colour(.5,.5,0));
      //brown->SetForceAuxEdgeVisible(true);
      //G4VisAttributes *orange = new G4VisAttributes(G4Colour(1,2,0));
      //orange->SetForceAuxEdgeVisible(true);
      G4VisAttributes* coppercolor = new G4VisAttributes(G4Colour(218. / 255., 138. / 255., 103. / 255.));
      coppercolor->SetForceAuxEdgeVisible(true);

      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        double x_pos[100];
        double y_pos[100];
        double x_off[100];
        double y_off[100];
        double z_pos[100];
        double phi[100];
        double thetaZ[100];
        double r[100];
        int dimx = 0;
        int dimy = 0;
        int dimx_offset = 0;
        int dimy_offset = 0;
        int dimz = 0;
        int dimr_dia = 0;
        for (double x_offset : activeParams.getArray("x_offset", {0})) {
          x_offset *= CLHEP::cm;
          x_off[dimx_offset] = x_offset;
          dimx_offset++;
        }
        for (double y_offset : activeParams.getArray("y_offset", {0})) {
          y_offset *= CLHEP::cm;
          y_off[dimy_offset] = y_offset;
          dimy_offset++;
        }
        for (double x : activeParams.getArray("x", {0})) {
          x *= CLHEP::cm;
          x_pos[dimx] = x + x_off[dimx];
          dimx++;
        }
        for (double y : activeParams.getArray("y", {0})) {
          y *= CLHEP::cm;
          y_pos[dimy] = y + y_off[dimy];
          dimy++;
        }
        for (double z : activeParams.getArray("z", {0})) {
          z *= CLHEP::cm;
          z_pos[dimz] = z;
          if (dimx != 0 && dimy != 0)
            r[dimz] = sqrt(x_pos[dimz] * x_pos[dimz] + y_pos[dimz] * y_pos[dimz]);
          dimz++;
        }
        int dimPhi = 0;
        for (double Phi : activeParams.getArray("Phi", {0})) {
          phi[dimPhi] = Phi  - 90. * CLHEP::deg;
          dimPhi++;
        }
        int dimThetaZ = 0;
        for (double ThetaZ : activeParams.getArray("ThetaZ", {0})) {
          thetaZ[dimThetaZ] = ThetaZ;
          dimThetaZ++;
        }
        if (dimx == 0 && dimy == 0) {
          for (double r_dia : activeParams.getArray("r_dia", {0})) {
            r_dia *= CLHEP::cm;
            r[dimr_dia] = r_dia;
            dimr_dia++;
          }
        }
        int Nscint = activeParams.getInt("Nscint");
        G4double dx_board = activeParams.getLength("dx_board") / 2.*CLHEP::cm;
        G4double dy_board = activeParams.getLength("dy_board") / 2.*CLHEP::cm;
        G4double dz_board = activeParams.getLength("dz_board") / 2.*CLHEP::cm;
        G4double dz_Culayer =  activeParams.getLength("dz_Culayer") / 2.*CLHEP::cm;
        G4double dx_scint = activeParams.getLength("dx_scint") / 2.*CLHEP::cm;
        G4double dy_scint = activeParams.getLength("dy_scint") / 2.*CLHEP::cm;
        G4double dz_scint = activeParams.getLength("dz_scint") / 2.*CLHEP::cm;
        G4double Al_width = activeParams.getLength("Al_width") / 2.*CLHEP::cm;
        G4double dx_Al = dx_scint + Al_width;
        G4double dy_Al = dy_scint + Al_width;
        G4double dz_Al = dz_scint + Al_width;
        G4double dx_air = dx_Al;
        G4double dy_air = dy_Al * (double)Nscint;
        G4double dz_air = dz_board + 2. * dz_Culayer + dz_Al;

        //create air volume engloging ladder + scintillator + Al foil
        G4Box* s_air = new G4Box("s_air", dx_air, dy_air, dz_air);
        G4LogicalVolume* l_air = new G4LogicalVolume(s_air, geometry::Materials::get("G4_AIR"), "l_air");

        //create claw G10 board
        G4Box* s_board = new G4Box("s_board", dx_board, dy_board, dz_board);
        G4LogicalVolume* l_board = new G4LogicalVolume(s_board, geometry::Materials::get("G10"), "l_board");
        l_board->SetVisAttributes(green);
        G4double r_board = dz_air - 2. * dz_Culayer - dz_board;

        //create copper layer on each side of the G10 board
        G4Box* s_Culayer = new G4Box("s_Culayer", dx_board, dy_board, dz_Culayer);
        G4LogicalVolume* l_Culayer = new G4LogicalVolume(s_Culayer, geometry::Materials::get("MetalCopper"), "l_Culayer");
        l_Culayer->SetVisAttributes(coppercolor);

        G4double r_Culayer_bot = r_board - dz_board - dz_Culayer;
        G4double r_Culayer_top = r_board + dz_board + dz_Culayer;
        //cout << " cu bot sol 1 " << r_Culayer_bot << " sol 2 " << -dz_air + dz_Culayer<< endl;

        //create scintillator and Al foil around it
        G4VSolid* s_scint = new G4Box("s_scint", dx_scint, dy_scint, dz_scint);
        G4VSolid* s_Al = new G4Box("s_Al", dx_Al, dy_Al, dz_Al);
        s_Al = new G4SubtractionSolid("s_Al", s_Al, s_scint, 0, G4ThreeVector(0, 0, 0));
        G4LogicalVolume* l_Al = new G4LogicalVolume(s_Al, geometry::Materials::get("Aluminum"), "l_Al");
        l_Al->SetVisAttributes(gray);

        G4LogicalVolume* l_scint = new G4LogicalVolume(s_scint, geometry::Materials::get("G4_POLYSTYRENE"), "l_scint", 0, m_sensitive);
        l_scint->SetVisAttributes(red);

        //Lets limit the Geant4 stepsize inside the volume
        l_scint->SetUserLimits(new G4UserLimits(stepSize));
        G4double r_Al = r_Culayer_bot - dz_Culayer - dz_Al;
        double z_0 = -dy_air + dy_Al;

        for (int i = 0; i < dimz; i++) {
          G4Transform3D transform = G4RotateZ3D(phi[i]) * G4Translate3D(0, r[i], z_pos[i]) * G4RotateX3D(-M_PI / 2 - thetaZ[i]);
          new G4PVPlacement(transform, l_air, "p_air", &topVolume, false, 1);
          new G4PVPlacement(0, G4ThreeVector(0, 0, r_board), l_board, "p_board", l_air, false, 1);
          new G4PVPlacement(0, G4ThreeVector(0, 0, r_Culayer_bot), l_Culayer, "p_Culayer_bot", l_air, false, 1);
          new G4PVPlacement(0, G4ThreeVector(0, 0, r_Culayer_top), l_Culayer, "p_Culayer_top", l_air, false, 1);
          for (int j = 0; j < Nscint; j++) {
            double i_z = z_0 + j * 2. * dy_Al;
            new G4PVPlacement(0, G4ThreeVector(0, i_z, r_Al), l_Al, "p_Al", l_air, false, 1);
            new G4PVPlacement(0, G4ThreeVector(0, i_z, r_Al), l_scint, "p_scint", l_air, false, detID);
            cout << "# CLAWS " << detID << endl;
            detID++;
          }
        }
      }
    }
  } // claw namespace
} // Belle2 namespace
