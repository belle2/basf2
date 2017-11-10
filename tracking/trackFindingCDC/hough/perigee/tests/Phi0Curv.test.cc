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

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleSegmentHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PerigeeBinSpec;

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_hits)
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

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2DEBUG(100, "Tree generated " << nNodes << " nodes");
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
      B2DEBUG(100, "Tags of the hits");

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
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_on_segments)
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


namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_hits_using_leaf_processing)
  {
    std::string svgFileName = "phi0_curv_on_hits.svg";

    Helix zeroCurvOriginHelix(0.000, 1.52033, 0, -0.5, 0);
    Helix lowerCurvOriginHelix(0.015, 2.52033, 0, 1, 0);
    Helix higherCurvOriginHelix(0.027, 3.0718, 3, 0, 0);

    // Helix lowerCurvOriginHelix(0.012, 1.4, 0, 1, 0);
    // Helix higherCurvOriginHelix(0.027, 3.0718, 0, 0.95, 0);

    simulate({zeroCurvOriginHelix, lowerCurvOriginHelix, higherCurvOriginHelix});
    saveDisplay(svgFileName);

    using SimpleRLTaggedWireHitPhi0CurvHough =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    SimpleRLTaggedWireHitPhi0CurvHough houghTree(maxLevel, curlCurv);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    const double minWeight = 30.0;
    using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
    AxialLegendreLeafProcessor<Node> leafProcessor(maxLevel);

    ModuleParamList moduleParamList;
    const std::string prefix = "";
    leafProcessor.exposeParameters(&moduleParamList, prefix);
    moduleParamList.getParameter<double>("minWeight").setDefaultValue(minWeight);
    moduleParamList.getParameter<double>("maxCurv").setDefaultValue(maxCurvAcceptance);

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = this->timeIt(100, true, [&]() {
      houghTree.fell();

      for (const CDCWireHit* wireHit : m_axialWireHits) {
        AutomatonCell& automatonCell = wireHit->getAutomatonCell();
        automatonCell.unsetTakenFlag();
        automatonCell.unsetTemporaryFlags();
      }

      houghTree.seed(m_axialWireHits);

      leafProcessor.clear();

      // Make two passes - one with more restrictive curvature
      moduleParamList.getParameter<double>("maxCurv").setDefaultValue(curlCurv);
      moduleParamList.getParameter<int>("nRoadSearches").setDefaultValue(2);
      moduleParamList.getParameter<int>("roadLevel").setDefaultValue(4);
      leafProcessor.beginWalk();
      houghTree.findUsing(leafProcessor);

      moduleParamList.getParameter<double>("maxCurv").setDefaultValue(maxCurvAcceptance);
      moduleParamList.getParameter<int>("nRoadSearches").setDefaultValue(3);
      moduleParamList.getParameter<int>("roadLevel").setDefaultValue(0);
      leafProcessor.beginWalk();
      houghTree.findUsing(leafProcessor);

      candidates = leafProcessor.getCandidates();

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
    for (std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit> >& candidate : candidates) {
      const CDCTrajectory2D& trajectory2D = candidate.first;
      const std::vector<CDCRLWireHit >& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Impact " << trajectory2D.getGlobalCircle().impact());
      B2DEBUG(100, "Phi0 " << trajectory2D.getGlobalCircle().phi0());
      B2DEBUG(100, "Curv " << trajectory2D.getCurvature());
      B2DEBUG(100, "Support " << trajectory2D.getSupport());
      B2DEBUG(100, "Tags of the hits");

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    rl = " << static_cast<int>(rlTaggedWireHit.getRLInfo()) <<
                " dl = " << rlTaggedWireHit.getRefDriftLength());
      }

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit wireHit = rlTaggedWireHit.getWireHit();
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
      draw(trajectory2D);
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }
}

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_prepared_event_hits_using_leaf_processing)
  {
    std::string svgFileName = "phi0_curv_on_prepared_event_hits_using_leaf_processing.svg";
    loadPreparedEvent();
    saveDisplay(svgFileName);

    // Prepare the hough algorithm
    using SimpleRLTaggedWireHitPhi0CurvHough =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    SimpleRLTaggedWireHitPhi0CurvHough houghTree(maxLevel, curlCurv);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
    houghTree.initialize();

    const double minWeight = 30.0;
    using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
    AxialLegendreLeafProcessor<Node> leafProcessor(maxLevel);

    ModuleParamList moduleParamList;
    const std::string prefix = "";
    leafProcessor.exposeParameters(&moduleParamList, prefix);
    moduleParamList.getParameter<double>("minWeight").setDefaultValue(minWeight);
    moduleParamList.getParameter<double>("maxCurv").setDefaultValue(maxCurvAcceptance);

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();

      for (const CDCWireHit* wireHit : m_axialWireHits) {
        AutomatonCell& automatonCell = wireHit->getAutomatonCell();
        automatonCell.unsetTakenFlag();
        automatonCell.unsetTemporaryFlags();
      }

      houghTree.seed(m_axialWireHits);

      leafProcessor.clear();

      // Make two passes - one with more restrictive curvature
      moduleParamList.getParameter<double>("maxCurv").setDefaultValue(curlCurv);
      moduleParamList.getParameter<int>("nRoadSearches").setDefaultValue(2);
      moduleParamList.getParameter<int>("roadLevel").setDefaultValue(4);
      leafProcessor.beginWalk();
      houghTree.findUsing(leafProcessor);

      moduleParamList.getParameter<double>("maxCurv").setDefaultValue(maxCurvAcceptance);
      moduleParamList.getParameter<int>("nRoadSearches").setDefaultValue(3);
      moduleParamList.getParameter<int>("roadLevel").setDefaultValue(0);
      leafProcessor.beginWalk();
      houghTree.findUsing(leafProcessor);

      candidates = leafProcessor.getCandidates();

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
    for (std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit> >& candidate : candidates) {
      const CDCTrajectory2D& trajectory2D = candidate.first;
      const std::vector<CDCRLWireHit >& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Impact " << trajectory2D.getGlobalCircle().impact());
      B2DEBUG(100, "Phi0 " << trajectory2D.getGlobalCircle().phi0());
      B2DEBUG(100, "Curv " << trajectory2D.getCurvature());
      B2DEBUG(100, "Support " << trajectory2D.getSupport());
      B2DEBUG(100, "Tags of the hits");

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    rl = " << static_cast<int>(rlTaggedWireHit.getRLInfo()) <<
                " dl = " << rlTaggedWireHit.getRefDriftLength());
      }

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit wireHit = rlTaggedWireHit.getWireHit();
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
      draw(trajectory2D);
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_perigee_phi0_curv_prepared_event_hits)
  {
    std::string svgFileName = "phi0_curv_on_prepared_event_hits.svg";
    loadPreparedEvent();
    saveDisplay(svgFileName);

    using RLTaggedWireHitPhi0CurvHough = SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    using Phi0CurvBox = RLTaggedWireHitPhi0CurvHough::HoughBox;
    RLTaggedWireHitPhi0CurvHough houghTree(maxLevel);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());

    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());

    houghTree.initialize();
    const double minWeight = 30.0;

    // Execute the finding a couple of time to find a stable execution time.
    std::vector< std::pair<Phi0CurvBox, std::vector<CDCRLWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_axialWireHits);

      candidates = houghTree.find(minWeight, maxCurvAcceptance);
      //candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

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
    for (std::pair<Phi0CurvBox, std::vector<CDCRLWireHit> >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<CDCRLWireHit>& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Phi0 " << phi0CurvBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << phi0CurvBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Tags of the hits");

      for (const CDCRLWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    rl = " << static_cast<int>(rlTaggedWireHit.getRLInfo()) <<
                " dl = " << rlTaggedWireHit.getRefDriftLength());
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
