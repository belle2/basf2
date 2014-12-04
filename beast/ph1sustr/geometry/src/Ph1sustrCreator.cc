/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/ph1sustr/geometry/Ph1sustrCreator.h>
#include <beast/ph1sustr/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <ph1sustr/simulation/SensitiveDetector.h>

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

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PH1SUSTR detector */
  namespace ph1sustr {

    // Register the creator
    /** Creator creates the phase 1 support structure geometry */
    geometry::CreatorFactory<Ph1sustrCreator> Ph1sustrFactory("PH1SUSTRCreator");

    Ph1sustrCreator::Ph1sustrCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    Ph1sustrCreator::~Ph1sustrCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1sustrCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
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
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create ph1sustr volume
        G4Trap* s_PH1SUSTR = new G4Trap("s_PH1SUSTR",
                                        activeParams.getLength("cDz") / 2.*CLHEP::mm , 0 , 0,
                                        activeParams.getLength("cDy1") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx1") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx2") / 2.*CLHEP::mm , 0,
                                        activeParams.getLength("cDy2") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx3") / 2.*CLHEP::mm ,
                                        activeParams.getLength("cDx4") / 2.*CLHEP::mm , 0);


        G4LogicalVolume* l_PH1SUSTR = new G4LogicalVolume(s_PH1SUSTR, geometry::Materials::get("PH1SUSTR"), "l_PH1SUSTR", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_PH1SUSTR->SetUserLimits(new G4UserLimits(stepSize));

        //position ph1sustr volume
        G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                                      activeParams.getLength("x_ph1sustr") * CLHEP::cm,
                                      activeParams.getLength("y_ph1sustr") * CLHEP::cm,
                                      activeParams.getLength("z_ph1sustr") * CLHEP::cm
                                    );

        G4RotationMatrix* rot_ph1sustr = new G4RotationMatrix();
        rot_ph1sustr->rotateX(activeParams.getLength("AngleX"));
        rot_ph1sustr->rotateY(activeParams.getLength("AngleY"));
        rot_ph1sustr->rotateZ(activeParams.getLength("AngleZ"));
        //geometry::setColor(*l_PH1SUSTR, "#006699");

        new G4PVPlacement(rot_ph1sustr, PH1SUSTRpos, l_PH1SUSTR, "p_PH1SUSTR", &topVolume, false, detID);

        detID++;
      }
      */
      G4double dx_20V2000 = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dy_20V2000 = 2.54 * 1.63 / 2.*CLHEP::cm;
      G4double dz_20V2000 = 2200. / 2.*CLHEP::mm;
      G4double dz_20V2100v = 1537.37 / 2.*CLHEP::mm;
      G4double dz_20V2100h = 1792. / 2.*CLHEP::mm;
      G4double dz_20V2100bgov = 843.72 / 2.*CLHEP::mm;
      G4double dz_20V2100bgoh = 280.00 / 2.*CLHEP::mm;
      G4double dw_20V2000 = 2.54 * 0.25 / 2.*CLHEP::cm;
      G4VSolid* s_20V2000_a = new G4Box("s_20V2000_a", dx_20V2000, dy_20V2000, dz_20V2000);
      G4VSolid* s_20V2000_b = new G4Box("s_20V2000_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2000);
      G4VSolid* s_20V2100v_a = new G4Box("s_20V1000v_a", dx_20V2000, dy_20V2000, dz_20V2100v);
      G4VSolid* s_20V2100v_b = new G4Box("s_20V1000v_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100v);
      G4VSolid* s_20V2100bgov_a = new G4Box("s_20V1000bgov_a", dx_20V2000, dy_20V2000, dz_20V2100bgov);
      G4VSolid* s_20V2100bgov_b = new G4Box("s_20V1000bgov_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100bgov);
      G4VSolid* s_20V2100h_a = new G4Box("s_20V1000h_a", dx_20V2000, dy_20V2000, dz_20V2100h);
      G4VSolid* s_20V2100h_b = new G4Box("s_20V1000h_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100h);
      G4VSolid* s_20V2100bgoh_a = new G4Box("s_20V1000bgoh_a", dx_20V2000, dy_20V2000, dz_20V2100bgoh);
      G4VSolid* s_20V2100bgoh_b = new G4Box("s_20V1000bgoh_b", dx_20V2000 - 2.*dw_20V2000, dy_20V2000 - dw_20V2000, dz_20V2100bgoh);

      G4VSolid* s_20V2000 = new G4SubtractionSolid("s_20V2000", s_20V2000_a, s_20V2000_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100vpos = new G4SubtractionSolid("s_20V2100vpos", s_20V2100v_a, s_20V2100v_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100vneg = new G4SubtractionSolid("s_20V2100vneg", s_20V2100v_a, s_20V2100v_b, 0, G4ThreeVector(0, -dw_20V2000, 0));
      G4VSolid* s_20V2100bgovpos = new G4SubtractionSolid("s_20V2100bgovpos", s_20V2100bgov_a, s_20V2100bgov_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100bgovneg = new G4SubtractionSolid("s_20V2100bgovneg", s_20V2100bgov_a, s_20V2100bgov_b, 0, G4ThreeVector(0, -dw_20V2000, 0));

      G4VSolid* s_20V2100hpos = new G4SubtractionSolid("s_20V2100hpos", s_20V2100h_a, s_20V2100h_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100hneg = new G4SubtractionSolid("s_20V2100hneg", s_20V2100h_a, s_20V2100h_b, 0, G4ThreeVector(0, -dw_20V2000, 0));
      G4VSolid* s_20V2100bgohpos = new G4SubtractionSolid("s_20V2100bgohpos", s_20V2100bgoh_a, s_20V2100bgoh_b, 0, G4ThreeVector(0, dw_20V2000, 0));
      G4VSolid* s_20V2100bgohneg = new G4SubtractionSolid("s_20V2100bgohneg", s_20V2100bgoh_a, s_20V2100bgoh_b, 0, G4ThreeVector(0, -dw_20V2000, 0));

      G4double dx_20V2300 = 2.54 * 2.38 / 2.*CLHEP::cm;
      G4double dy_20V2300 = 2.54 * 0.25 / 2.*CLHEP::cm;
      G4double dz_20V2300 = 2200. / 2.*CLHEP::mm;
      G4VSolid* s_20V2300_a = new G4Box("s_20V2300_a", dx_20V2300, dy_20V2300, dz_20V2300);
      G4VSolid* s_20V2300 = new G4UnionSolid("s_20V2300", s_20V2000, s_20V2300_a, 0, G4ThreeVector(0, -dy_20V2000 - dw_20V2000, 0));

      G4LogicalVolume* l_20V2300 = new G4LogicalVolume(s_20V2300,  geometry::Materials::get("MetalCopper") , "l_20V2300", 0, m_sensitive);

      //Lets limit the Geant4 stepsize inside the volume
      l_20V2300->SetUserLimits(new G4UserLimits(stepSize));

      G4ThreeVector PH1SUSTRpos = G4ThreeVector(
                                    708.72 * CLHEP::mm,
                                    136.67 * CLHEP::mm,
                                    0.
                                  );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      708.72 * CLHEP::mm,
                      -136.67 * CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 1);

      PH1SUSTRpos = G4ThreeVector(
                      -707.81 * CLHEP::mm,
                      136.67 * CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 2);

      PH1SUSTRpos = G4ThreeVector(
                      -707.81 * CLHEP::mm,
                      -136.67 * CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 3);

      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 4);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      -567.97 * CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 5);

      PH1SUSTRpos = G4ThreeVector(
                      136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 6);

      PH1SUSTRpos = G4ThreeVector(
                      -136.68 * CLHEP::mm,
                      818.*CLHEP::mm,
                      0.
                    );
      new G4PVPlacement(0, PH1SUSTRpos, l_20V2300, "p_PH1SUSTR", &topVolume, false, 7);

      G4VSolid* s_20V2100v = new G4UnionSolid("s_20V2100v", s_20V2100vpos, s_20V2100vneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100v = new G4LogicalVolume(s_20V2100v,  geometry::Materials::get("MetalCopper") , "l_20V2100v", 0, 0);

      G4RotationMatrix* rotX = new G4RotationMatrix();
      rotX->rotateX(90.*CLHEP::deg);
      PH1SUSTRpos = G4ThreeVector(
                      -791.3 * CLHEP::mm,
                      0.,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      791.3 * CLHEP::mm,
                      0,
                      -800.*CLHEP::mm
                    );

      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -791.3 * CLHEP::mm,
                      0.,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      791.3 * CLHEP::mm,
                      0.,
                      1100.*CLHEP::mm
                    );

      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100v, "p_20V2100v", &topVolume, false, 0);


      G4VSolid* s_20V2100h = new G4UnionSolid("s_20V2100h", s_20V2100hpos, s_20V2100hneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100h = new G4LogicalVolume(s_20V2100h,  geometry::Materials::get("MetalCopper") , "l_20V2100h", 0, 0);

      G4RotationMatrix* rotY = new G4RotationMatrix();
      rotY->rotateY(90.*CLHEP::deg);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -896.7 * CLHEP::mm,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      895.3 * CLHEP::mm,
                      1100.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);


      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -896.7 * CLHEP::mm,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      895.3 * CLHEP::mm,
                      -800.*CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100h, "p_20V2100h", &topVolume, false, 0);

      G4VSolid* s_20V2100bgov = new G4UnionSolid("s_20V2100bgov", s_20V2100bgovpos, s_20V2100bgovneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100bgov = new G4LogicalVolume(s_20V2100bgov,  geometry::Materials::get("MetalCopper") , "l_20V2100bgov", 0, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -181.3 * CLHEP::mm,
                      0.,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      181.3 * CLHEP::mm,
                      0.,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      -181.3 * CLHEP::mm,
                      0.,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      181.3 * CLHEP::mm,
                      0.,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotX, PH1SUSTRpos, l_20V2100bgov, "p_20V2100bgov", &topVolume, false, 0);

      G4VSolid* s_20V2100bgoh = new G4UnionSolid("s_20V2100bgoh", s_20V2100bgohpos, s_20V2100bgohneg, 0, G4ThreeVector(0, -2.*dy_20V2000, 0));
      G4LogicalVolume* l_20V2100bgoh = new G4LogicalVolume(s_20V2100bgoh,  geometry::Materials::get("MetalCopper") , "l_20V2100bgoh", 0, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -166.3 * CLHEP::mm,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      166.3 * CLHEP::mm,
                      348.72 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      -166.3 * CLHEP::mm,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

      PH1SUSTRpos = G4ThreeVector(
                      0 * CLHEP::mm,
                      166.3 * CLHEP::mm,
                      -188.73 * CLHEP::mm
                    );
      new G4PVPlacement(rotY, PH1SUSTRpos, l_20V2100bgoh, "p_20V2100bgoh", &topVolume, false, 0);

    }
  } // ph1sustr namespace
} // Belle2 namespace
