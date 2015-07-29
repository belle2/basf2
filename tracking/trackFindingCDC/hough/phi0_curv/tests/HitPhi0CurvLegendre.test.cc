/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>
#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/geometry/Helix.h>
#include <tracking/trackFindingCDC/hough/phi0_curv/HitPhi0CurvLegendre.h>
#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <vector>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, hough_phi0_curv_HitPhi0CurvLegendre_onHits)
  {
    // Prepare event
    CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    CDCSimpleSimulation simpleSimulation(&wireHitTopology);

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.027, 3.0718, 0, 1, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    CDCTrajectory3D lowerCurvTrajectory(lowerCurvOriginHelix);
    CDCTrajectory3D higherCurvTrajectory(higherCurvOriginHelix);

    std::vector<CDCTrajectory3D> trajectories{lowerCurvTrajectory, higherCurvTrajectory};
    std::vector<CDCTrack> mcTracks = simpleSimulation.simulate(trajectories);

    EventDataPlotter plotter;
    plotter.draw(CDCWireTopology::getInstance());
    for (CDCTrack& mcTrack : mcTracks) {
      plotter.draw(mcTrack);
    }

    // plotter.draw(lowerCurvTrajectory.getTrajectory2D());
    // plotter.draw(lowerCurvTrajectory.reversed().getTrajectory2D());
    // plotter.draw(higherCurvTrajectory.getTrajectory2D());

    plotter.save("legendre_event.svg");

    B2INFO("Size mc track 0 : " << mcTracks[0].size());
    B2INFO("Size mc track 1 : " << mcTracks[1].size());

    // Prepare the hough algorithm
    // const size_t maxLevel = 12;
    // const size_t phi0Divisions = 2;
    // const size_t curvDivisions = 2;
    // const double maxCurv = 2.75;
    // const double minCurv = 0;

    const size_t maxLevel = 13;
    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const double maxCurv = 0.13;
    const double minCurv = -0.018;

    // const size_t maxLevel = 9;
    // const size_t phi0Divisions = 3;
    // const size_t curvDivisions = 2;
    // const double maxCurv = 0.13;
    // const double minCurv = -0.018;

    // const size_t maxLevel = 8;
    // const size_t phiDivisions = 3;
    // const size_t curvDivisions = 3;

    using WireHitPhi0CurvQuadLegendre =
      HitPhi0CurvLegendre<RLTagged<const CDCWireHit*>, phi0Divisions, curvDivisions>;
    WireHitPhi0CurvQuadLegendre wireHitPhi0CurvQuadLegendre(maxLevel, minCurv, maxCurv);
    wireHitPhi0CurvQuadLegendre.initialize();

    std::vector<const CDCWireHit*> axialWireHits;
    for (const CDCWireHit& wireHit : wireHitTopology.getWireHits()) {
      if (wireHit.isAxial()) {
        axialWireHits.push_back(&wireHit);
      }
    }

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<Phi0CurvBox, vector<RLTagged<const CDCWireHit*> > > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      wireHitPhi0CurvQuadLegendre.fell();

      wireHitPhi0CurvQuadLegendre.seed(axialWireHits);

      const double minWeight = 30.0;
      const double maxCurv = NAN;
      // candidates = wireHitPhi0CurvQuadLegendre.find(minWeight, maxCurv);
      candidates = wireHitPhi0CurvQuadLegendre.findBest(minWeight, maxCurv);

      // B2INFO("Execution " << iExecution);
      /// Check if exactly two candidates have been found
      ASSERT_EQ(2, candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      EXPECT_GE(candidates[0].second.size(), 30);
      EXPECT_GE(candidates[1].second.size(), 30);

    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = 0;
    using Node = WireHitPhi0CurvQuadLegendre::Node;
    auto countNodes = [&nNodes](Node*) -> bool {
      ++nNodes;
      return true;
    };
    wireHitPhi0CurvQuadLegendre.getTree()->walk(countNodes);
    B2INFO("Tree generated " << nNodes << " nodes");
    wireHitPhi0CurvQuadLegendre.fell();
    wireHitPhi0CurvQuadLegendre.raze();

    for (std::pair<Phi0CurvBox, std::vector<RLTagged<const CDCWireHit*> > >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<RLTagged<const CDCWireHit*> >& taggedHits = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << taggedHits.size());
      B2INFO("Phi " << phi0CurvBox.getLowerPhi0Vec());
      B2INFO("Curv " << phi0CurvBox.getLowerCurv());
      B2INFO("Tags of the hits");

      for (const RLTagged<const CDCWireHit*>& rlTaggedWireHit : taggedHits) {
        B2INFO("    rl = " << rlTaggedWireHit.getRLInfo() <<
               " dl = " << rlTaggedWireHit->getRefDriftLength());
      }
    }

    B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
    B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
           "in " << timeItResult.getNExecutions() << " executions.");

    for (const RLTagged<const CDCWireHit*>& rlTaggedWireHit : candidates.at(0).second) {
      const CDCWireHit* wireHit = rlTaggedWireHit;
      std::string color = "blue";
      if (rlTaggedWireHit.getRLInfo() == RIGHT) {
        color = "green";
      } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
        color = "red";
      }
      EventDataPlotter::AttributeMap rl {{"stroke", color}};
      //EventDataPlotter::AttributeMap rl {{"stroke", "blue"}};
      plotter.draw(*wireHit, rl);
    }
    plotter.save("legendre_event.svg");

    for (const RLTagged<const CDCWireHit*>& rlTaggedWireHit : candidates.at(1).second) {
      const CDCWireHit* wireHit = rlTaggedWireHit;
      std::string color = "blue";
      if (rlTaggedWireHit.getRLInfo() == RIGHT) {
        color = "green";
      } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
        color = "red";
      }
      EventDataPlotter::AttributeMap rl {{"stroke", color}};
      //EventDataPlotter::AttributeMap rl {{"stroke", "red"}};
      plotter.draw(*wireHit, rl);
    }

    plotter.save("legendre_event.svg");

  }
}

