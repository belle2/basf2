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

#include <tracking/trackFindingCDC/hough/perigee/SimpleHitBasedHough.h>

#include <tracking/trackFindingCDC/hough/perigee/Phi0Sweeped.h>
#include <tracking/trackFindingCDC/hough/perigee/ImpactSweeped.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvSweepedXLine.h>

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>

#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <vector>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  template<class SweepedThingy, size_t ... divisions>
  using SimpleWireHitHough =
    SimpleHitBasedHough<CDCRLTaggedWireHit, SweepedThingy, divisions ... >;

  template<class SweepedThingy, size_t ... divisions>
  using SimpleSegmentHough =
    SimpleHitBasedHough<const CDCRecoSegment2D*, SweepedThingy, divisions ... >;

  using Phi0CurvSweeped = Phi0Sweeped<CurvSweepedXLine>;

  using Phi0ImpactCurvSweeped = Phi0Sweeped<ImpactSweeped<CurvSweepedXLine> >;


  TEST_F(DISABLED_Long_TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_impact_curv_onHits)
  {
    std::string svgFileName = "phi0_impact_curv_on_hits.svg";

    Helix lowerCurvHelix(0.015, 2.52033, -20, 0, 0);
    Helix higherCurvHelix(0.027, 3.0718, 20, 0, 0);

    simulate({lowerCurvHelix, higherCurvHelix});
    getPlotter().save(svgFileName);

    const size_t maxLevel = 13;

    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const size_t impactDivisions = 2;


    const size_t discretePhi0Overlap = 1;
    const size_t discretePhi0Width = 3;
    const size_t nPhi0Bins = std::pow(phi0Divisions, maxLevel);
    Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                              discretePhi0Overlap,
                              discretePhi0Width);

    const double maxCurv = 0.13;
    const double minCurv = -0.018;

    const size_t discreteCurvOverlap = 2;
    const size_t discreteCurvWidth = 5;
    const size_t nCurvBins = std::pow(curvDivisions, maxLevel);
    CurvBinsSpec curvBinsSpec(minCurv,
                              maxCurv,
                              nCurvBins,
                              discreteCurvOverlap,
                              discreteCurvWidth);

    const double maxImpact = 100;
    const double minImpact = -100;

    const size_t discreteImpactOverlap = 1;
    const size_t discreteImpactWidth = 2;
    const size_t nImpactBins = std::pow(impactDivisions, maxLevel);
    ImpactBinsSpec impactBinsSpec(minImpact,
                                  maxImpact,
                                  nImpactBins,
                                  discreteImpactOverlap,
                                  discreteImpactWidth);

    const double curlCurv = 0.018;

    using SimpleWireHitPhi0ImpactCurvHough =
      SimpleWireHitHough<Phi0ImpactCurvSweeped, phi0Divisions, impactDivisions, curvDivisions>;

    SimpleWireHitPhi0ImpactCurvHough houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleWireHitPhi0ImpactCurvHough::HoughBox;

    houghTree.assignArray<0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    //houghTree.assignArray<1>(impactBinsSpec.constructArray(), impactBinsSpec.getNOverlap()); // Discrete
    houghTree.assignArray<1>({minImpact, maxImpact}, impactBinsSpec.getOverlap()); // Continuous
    houghTree.assignArray<2>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      houghTree.fell();
      houghTree.seed(m_axialWireHits);

      const double minWeight = 70.0;
      const double maxCurv = NAN;
      candidates = houghTree.find(minWeight, maxCurv);
      // candidates = houghTree.findBest(minWeight, maxCurv);

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
    for (std::pair<HoughBox, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const HoughBox& houghBox = candidate.first;
      const std::vector<CDCRLTaggedWireHit>& taggedHits = candidate.second;

      B2INFO("Candidate");
      B2INFO("size " << taggedHits.size());
      B2INFO("Phi " << houghBox.getLowerBound<DiscretePhi0>());
      B2INFO("Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2INFO("Impact " << houghBox.getLowerBound<ContinuousImpact>());

      B2INFO("Tags of the hits");
      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2INFO("    " <<
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
        getPlotter().draw(*wireHit, strokeAttr);
      }
      ++iColor;
    }
    getPlotter().save(svgFileName);

    timeItResult.printSummary();
  }
}

namespace {
  TEST_F(DISABLED_Long_TrackFindingCDCTestWithSimpleSimulation, hough_perigee_SimpleHitBasedHough_phi0_impact_curv_onSegment)
  {
    std::string svgFileName = "phi0_impact_curv_on_segments.svg";

    Helix lowerCurvHelix(0.015, 2.52033, -20, 0, 0);
    Helix higherCurvHelix(0.027, 3.0718, 20, 0, 0);

    simulate({lowerCurvHelix, higherCurvHelix});
    getPlotter().save(svgFileName);

    const size_t maxLevel = 13;

    const size_t phi0Divisions = 2;
    const size_t curvDivisions = 2;
    const size_t impactDivisions = 2;

    const size_t discretePhi0Overlap = 1;
    const size_t discretePhi0Width = 3;
    const size_t nPhi0Bins = std::pow(phi0Divisions, maxLevel);
    Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                              discretePhi0Overlap,
                              discretePhi0Width);

    const double maxCurv = 0.13;
    const double minCurv = -0.018;

    const size_t discreteCurvOverlap = 2;
    const size_t discreteCurvWidth = 5;
    const size_t nCurvBins = std::pow(curvDivisions, maxLevel);
    CurvBinsSpec curvBinsSpec(minCurv,
                              maxCurv,
                              nCurvBins,
                              discreteCurvOverlap,
                              discreteCurvWidth);

    const double maxImpact = 100;
    const double minImpact = -100;

    const size_t discreteImpactOverlap = 1;
    const size_t discreteImpactWidth = 2;
    const size_t nImpactBins = std::pow(impactDivisions, maxLevel);
    ImpactBinsSpec impactBinsSpec(minImpact,
                                  maxImpact,
                                  nImpactBins,
                                  discreteImpactOverlap,
                                  discreteImpactWidth);

    const double curlCurv = 0.018;

    using SimpleSegmentPhi0ImpactCurvHough =
      SimpleSegmentHough<Phi0ImpactCurvSweeped, phi0Divisions, impactDivisions, curvDivisions>;

    SimpleSegmentPhi0ImpactCurvHough houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleSegmentPhi0ImpactCurvHough::HoughBox;

    houghTree.assignArray<0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    //houghTree.assignArray<1>(impactBinsSpec.constructArray(), impactBinsSpec.getNOverlap()); // Discrete
    houghTree.assignArray<1>({minImpact, maxImpact}, impactBinsSpec.getOverlap()); // Continuous
    houghTree.assignArray<2>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<const CDCRecoSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();

      houghTree.seed(m_mcAxialSegment2Ds);

      const double minWeight = 70.0;
      const double maxCurv = NAN;
      candidates = houghTree.find(minWeight, maxCurv);
      // candidates = houghTree.findBest(minWeight, maxCurv);

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

      B2INFO("Candidate");
      B2INFO("size " << segments.size());
      B2INFO("Phi " << houghBox.getLowerBound<DiscretePhi0>());
      B2INFO("Curv " << houghBox.getLowerBound<DiscreteCurv>());
      B2INFO("Impact " << houghBox.getLowerBound<ContinuousImpact>());

      for (const CDCRecoSegment2D* recoSegment2D : segments) {
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        getPlotter().draw(*recoSegment2D, strokeAttr);
      }
      ++iColor;
    }
    getPlotter().save(svgFileName);
    timeItResult.printSummary();
  }
}
