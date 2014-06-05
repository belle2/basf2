/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/ph1bpipe/geometry/Ph1bpipeCreator.h>
#include <beast/ph1bpipe/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <ph1bpipe/simulation/SensitiveDetector.h>

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
#include <G4Ellipsoid.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PH1BPIPE detector */
  namespace ph1bpipe {

    // Register the creator
    /** Creator creates the phase 1 beam pipe for |s| < 4 m geometry */
    geometry::CreatorFactory<Ph1bpipeCreator> Ph1bpipeFactory("PH1BPIPECreator");

    Ph1bpipeCreator::Ph1bpipeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    Ph1bpipeCreator::~Ph1bpipeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1bpipeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * Unit::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      /*
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }

      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create ph1bpipe volume
        G4double startAngle = 0.*Unit::deg;
        G4double spanningAngle = 360.*Unit::deg;
        G4Tubs* s_PH1BPIPE = new G4Tubs("s_PH1BPIPE",
                                        activeParams.getLength("pipe_innerRadius")*Unit::cm,
                                        activeParams.getLength("pipe_outerRadius")*Unit::cm,
                                        activeParams.getLength("pipe_hz")*Unit::cm,
                                        startAngle, spanningAngle);

        string matPipe = activeParams.getString("MaterialPipe");
        G4LogicalVolume* l_PH1BPIPE = new G4LogicalVolume(s_PH1BPIPE, geometry::Materials::get(matPipe), "l_PH1BPIPE", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_PH1BPIPE->SetUserLimits(new G4UserLimits(stepSize));

        //position ph1bpipe volume
        G4ThreeVector PH1BPIPEpos = G4ThreeVector(
                                      activeParams.getLength("x_ph1bpipe") * Unit::cm,
                                      activeParams.getLength("y_ph1bpipe") * Unit::cm,
                                      activeParams.getLength("z_ph1bpipe") * Unit::cm
                                    );

        //geometry::setColor(*l_PH1BPIPE, "#006699");

        new G4PVPlacement(0, PH1BPIPEpos, l_PH1BPIPE, "p_PH1BPIPE", &topVolume, false, 1);

        //create endcaps
        G4Tubs* s_PH1BPIPEendcap = new G4Tubs("s_PH1BPIPEendcap",
                                              activeParams.getLength("endcap_innerRadius")*Unit::cm,
                                              activeParams.getLength("endcap_outerRadius")*Unit::cm,
                                              activeParams.getLength("endcap_hz")*Unit::cm,
                                              startAngle, spanningAngle);

        G4LogicalVolume* l_PH1BPIPEendcap = new G4LogicalVolume(s_PH1BPIPEendcap, geometry::Materials::get(matPipe), "l_PH1BPIPEendcap", 0, m_sensitive);

        //position endcaps
        G4ThreeVector PH1BPIPEendcapposTop = G4ThreeVector(
                                               activeParams.getLength("x_ph1bpipe") * Unit::cm,
                                               activeParams.getLength("y_ph1bpipe") * Unit::cm,
                                               activeParams.getLength("z_ph1bpipe") * Unit::cm + activeParams.getLength("pipe_hz") * Unit::cm + activeParams.getLength("endcap_hz") * Unit::cm
                                             );

        G4ThreeVector PH1BPIPEendcapposBot = G4ThreeVector(
                                               activeParams.getLength("x_ph1bpipe") * Unit::cm,
                                               activeParams.getLength("y_ph1bpipe") * Unit::cm,
                                               activeParams.getLength("z_ph1bpipe") * Unit::cm - activeParams.getLength("pipe_hz") * Unit::cm - activeParams.getLength("endcap_hz") * Unit::cm
                                             );

        new G4PVPlacement(0, PH1BPIPEendcapposTop, l_PH1BPIPEendcap, "p_PH1BPIPEendcapTop", &topVolume, false, 1);
        new G4PVPlacement(0, PH1BPIPEendcapposBot, l_PH1BPIPEendcap, "p_PH1BPIPEendcapBot", &topVolume, false, 1);

      }
      */

      //create ph1bpipe volume
      G4double startAngle = 0.*Unit::deg;
      G4double spanningAngle = 360.*Unit::deg;
      /*G4Tubs* s_PH1BPIPE = new G4Tubs("s_PH1BPIPE",
                                      content.getLength("pipe_innerRadius")*Unit::cm,
                                      content.getLength("pipe_outerRadius")*Unit::cm,
                                      content.getLength("pipe_hz")*Unit::cm,
                                      startAngle, spanningAngle);*/

      G4VSolid* s_PH1BPIPE1 = new G4Ellipsoid("s_PH1BPIPE1",
                                              content.getLength("pipe_outerRadius_x")*Unit::cm,
                                              content.getLength("pipe_outerRadius_y")*Unit::cm,
                                              content.getLength("pipe_hz")*Unit::cm,
                                              -2.* content.getLength("pipe_hz")*Unit::cm, 2.* content.getLength("pipe_hz")*Unit::cm);

      G4VSolid* s_PH1BPIPE2 = new G4Ellipsoid("s_PH1BPIPE2",
                                              content.getLength("pipe_innerRadius_x")*Unit::cm,
                                              content.getLength("pipe_innerRadius_y")*Unit::cm,
                                              content.getLength("pipe_hz")*Unit::cm,
                                              -2.* content.getLength("pipe_hz")*Unit::cm, 2.* content.getLength("pipe_hz")*Unit::cm);
      /*G4VSolid* s_PH1BPIPE2 = new G4Tubs("s_PH1BPIPE2",
                                         content.getLength("pipe_innerRadius_x")*Unit::cm,
                                         content.getLength("pipe_innerRadius_y")*Unit::cm,
                                         content.getLength("pipe_hz")*Unit::cm,
                                         startAngle, spanningAngle);*/

      G4VSolid* s_PH1BPIPE = new G4SubtractionSolid("s_PH1BPIPE", s_PH1BPIPE1, s_PH1BPIPE2);


      string matPipe = content.getString("MaterialPipe");
      G4LogicalVolume* l_PH1BPIPE = new G4LogicalVolume(s_PH1BPIPE, geometry::Materials::get(matPipe), "l_PH1BPIPE", 0, m_sensitive);

      //Lets limit the Geant4 stepsize inside the volume
      l_PH1BPIPE->SetUserLimits(new G4UserLimits(stepSize));

      //position central ph1bpipe volume
      G4ThreeVector PH1BPIPEpos = G4ThreeVector(
                                    content.getLength("x_ph1bpipe") * Unit::cm,
                                    content.getLength("y_ph1bpipe") * Unit::cm,
                                    content.getLength("z_ph1bpipe") * Unit::cm
                                  );

      //geometry::setColor(*l_PH1BPIPE, "#006699");

      new G4PVPlacement(0, PH1BPIPEpos, l_PH1BPIPE, "p_PH1BPIPE", &topVolume, false, 1);

      //create central endcaps
      G4Tubs* s_PH1BPIPEendcap = new G4Tubs("s_PH1BPIPEendcap",
                                            content.getLength("endcap_innerRadius")*Unit::cm,
                                            content.getLength("endcap_outerRadius")*Unit::cm,
                                            content.getLength("endcap_hz")*Unit::cm,
                                            startAngle, spanningAngle);

      G4LogicalVolume* l_PH1BPIPEendcap = new G4LogicalVolume(s_PH1BPIPEendcap, geometry::Materials::get(matPipe), "l_PH1BPIPEendcap", 0, m_sensitive);

      //position central endcaps
      G4ThreeVector PH1BPIPEendcapposTop = G4ThreeVector(
                                             content.getLength("x_ph1bpipe") * Unit::cm,
                                             content.getLength("y_ph1bpipe") * Unit::cm,
                                             content.getLength("z_ph1bpipe") * Unit::cm + content.getLength("pipe_hz") * Unit::cm + content.getLength("endcap_hz") * Unit::cm
                                           );

      G4ThreeVector PH1BPIPEendcapposBot = G4ThreeVector(
                                             content.getLength("x_ph1bpipe") * Unit::cm,
                                             content.getLength("y_ph1bpipe") * Unit::cm,
                                             content.getLength("z_ph1bpipe") * Unit::cm - content.getLength("pipe_hz") * Unit::cm - content.getLength("endcap_hz") * Unit::cm
                                           );

      //place the central endcaps
      new G4PVPlacement(0, PH1BPIPEendcapposTop, l_PH1BPIPEendcap, "p_PH1BPIPEendcapTop", &topVolume, false, 1);
      new G4PVPlacement(0, PH1BPIPEendcapposBot, l_PH1BPIPEendcap, "p_PH1BPIPEendcapBot", &topVolume, false, 1);

      //create x shape tubes
      G4VSolid* s_Xshape1 = new G4Tubs("s_Xshape1",
                                       content.getLength("xpipe_innerRadius")*Unit::cm,
                                       content.getLength("xpipe_outerRadius")*Unit::cm,
                                       content.getLength("xpipe_hz")*Unit::cm,
                                       startAngle, spanningAngle);
      G4VSolid* s_Xshape2 = new G4Tubs("s_Xshape2",
                                       content.getLength("xpipe_innerRadius")*Unit::cm,
                                       content.getLength("xpipe_outerRadius")*Unit::cm,
                                       content.getLength("xpipe_hz")*Unit::cm,
                                       startAngle, spanningAngle);
      //make the union
      G4Transform3D transform_X = G4Translate3D(0., 0., 0.);
      transform_X = transform_X * G4RotateY3D(0.083 * Unit::rad);
      G4VSolid* s_X = new G4UnionSolid("s_Xshape1+s_Xshape2", s_Xshape1, s_Xshape2, transform_X);

      //substract central parts
      transform_X = G4Translate3D(
                      content.getLength("x_ph1bpipe") * Unit::cm,
                      content.getLength("y_ph1bpipe") * Unit::cm,
                      content.getLength("z_ph1bpipe") * Unit::cm + content.getLength("pipe_hz") * Unit::cm + content.getLength("endcap_hz") * Unit::cm
                    );
      transform_X = transform_X * G4RotateY3D(0.083 / 2. * Unit::rad);
      s_X = new G4SubtractionSolid("s_X1", s_X, s_PH1BPIPEendcap, transform_X);
      transform_X = G4Translate3D(
                      content.getLength("x_ph1bpipe") * Unit::cm,
                      content.getLength("y_ph1bpipe") * Unit::cm,
                      content.getLength("z_ph1bpipe") * Unit::cm - content.getLength("pipe_hz") * Unit::cm - content.getLength("endcap_hz") * Unit::cm
                    );
      transform_X = transform_X * G4RotateY3D(0.083 / 2. * Unit::rad);
      s_X = new G4SubtractionSolid("s_X2", s_X, s_PH1BPIPEendcap, transform_X);
      transform_X = G4Translate3D(0, 0, 0);
      transform_X = transform_X * G4RotateY3D(0.083 / 2. * Unit::rad);
      s_X = new G4SubtractionSolid("s_X3", s_X, s_PH1BPIPE, transform_X);

      G4LogicalVolume* l_X = new G4LogicalVolume(s_X, geometry::Materials::get(matPipe), "l_X", 0, m_sensitive);

      transform_X = G4Translate3D(0, 0, 0);
      transform_X = transform_X * G4RotateY3D(-0.083 / 2. * Unit::rad);
      new G4PVPlacement(transform_X, l_X, "p_X", &topVolume, false, 1);

    }
  } // ph1bpipe namespace
} // Belle2 namespace