namespace {
  TEST_F(DISABLED_Long_TrackFindingCDCTestWithTopology, hough_phi0_curv_HitPhi0CurvLegendre_onSegments)
  {
    // Prepare event
    CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    CDCSimpleSimulation simpleSimulation(&wireHitTopology);

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.027, 3.0718, 0, 1, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    CDCTrajectory3D lowerCurvTrajectory(lowerCurvOriginHelix);
    CDCTrajectory3D higherCurvTrajectory(higherCurvOriginHelix);

    std::vector<CDCTrajectory3D> trajectories{lowerCurvTrajectory, higherCurvTrajectory};
    std::vector<CDCTrack> mcTracks = simpleSimulation.simulate(trajectories);

    EventDataPlotter plotter;
    plotter.draw(CDCWireTopology::getInstance());
    for (CDCTrack& mcTrack : mcTracks) {
      plotter.draw(mcTrack);
    }

    // plotter.draw(lowerCurvTrajectory.getTrajectory2D());
    // plotter.draw(lowerCurvTrajectory.reversed().getTrajectory2D());
    // plotter.draw(higherCurvTrajectory.getTrajectory2D());

    plotter.save("segment_legendre_event.svg");

    B2INFO("Size mc track 0 : " << mcTracks[0].size());
    B2INFO("Size mc track 1 : " << mcTracks[1].size());

    std::vector<CDCRecoSegment2D> recoSegment2Ds;
    for (const CDCTrack& mcTrack : mcTracks) {
      std::vector<CDCRecoSegment3D> recoSegment3DsInTrack = mcTrack.splitIntoSegments();
      for (const CDCRecoSegment3D& recoSegment3D :  recoSegment3DsInTrack) {
        recoSegment2Ds.push_back(recoSegment3D.projectXY());
      }
    }

    // Prepare the hough algorithm
    // const size_t maxLevel = 12;
    // const size_t phi0Divisions = 2;
    // const size_t curvDivisions = 2;
    // const double maxCurv = 2.75;
    // const double minCurv = 0;

    const size_t maxLevel = 13;
    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const double maxCurv = 0.13;
    const double minCurv = -0.018;

    // const size_t maxLevel = 9;
    // const size_t phi0Divisions = 3;
    // const size_t curvDivisions = 2;
    // const double maxCurv = 0.13;
    // const double minCurv = -0.018;

    // const size_t maxLevel = 8;
    // const size_t phiDivisions = 3;
    // const size_t curvDivisions = 3;

    using SegmentPhi0CurvQuadLegendre =
      HitPhi0CurvLegendre<const CDCRecoSegment2D*, phi0Divisions, curvDivisions>;
    SegmentPhi0CurvQuadLegendre segmentPhi0CurvQuadLegendre(maxLevel, minCurv, maxCurv);
    segmentPhi0CurvQuadLegendre.initialize();

    std::vector<const CDCRecoSegment2D*> axialSegments;
    for (const CDCRecoSegment2D& recoSegment2D : recoSegment2Ds) {
      if (recoSegment2D.getStereoType() == AXIAL) {
        axialSegments.push_back(&recoSegment2D);
      }
    }

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<Phi0CurvBox, vector<const CDCRecoSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      segmentPhi0CurvQuadLegendre.fell();

      segmentPhi0CurvQuadLegendre.seed(axialSegments);

      const double minWeight = 30.0;
      const double maxCurv = NAN;
      // candidates = segmentPhi0CurvQuadLegendre.find(minWeight, maxCurv);
      candidates = segmentPhi0CurvQuadLegendre.findBest(minWeight, maxCurv);

      // B2INFO("Execution " << iExecution);
      /// Check if exactly two candidates have been found
      ASSERT_EQ(2, candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      EXPECT_GE(candidates[0].second.size(), 4);
      EXPECT_GE(candidates[1].second.size(), 4);

    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = 0;
    using Node = SegmentPhi0CurvQuadLegendre::Node;
    auto countNodes = [&nNodes](Node*) -> bool {
      ++nNodes;
      return true;
    };
    segmentPhi0CurvQuadLegendre.getTree()->walk(countNodes);
    B2INFO("Tree generated " << nNodes << " nodes");

    segmentPhi0CurvQuadLegendre.fell();
    segmentPhi0CurvQuadLegendre.raze();

    for (std::pair<Phi0CurvBox, std::vector<const CDCRecoSegment2D*> >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<const CDCRecoSegment2D*>& segments = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << segments.size());
      B2INFO("Phi " << phi0CurvBox.getLowerPhi0Vec());
      B2INFO("Curv " << phi0CurvBox.getLowerCurv());
    }

    B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
    B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
           "in " << timeItResult.getNExecutions() << " executions.");

    for (const CDCRecoSegment2D* recoSegment2D : candidates.at(0).second) {
      EventDataPlotter::AttributeMap rl {{"stroke", "blue"}};
      plotter.draw(*recoSegment2D, rl);
    }
    plotter.save("segment_legendre_event.svg");

    for (const CDCRecoSegment2D* recoSegment2D : candidates.at(1).second) {
      EventDataPlotter::AttributeMap rl {{"stroke", "red"}};
      plotter.draw(*recoSegment2D, rl);
    }
    plotter.save("segment_legendre_event.svg");
  }
}
