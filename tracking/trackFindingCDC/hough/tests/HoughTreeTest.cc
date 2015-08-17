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

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>

#include <tracking/trackFindingCDC/hough/perigee/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>

#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/TimeIt.h>
#include <set>
#include <vector>

#include <gtest/gtest.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PerigeeBinSpec;

namespace {

  TEST_F(CDCLegendreTestFixture, hough_HitPhi0CurvLegendre)
  {
    // Prepare the hough algorithm
    using WireHitPhi0CurvQuadLegendre =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    using Node = typename WireHitPhi0CurvQuadLegendre::Node;
    WireHitPhi0CurvQuadLegendre houghTree(maxLevel, curlCurv);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());

    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());
    houghTree.initialize();

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
      houghTree.fell();
      houghTree.seed(hitVector);

      const double minWeight = 30.0;
      const double maxCurv = 0.13;
      AxialLegendreLeafProcessor<Node> m_leafProcessor(maxLevel);
      m_leafProcessor.setMinWeight(minWeight);
      m_leafProcessor.setMaxCurv(maxCurv);
      houghTree.findUsing(m_leafProcessor);
      candidates = m_leafProcessor.getCandidates();

      // B2INFO("Execution " << iExecution);
      /// Check if exactly two candidates have been found
      ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      EXPECT_GE(candidates[0].second.size(), 30);
      EXPECT_GE(candidates[1].second.size(), 30);

    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2INFO("Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

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

    using RLTaggedWireHitPhi0CurvHough = SimpleHitBasedHoughTree<CDCRLTaggedWireHit, InPhi0CurvBox, phi0Divisions, curvDivisions>;
    using Phi0CurvBox = Box<DiscretePhi0, DiscreteCurv>;
    RLTaggedWireHitPhi0CurvHough houghTree(maxLevel);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());

    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());

    houghTree.initialize();

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
      houghTree.fell();
      houghTree.seed(axialWireHits);

      const double minWeight = 30.0;
      const double maxCurv = 0.13;
      candidates = houghTree.find(minWeight, maxCurv);
      //candidates = houghTree.findBest(minWeight, maxCurv);

      // B2INFO("Execution " << iExecution);
      /// Check if exactly two candidates have been found
      ASSERT_EQ(2, candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      EXPECT_GE(candidates[0].second.size(), 30);
      EXPECT_GE(candidates[1].second.size(), 30);

    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2INFO("Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    for (std::pair<Phi0CurvBox, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<CDCRLTaggedWireHit>& taggedHits = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << taggedHits.size());
      B2INFO("Phi " << phi0CurvBox.getLowerBound<DiscretePhi0>()->phi());
      B2INFO("Curv " << phi0CurvBox.getLowerBound<DiscreteCurv>());
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
