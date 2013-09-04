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
    geometry::CreatorFactory<Ph1bpipeCreator> Ph1bpipeFactory("PH1BPIPECreator");

    Ph1bpipeCreator::Ph1bpipeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    Ph1bpipeCreator::~Ph1bpipeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1bpipeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * Unit::um);

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
        G4double EndcapinnerRadius = 0.;
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
    }
  } // ph1bpipe namespace
} // Belle2 namespace
