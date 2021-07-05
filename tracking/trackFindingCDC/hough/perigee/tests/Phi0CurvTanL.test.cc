/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleSegmentHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvTanLBox.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PerigeeBinSpec;

namespace {

  // /// Hough tree depth and divisions
  const int maxLevel = 13;

  // Phi0
  ////////
  const int phi0Divisions = 2;
  const int discretePhi0Overlap = 4;
  const int discretePhi0Width = 5;
  const int nPhi0Bins = std::pow(phi0Divisions, maxLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  discretePhi0Overlap,
                                  discretePhi0Width);


  // Curv
  ////////
  const int curvDivisions = 2;
  const double maxCurv = 0.13;
  const double minCurv = -0.018;
  const double maxCurvAcceptance = 0.13;

  const int discreteCurvOverlap = 4;
  const int discreteCurvWidth = 5;
  const int nCurvBins = std::pow(curvDivisions, maxLevel);
  const CurvBinsSpec curvBinsSpec(minCurv,
                                  maxCurv,
                                  nCurvBins,
                                  discreteCurvOverlap,
                                  discreteCurvWidth);


  // TanL
  ////////
  const int tanLDivisions = 2;
  const double maxTanL = 3.27;
  const double minTanL = -1.73;

  const int discreteTanLOverlap = 1;
  const int discreteTanLWidth = 2;
  const int nTanLBins = std::pow(tanLDivisions, maxLevel);
  ImpactBinsSpec tanLBinsSpec(minTanL,
                              maxTanL,
                              nTanLBins,
                              discreteTanLOverlap,
                              discreteTanLWidth);

  const double curlCurv = 0.018;

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_tanl_hits)
  {
    std::string svgFileName = "phi0_curv_tanl_hits.svg";

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.020, 3.0718, 0, -0.2, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    simulate({lowerCurvOriginHelix, higherCurvOriginHelix});
    saveDisplay(svgFileName);

    using SimpleRLTaggedWireHitPhi0CurvHough =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvTanLBox, phi0Divisions, curvDivisions, tanLDivisions>;
    SimpleRLTaggedWireHitPhi0CurvHough houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleRLTaggedWireHitPhi0CurvHough::HoughBox;

    const double minWeight = 50.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    // houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
    //                  curvBinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurvWithArcLength2DCache>(curvBinsSpec.constructCacheArray(),
                                                            curvBinsSpec.getNOverlap());

    houghTree.assignArray<ContinuousTanL>({{minTanL, maxTanL}}, tanLBinsSpec.getOverlap());
    //houghTree.assignArray<ContinuousTanL>({{0.0, 0.0}}, 0.0);
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<HoughBox, std::vector<CDCRLWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(1, true, [&]() {
      houghTree.fell();
      //houghTree.seed(m_axialWireHits);
      houghTree.seed(m_wireHits);

      // candidates = houghTree.find(minWeight, maxCurvAcceptance);
      candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

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

    int iColor = 0;
    for (std::pair<HoughBox, std::vector<CDCRLWireHit> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<CDCRLWireHit>& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());

      B2DEBUG(100, "Lower Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Upper Phi0 " << houghBox.getUpperBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Lower Curv " << houghBox.getLowerBound<DiscreteCurvWithArcLength2DCache>());
      B2DEBUG(100, "Upper Curv " << houghBox.getUpperBound<DiscreteCurvWithArcLength2DCache>());
      B2DEBUG(100, "Lower TanL " << houghBox.getLowerBound<ContinuousTanL>());
      B2DEBUG(100, "Upper TanL " << houghBox.getUpperBound<ContinuousTanL>());
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

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_tanl_segments)
  {
    std::string svgFileName = "phi0_curv_tanl_segments.svg";

    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.020, 3.0718, 0, -0.2, 0);

    simulate({lowerCurvOriginHelix, higherCurvOriginHelix});
    saveDisplay(svgFileName);

    using SimpleSegmentPhi0CurvHoughTree =
      SimpleSegmentHoughTree<InPhi0CurvTanLBox, phi0Divisions, curvDivisions, tanLDivisions>;
    SimpleSegmentPhi0CurvHoughTree houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleSegmentPhi0CurvHoughTree::HoughBox;

    const double minWeight = 50.0;

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurvWithArcLength2DCache>(curvBinsSpec.constructCacheArray(),
                                                            curvBinsSpec.getNOverlap());

    houghTree.assignArray<ContinuousTanL>({{minTanL, maxTanL}}, tanLBinsSpec.getOverlap());
    houghTree.initialize();

    // Obtain the segments as pointers.
    std::vector<const CDCSegment2D*> m_ptrSegment2Ds;
    m_ptrSegment2Ds.reserve(m_mcSegment2Ds.size());
    for (const CDCSegment2D& segment2D : m_mcSegment2Ds) {
      m_ptrSegment2Ds.push_back(&segment2D);
    }

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<HoughBox, std::vector<const CDCSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(1, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_ptrSegment2Ds);

      //candidates = houghTree.find(minWeight, maxCurvAcceptance);
      candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 4 segment, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 4);
      }
    });

    /// Test idiom to output statistics about the tree.
    int nNodes = houghTree.getTree()->getNNodes();
    B2INFO("Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    int iColor = 0;
    for (std::pair<HoughBox, std::vector<const CDCSegment2D*> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<const CDCSegment2D*>& segments = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << segments.size());
      B2DEBUG(100, "Lower Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Upper Phi0 " << houghBox.getUpperBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Lower Curv " << houghBox.getLowerBound<DiscreteCurvWithArcLength2DCache>());
      B2DEBUG(100, "Upper Curv " << houghBox.getUpperBound<DiscreteCurvWithArcLength2DCache>());
      B2DEBUG(100, "Lower TanL " << houghBox.getLowerBound<ContinuousTanL>());
      B2DEBUG(100, "Upper TanL " << houghBox.getUpperBound<ContinuousTanL>());
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
