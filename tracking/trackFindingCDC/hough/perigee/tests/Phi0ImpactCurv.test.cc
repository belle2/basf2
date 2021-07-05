/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleSegmentHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/hough/algorithms/InPhi0ImpactCurvBox.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PerigeeBinSpec;

namespace {

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_impact_curv_onHits)
  {
    std::string svgFileName = "phi0_impact_curv_on_hits.svg";

    Helix lowerCurvHelix(0.015, 2.52033, -20, 0, 0);
    Helix higherCurvHelix(0.027, 3.0718, 20, 0, 0);

    // Helix lowerCurvHelix(0.015, 2.52033, 0, 0, 0);
    // Helix higherCurvHelix(0.027, 3.0718, 0, 0, 0);


    simulate({lowerCurvHelix, higherCurvHelix});
    saveDisplay(svgFileName);

    using SimpleWireHitPhi0ImpactCurvHoughTree =
      SimpleRLTaggedWireHitHoughTree<InPhi0ImpactCurvBox,  phi0Divisions, impactDivisions, curvDivisions>;

    SimpleWireHitPhi0ImpactCurvHoughTree houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleWireHitPhi0ImpactCurvHoughTree::HoughBox;

    const double minWeight = 70.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    //houghTree.assignArray<DiscreteImpact>(impactBinsSpec.constructArray(), impactBinsSpec.getNOverlap()); // Discrete
    houghTree.assignArray<ContinuousImpact>({{minImpact, maxImpact}}, impactBinsSpec.getOverlap()); // Continuous
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<HoughBox, std::vector<CDCRLWireHit> > > candidates;

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

    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2INFO("Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();


    size_t iColor = 0;
    for (std::pair<HoughBox, std::vector<CDCRLWireHit> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<CDCRLWireHit>& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Impact " << houghBox.getLowerBound<ContinuousImpact>());

      B2DEBUG(100, "Tags of the hits");
      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    " <<
                "rl = " << static_cast<int>(rlTaggedWireHit.getRLInfo()) << " " <<
                "dl = " << rlTaggedWireHit.getRefDriftLength());
      }

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit& wireHit = rlTaggedWireHit.getWireHit();
        std::string color = "blue";
        if (rlTaggedWireHit.getRLInfo() == ERightLeft::c_Right) {
          color = "green";
        } else if (rlTaggedWireHit.getRLInfo() == ERightLeft::c_Left) {
          color = "red";
        }
        //EventDataPlotter::AttributeMap strokeAttr {{"stroke", color}};
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(wireHit, strokeAttr);
      }
      ++iColor;
    }
    saveDisplay(svgFileName);

    timeItResult.printSummary();
  }
}

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_impact_curv_onSegment)
  {
    std::string svgFileName = "phi0_impact_curv_on_segments.svg";

    Helix lowerCurvHelix(0.015, 2.52033, -20, 0, 0);
    Helix higherCurvHelix(0.027, 3.0718, 20, 0, 0);

    simulate({lowerCurvHelix, higherCurvHelix});
    saveDisplay(svgFileName);

    using SimpleSegmentPhi0ImpactCurvHoughTree =
      SimpleSegmentHoughTree<InPhi0ImpactCurvBox, phi0Divisions, impactDivisions, curvDivisions>;

    SimpleSegmentPhi0ImpactCurvHoughTree houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleSegmentPhi0ImpactCurvHoughTree::HoughBox;

    const double minWeight = 70.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    //houghTree.assignArray<DiscreteImpact>(impactBinsSpec.constructArray(), impactBinsSpec.getNOverlap()); // Discrete
    houghTree.assignArray<ContinuousImpact>({{minImpact, maxImpact}}, impactBinsSpec.getOverlap()); // Continuous
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<HoughBox, std::vector<const CDCSegment2D*> > > candidates;

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
    for (std::pair<HoughBox, std::vector<const CDCSegment2D*> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<const CDCSegment2D*>& segments = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << segments.size());
      B2DEBUG(100, "Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Impact " << houghBox.getLowerBound<ContinuousImpact>());

      for (const CDCSegment2D* segment2D : segments) {
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*segment2D, strokeAttr);
      }
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }


  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_impact_curv_prepared_event_segments)
  {
    std::string svgFileName = "phi0_impact_curv_on_prepared_event_segments.svg";
    loadPreparedEvent();
    saveDisplay(svgFileName);

    using SimpleSegmentPhi0ImpactCurvHoughTree =
      SimpleSegmentHoughTree<InPhi0ImpactCurvBox, phi0Divisions, impactDivisions, curvDivisions>;

    using HoughBox = SimpleSegmentPhi0ImpactCurvHoughTree::HoughBox;
    SimpleSegmentPhi0ImpactCurvHoughTree houghTree(maxLevel);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());
    houghTree.assignArray<ContinuousImpact>({{minImpact, maxImpact}},
    impactBinsSpec.getOverlap()); // Continuous
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());

    houghTree.initialize();
    const double minWeight = 30.0;

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<HoughBox, std::vector<const CDCSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(1, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_mcAxialSegment2Ds);

      // candidates = houghTree.find(minWeight, maxCurvAcceptance);
      candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());
      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 3);
      }
    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2DEBUG(100, "Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    size_t iColor = 0;
    for (std::pair<HoughBox, std::vector<const CDCSegment2D*> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<const CDCSegment2D*>& segments = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << segments.size());
      B2DEBUG(100, "Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Impact " << houghBox.getLowerBound<ContinuousImpact>());

      for (const CDCSegment2D* segment2D : segments) {
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*segment2D, strokeAttr);
      }
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }

}
