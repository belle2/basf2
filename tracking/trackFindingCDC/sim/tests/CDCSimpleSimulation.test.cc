/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>
#include <tracking/trackFindingCDC/display/EventDataPlotter.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_straight)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  Helix straightOriginLine(0, 0, 0, 0, 0);

  CDCTrajectory3D straightTrajectory(straightOriginLine);
  CDCTrack mcTrack = simpleSimulation.simulate(straightTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  plotter.draw(mcTrack);
  plotter.save("straight.svg");
}


TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_high)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  Helix lowCurvOriginHelix(0.015, 0, 0, 1, 0);

  CDCTrajectory3D straightTrajectory(lowCurvOriginHelix);
  CDCTrack mcTrack = simpleSimulation.simulate(straightTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  for (CDCRecoHit3D& recoHit3D : mcTrack) {
    EventDataPlotter::AttributeMap rl {{"stroke", recoHit3D.getRLInfo() == RIGHT ? "green" : "red"}};
    plotter.draw(recoHit3D, rl);
  }
  plotter.save("low.svg");
}


TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_cosmic)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  Helix straightOffOriginLine(0, 1, 20, 1, 75);
  double outerWallR = CDCWireTopology::getInstance().getOuterCylindricalR();
  double arcLengthToOuterWall =
    straightOffOriginLine.arcLength2DToCylindricalR(outerWallR);

  Vector3D startPoint = straightOffOriginLine.atArcLength2D(-arcLengthToOuterWall);
  CDCTrajectory3D cosmicTrajectory(straightOffOriginLine);
  cosmicTrajectory.setLocalOrigin(startPoint);

  CDCTrack mcTrack = simpleSimulation.simulate(cosmicTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  plotter.draw(mcTrack);
  plotter.save("cosmic.svg");
}


TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_curl)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  Helix highCurvOriginHelix(0.02, 1, 0, 0.5, 0);

  CDCTrajectory3D straightTrajectory(highCurvOriginHelix);
  CDCTrack mcTrack = simpleSimulation.simulate(straightTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  plotter.draw(mcTrack);
  plotter.save("curl.svg");
}

TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_secondary_curl)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  Helix highCurvOffOriginHelix(0.02, 1, -30, 0.2, 0);

  CDCTrajectory3D straightTrajectory(highCurvOffOriginHelix);
  CDCTrack mcTrack = simpleSimulation.simulate(straightTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  plotter.draw(mcTrack);
  plotter.save("secondary_curl.svg");
}


TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_photon_conversion)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);

  // Should probably be placed at a vxd ladder
  Vector3D vertex(1.0, 0.0, 0.0);
  Vector3D momentum(1.0, 0.0, 0.0);

  CDCTrajectory3D electronTrajectory(vertex, momentum, -1, 1.5);
  CDCTrajectory3D positronTrajectory(vertex, momentum, 1, 1.5);

  std::vector<CDCTrajectory3D> trajectories{electronTrajectory, positronTrajectory};

  std::vector<CDCTrack> mcTracks = simpleSimulation.simulate(trajectories);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  for (CDCTrack& mcTrack : mcTracks) {
    plotter.draw(mcTrack);
  }
  plotter.save("photon_conversion.svg");
}

TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, eventtopology_CDCSimpleSimulation_cosmic_with_delay)
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCSimpleSimulation simpleSimulation(&wireHitTopology);
  simpleSimulation.activateTOFDelay();
  simpleSimulation.activateInWireSignalDelay();

  Helix straightOffOriginLine(0, 1, 20, 1, 50);
  double outerWallR = CDCWireTopology::getInstance().getOuterCylindricalR();
  double arcLengthToOuterWall =
    straightOffOriginLine.arcLength2DToCylindricalR(outerWallR);

  Vector3D startPoint = straightOffOriginLine.atArcLength2D(-arcLengthToOuterWall);
  CDCTrajectory3D cosmicTrajectory(straightOffOriginLine);
  cosmicTrajectory.setLocalOrigin(startPoint);

  CDCTrack mcTrack = simpleSimulation.simulate(cosmicTrajectory);

  EventDataPlotter plotter;
  plotter.draw(CDCWireTopology::getInstance());
  plotter.draw(mcTrack);
  plotter.save("cosmic_with_delay.svg");
}
