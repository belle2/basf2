/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>
#include <tracking/trackFindingCDC/display/EventDataPlotter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_straight)
{
  Helix straightOriginLine(0, 0, 0, 0, 0);
  simulate({straightOriginLine});
  saveDisplay("straight.svg");
}


TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_high)
{
  Helix lowCurvOriginHelix(0.015, 0, 0, 1, 0);
  simulate({lowCurvOriginHelix});

  for (const CDCRecoHit3D& recoHit3D : m_mcTracks[0]) {
    EventDataPlotter::AttributeMap rl {{"stroke", recoHit3D.getRLInfo() == ERightLeft::c_Right ? "green" : "red"}};
    draw(recoHit3D, rl);
  }

  saveDisplay("low.svg");
}


TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_cosmic)
{
  Helix straightOffOriginLine(0, 1, 20, 1, 75);
  double outerWallR = CDCWireTopology::getInstance().getOuterCylindricalR();
  double arcLengthToOuterWall =
    straightOffOriginLine.arcLength2DToCylindricalR(outerWallR);

  Vector3D startPoint = straightOffOriginLine.atArcLength2D(-arcLengthToOuterWall);
  CDCTrajectory3D cosmicTrajectory(straightOffOriginLine);
  cosmicTrajectory.setLocalOrigin(startPoint);

  simulate({cosmicTrajectory});
  saveDisplay("cosmic.svg");
}


TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_curl)
{
  Helix highCurvOriginHelix(0.02, 1, 0, 0.5, 0);
  simulate({highCurvOriginHelix});
  saveDisplay("curl.svg");
}

TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_secondary_curl)
{
  Helix highCurvOffOriginHelix(0.02, 1, -30, 0.2, 0);
  simulate({highCurvOffOriginHelix});
  saveDisplay("secondary_curl.svg");
}


TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_photon_conversion)
{
  // Should probably be placed at a vxd ladder
  Vector3D vertex(1.0, 0.0, 0.0);
  Vector3D momentum(1.0, 0.0, 0.0);
  double time = 0.0;

  CDCTrajectory3D electronTrajectory(vertex, time, momentum, -1, 1.5);
  CDCTrajectory3D positronTrajectory(vertex, time, momentum, 1, 1.5);

  simulate({electronTrajectory, positronTrajectory});
  saveDisplay("photon_conversion.svg");
}

TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_CDCSimpleSimulation_cosmic_with_delay)
{
  m_simpleSimulation.activateTOFDelay();
  m_simpleSimulation.activateInWireSignalDelay();

  Helix straightOffOriginLine(0, 1, 20, 1, 50);
  double outerWallR = CDCWireTopology::getInstance().getOuterCylindricalR();
  double arcLengthToOuterWall =
    straightOffOriginLine.arcLength2DToCylindricalR(outerWallR);

  Vector3D startPoint = straightOffOriginLine.atArcLength2D(-arcLengthToOuterWall);
  CDCTrajectory3D cosmicTrajectory(straightOffOriginLine);
  cosmicTrajectory.setLocalOrigin(startPoint);


  CDCTrack mcTrack = m_simpleSimulation.simulate(cosmicTrajectory);
  draw(CDCWireTopology::getInstance());
  draw(mcTrack);
  saveDisplay("cosmic_with_delay.svg");
}


TEST_F(TrackFindingCDCTestWithSimpleSimulation, sim_prepared_event_rl_flags)
{
  std::string svgFileName = "rl_flags_prepared_event.svg";
  loadPreparedEvent();

  for (CDCTrack& track : m_mcTracks) {
    for (const CDCRecoHit3D& recoHit3D : track) {
      const CDCRLWireHit& rlWireHit = recoHit3D.getRLWireHit();
      std::string color = "blue";
      if (rlWireHit.getRLInfo() == ERightLeft::c_Right) {
        color = "green";
      } else if (rlWireHit.getRLInfo() == ERightLeft::c_Left) {
        color = "red";
      }
      EventDataPlotter::AttributeMap strokeAttr {{"stroke", color}};
      draw(rlWireHit.getWireHit(), strokeAttr);
    }
  }
  saveDisplay(svgFileName);
}
