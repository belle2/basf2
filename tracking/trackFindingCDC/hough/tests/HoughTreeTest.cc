/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/hough/phi0_curv/HitPhi0CurvLegendre.h>
#include <tracking/trackFindingCDC/hough/phi0_curv/HitPhi0CurvLegendre.icc.h>

#include <tracking/trackFindingCDC/hough/phi0_curv/SimpleHitBasedPhi0CurvHough.h>

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/TimeIt.h>
#include <set>
#include <vector>

#include <gtest/gtest.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST_F(CDCLegendreTestFixture, hough_HitPhi0CurvLegendre)
  {
    // Prepare the hough algorithm
    const size_t maxLevel = 13;
    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const double maxCurv = 2.75;
    const double minCurv = -0.018;

    // const size_t maxLevel = 8;
    // const size_t phiDivisions = 3;
    // const size_t curvDivisions = 3;

    using WireHitPhi0CurvQuadLegendre = HitPhi0CurvLegendre<phi0Divisions, curvDivisions>;
    WireHitPhi0CurvQuadLegendre wireHitPhi0CurvQuadLegendre(maxLevel, minCurv, maxCurv);
    wireHitPhi0CurvQuadLegendre.initialize();

    // Get the hits form the test event
    markAllHitsAsUnused();
    std::set<TrackHit*>& hits_set = getHitSet();

    EventDataPlotter plotter;
    plotter.draw(CDCWireTopology::getInstance());
    for (TrackHit* trackHit : hits_set) {
      plotter.draw(*(trackHit->getUnderlayingCDCWireHit()));
    }
    plotter.save("org_legendre_event.svg");

    std::vector<const CDCWireHit*> hitVector;
    for (TrackHit* trackHit : hits_set) {
      if (trackHit->getSuperlayerId() % 2 == 0)
        hitVector.push_back(trackHit->getUnderlayingCDCWireHit());
    }

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<CDCTrajectory2D, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      wireHitPhi0CurvQuadLegendre.fell();
      wireHitPhi0CurvQuadLegendre.seed(hitVector);

      const double minWeight = 30.0;
      const double maxCurv = 0.13;
      wireHitPhi0CurvQuadLegendre.findWithCoprocessing(minWeight, maxCurv);

      candidates = wireHitPhi0CurvQuadLegendre.getCandidates();

      // B2INFO("Execution " << iExecution);
      /// Check if exactly two candidates have been found
      ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

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


    for (std::pair<CDCTrajectory2D, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const CDCTrajectory2D& trajectory2D = candidate.first;
      const std::vector<CDCRLTaggedWireHit >& taggedHits = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << taggedHits.size());
      B2INFO("Phi " << trajectory2D.getGlobalCircle().tangentialPhi());
      B2INFO("Curv " << trajectory2D.getCurvature());
      B2INFO("Tags of the hits");

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2INFO("    rl = " << rlTaggedWireHit.getRLInfo() <<
               " dl = " << rlTaggedWireHit->getRefDriftLength());
      }
    }

    for (const CDCRLTaggedWireHit& rlTaggedWireHit : candidates.at(0).second) {
      const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
      std::string color = "blue";
      if (rlTaggedWireHit.getRLInfo() == RIGHT) {
        color = "green";
      } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
        color = "red";
      }
      //EventDataPlotter::AttributeMap rl {{"stroke", color}};
      EventDataPlotter::AttributeMap rl {{"stroke", "blue"}};
      plotter.draw(*wireHit, rl);
    }

    const CDCTrajectory2D& firstTrajectory = candidates.at(0).first;
    plotter.draw(firstTrajectory);
    plotter.save("org_legendre_event.svg");

    for (const CDCRLTaggedWireHit& rlTaggedWireHit : candidates.at(1).second) {
      const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
      std::string color = "blue";
      if (rlTaggedWireHit.getRLInfo() == RIGHT) {
        color = "green";
      } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
        color = "red";
      }
      // EventDataPlotter::AttributeMap rl {{"stroke", color}};
      EventDataPlotter::AttributeMap rl {{"stroke", "red"}};
      plotter.draw(*wireHit, rl);
    }
    const CDCTrajectory2D& secondTrajectory = candidates.at(1).first;
    plotter.draw(secondTrajectory);
    plotter.save("org_legendre_event.svg");



    B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
    B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
           "in " << timeItResult.getNExecutions() << " executions.");
  }


  TEST_F(CDCLegendreTestFixture, hough_phi0_curv_SimpleHitBasedPhi0CurvHough_onLegendreEvent)
  {
    // Prepare event
    // Get the hits form the test event
    markAllHitsAsUnused();
    std::set<TrackHit*>& hits_set = getHitSet();

    EventDataPlotter plotter;
    plotter.draw(CDCWireTopology::getInstance());
    for (TrackHit* trackHit : hits_set) {
      plotter.draw(*(trackHit->getUnderlayingCDCWireHit()));
    }
    plotter.save("org_legendre_event2.svg");

    std::vector<const CDCWireHit*> wireHits;
    for (TrackHit* trackHit : hits_set) {
      if (trackHit->getSuperlayerId() % 2 == 0)
        wireHits.push_back(trackHit->getUnderlayingCDCWireHit());
    }

    const size_t maxLevel = 13;
    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const double maxCurv = 2.75;
    const double minCurv = -0.018;

    using RLTaggedWireHitPhi0CurvHough =
      SimpleHitBasedPhi0CurvHough<CDCRLTaggedWireHit, phi0Divisions, curvDivisions>;
    RLTaggedWireHitPhi0CurvHough rlTaggedWireHitPhi0CurvHough(maxLevel, minCurv, maxCurv);
    rlTaggedWireHitPhi0CurvHough.initialize();

    std::vector<const CDCWireHit*> axialWireHits;
    for (const CDCWireHit* wireHit : wireHits) {
      if (wireHit->isAxial()) {
        axialWireHits.push_back(wireHit);
      }
    }

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<Phi0CurvBox, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      rlTaggedWireHitPhi0CurvHough.fell();
      rlTaggedWireHitPhi0CurvHough.seed(axialWireHits);

      const double minWeight = 30.0;
      const double maxCurv = 0.13;
      candidates = rlTaggedWireHitPhi0CurvHough.find(minWeight, maxCurv);
      //candidates = rlTaggedWireHitPhi0CurvHough.findBest(minWeight, maxCurv);

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
    using Node = RLTaggedWireHitPhi0CurvHough::Node;
    auto countNodes = [&nNodes](Node*) -> bool {
      ++nNodes;
      return true;
    };
    rlTaggedWireHitPhi0CurvHough.getTree()->walk(countNodes);
    B2INFO("Tree generated " << nNodes << " nodes");
    rlTaggedWireHitPhi0CurvHough.fell();
    rlTaggedWireHitPhi0CurvHough.raze();

    for (std::pair<Phi0CurvBox, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<CDCRLTaggedWireHit>& taggedHits = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << taggedHits.size());
      B2INFO("Phi " << phi0CurvBox.getLowerPhi0Vec());
      B2INFO("Curv " << phi0CurvBox.getLowerCurv());
      B2INFO("Tags of the hits");

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2INFO("    rl = " << rlTaggedWireHit.getRLInfo() <<
               " dl = " << rlTaggedWireHit->getRefDriftLength());
      }
    }

    B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
    B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
           "in " << timeItResult.getNExecutions() << " executions.");

  }
}
