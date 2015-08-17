/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleSegmentHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/hough/perigee/InPhi0CurvBox.h>

#include <vector>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PergieeBinSpec;

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_curv_onHits)
  {
    std::string svgFileName = "phi0_curv_on_hits.svg";

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    simulate({lowerCurvOriginHelix, higherCurvOriginHelix});
    saveDisplay(svgFileName);

    using SimpleRLTaggedWireHitPhi0CurvHough =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    SimpleRLTaggedWireHitPhi0CurvHough houghTree(maxLevel);
    using HoughBox = SimpleRLTaggedWireHitPhi0CurvHough::HoughBox;

    const double minWeight = 30.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();


    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      houghTree.fell();
      houghTree.seed(m_axialWireHits);

      candidates = houghTree.find(minWeight, maxCurvAcceptance);
      // candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());
      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 30);
      }
    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2DEBUG(100, "Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    size_t iColor = 0;
    for (std::pair<HoughBox, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<CDCRLTaggedWireHit>& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Tags of the hits");

      B2DEBUG(100, "Tags of the hits");
      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    " <<
                "rl = " << rlTaggedWireHit.getRLInfo() << " " <<
                "dl = " << rlTaggedWireHit->getRefDriftLength());
      }

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
        std::string color = "blue";
        if (rlTaggedWireHit.getRLInfo() == RIGHT) {
          color = "green";
        } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
          color = "red";
        }
        //EventDataPlotter::AttributeMap strokeAttr {{"stroke", color}};
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*wireHit, strokeAttr);
      }
      ++iColor;
    }

    saveDisplay(svgFileName);

    timeItResult.printSummary();
  }
}

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_curv_onSegments)
  {
    std::string svgFileName = "phi0_curv_on_segments.svg";

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    simulate({lowerCurvOriginHelix, higherCurvOriginHelix});
    saveDisplay(svgFileName);

    using SimpleSegmentPhi0CurvHoughTree =
      SimpleSegmentHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    SimpleSegmentPhi0CurvHoughTree houghTree(maxLevel);
    using HoughBox = SimpleSegmentPhi0CurvHoughTree::HoughBox;

    const double minWeight = 30.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<const CDCRecoSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();

      houghTree.seed(m_mcAxialSegment2Ds);

      candidates = houghTree.find(minWeight, maxCurvAcceptance);
      // candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 4 segment, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 4);
      }
    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2INFO("Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    size_t iColor = 0;
    for (std::pair<HoughBox, std::vector<const CDCRecoSegment2D*> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<const CDCRecoSegment2D*>& segments = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << segments.size());
      B2DEBUG(100, "Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << houghBox.getLowerBound<DiscreteCurv>());

      for (const CDCRecoSegment2D* recoSegment2D : segments) {
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*recoSegment2D, strokeAttr);
      }
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }
}
