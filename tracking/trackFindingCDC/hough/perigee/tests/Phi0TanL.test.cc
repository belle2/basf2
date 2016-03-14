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

//#include <tracking/trackFindingCDC/hough/perigee/Phi0Sweeped.h>
//#include <tracking/trackFindingCDC/hough/perigee/TanLSweeped.h>
//#include <tracking/trackFindingCDC/hough/perigee/XLine.h>

//#include <tracking/trackFindingCDC/hough/perigee/TanLPhi0SweepedXLine.h>
//#include <tracking/trackFindingCDC/hough/perigee/TanLSweepedXLine.h>
//#include <tracking/trackFindingCDC/hough/perigee/Phi0TanLSweepedXLine.h>

#include <tracking/trackFindingCDC/hough/perigee/InPhi0TanLBox.h>

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/TanLRep.h>
#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>

#include <vector>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  // const size_t maxLevel = 7;
  // const size_t phi0Divisions = 7;
  // const size_t tanLDivisions = 3;

  // const size_t maxLevel = 5;
  // const size_t phi0Divisions = 7;
  // const size_t tanLDivisions = 6;

  // const size_t maxLevel = 9;
  // const size_t phi0Divisions = 3;
  // const size_t tanLDivisions = 2;


  // const size_t maxLevel = 13;
  // const size_t phi0Divisions = 2;
  // const size_t tanLDivisions = 2;

  // const size_t maxLevel = 8;
  // const size_t phi0Divisions = 3;
  // const size_t tanLDivisions = 3;

  // const size_t maxLevel = 7;
  // const size_t phi0Divisions = 4;
  // const size_t tanLDivisions = 3;

  // const size_t discretePhi0Overlap = 4;
  // const size_t discretePhi0Width = 5;

  const size_t maxLevel = 13;
  const size_t phi0Divisions = 2;
  const size_t tanLDivisions = 2;

  const size_t discretePhi0Overlap = 4;
  const size_t discretePhi0Width = 5;

  const size_t nPhi0Bins = std::pow(phi0Divisions, maxLevel);
  Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                            discretePhi0Overlap,
                            discretePhi0Width);

  const double maxTanL = 3.27;
  const double minTanL = -1.73;

  const size_t discreteTanLOverlap = 1;
  const size_t discreteTanLWidth = 2;
  const size_t nTanLBins = std::pow(tanLDivisions, maxLevel);
  ImpactBinsSpec tanLBinsSpec(minTanL,
                              maxTanL,
                              nTanLBins,
                              discreteTanLOverlap,
                              discreteTanLWidth);

  const double curlCurv = 0.018;


  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_tanl_on_hits)
  {
    std::string svgFileName = "phi0_tanl_on_hits.svg";

    Helix originLine0(0.0, -1.0, 0, 0.2, 0);
    Helix originLine1(0.0, 1.0, 0, -0.5, 0);

    simulate({originLine0, originLine1});
    saveDisplay(svgFileName);

    using SimpleRLTaggedWireHitPhi0TanLHoughTree =
      SimpleRLTaggedWireHitHoughTree< InPhi0TanLBox, phi0Divisions, tanLDivisions>;

    SimpleRLTaggedWireHitPhi0TanLHoughTree houghTree(maxLevel, 0.018);
    using HoughBox = SimpleRLTaggedWireHitPhi0TanLHoughTree::HoughBox;

    houghTree.assignArray<ContinuousTanL>({{minTanL, maxTanL}}, tanLBinsSpec.getOverlap());
    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());

    houghTree.initialize();

    std::vector<CDCRLTaggedWireHit> mcTaggedWireHits;
    for (const CDCTrack& mcTrack : m_mcTracks) {
      for (const CDCRecoHit3D& recoHit3D : mcTrack) {
        const CDCWireHit& wireHit = recoHit3D.getWireHit();
        mcTaggedWireHits.push_back(&wireHit);
        mcTaggedWireHits.back().setRLInfo(recoHit3D.getRLInfo());
      }
    }

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      houghTree.fell();

      //houghTree.seed(m_wireHits);
      houghTree.seed(mcTaggedWireHits);

      const double minWeight = 50.0;
      // candidates = houghTree.find(minWeight);
      candidates = houghTree.findBest(minWeight);

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

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Lower Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Upper Phi0 " << houghBox.getUpperBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Lower TanL " << houghBox.getLowerBound<ContinuousTanL>());
      B2DEBUG(100, "Upper TanL " << houghBox.getUpperBound<ContinuousTanL>());


      B2DEBUG(100, "Tags of the hits");
      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    " <<
                "rl = " << rlTaggedWireHit.getRLInfo() << " " <<
                "dl = " << rlTaggedWireHit->getRefDriftLength());
      }

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
        std::string color = "blue";
        if (rlTaggedWireHit.getRLInfo() == ERightLeft::c_Right) {
          color = "green";
        } else if (rlTaggedWireHit.getRLInfo() == ERightLeft::c_Left) {
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

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_tanl_on_segments)
  {
    std::string svgFileName = "phi0_tanl_on_segments.svg";

    Helix originLine0(0.0, -1.0, 0, 0.2, 0);
    Helix originLine1(0.0, 1.0, 0, -0.5, 0);

    simulate({originLine0, originLine1});
    saveDisplay(svgFileName);

    using SimpleSegmentPhi0TanLHoughTree =
      SimpleSegmentHoughTree<InPhi0TanLBox, phi0Divisions, tanLDivisions>;

    SimpleSegmentPhi0TanLHoughTree houghTree(maxLevel, curlCurv);
    using HoughBox = SimpleSegmentPhi0TanLHoughTree::HoughBox;

    houghTree.assignArray<ContinuousTanL>({{minTanL, maxTanL}}, tanLBinsSpec.getOverlap());
    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());

    houghTree.initialize();

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<HoughBox, vector<const CDCRecoSegment2D*> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_mcSegment2Ds);

      const double minWeight = 50.0;
      // candidates = houghTree.find(minWeight);
      candidates = houghTree.findBest(minWeight);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());

      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 4 segment, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 7);
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
      B2DEBUG(100, "Lower Phi0 " << houghBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Upper Phi0 " << houghBox.getUpperBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Lower TanL " << houghBox.getLowerBound<ContinuousTanL>());
      B2DEBUG(100, "Upper TanL " << houghBox.getUpperBound<ContinuousTanL>());

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
