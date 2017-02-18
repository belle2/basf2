/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNode.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <set>
#include <cmath>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, legendre_QuadTreeTest)
  {
    using XYSpans = AxialHitQuadTreeProcessor::XYSpans;
    const int maxTheta = std::pow(2, PrecisionUtil::getLookupGridLevel());
    // high-pt candidate
    XYSpans ranges1({0, maxTheta}, {0., 0.15});
    PrecisionUtil::PrecisionFunction highPtPrecisionFunction = &PrecisionUtil::getOriginCurvPrecision;

    // low-pt candidate
    XYSpans ranges2({0, maxTheta}, {0., 0.30});
    PrecisionUtil::PrecisionFunction lowPtPrecisionFunction = &PrecisionUtil::getNonOriginCurvPrecision;

    std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;

    this->loadPreparedEvent();
    const int numberOfPossibleTrackCandidate = m_mcTracks.size();

    AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdProcessor =
      [&candidates](const AxialHitQuadTreeProcessor::ReturnList & hits,
    AxialHitQuadTreeProcessor::QuadTree*) { candidates.push_back(hits); };

    auto now = std::chrono::high_resolution_clock::now();
    AxialHitQuadTreeProcessor qtProcessor1(13, ranges1, highPtPrecisionFunction);
    qtProcessor1.provideItemsSet(m_axialWireHits);

    // actual filling of the hits into the quad tree structure
    qtProcessor1.fillGivenTree(lmdProcessor, 30);

    AxialHitQuadTreeProcessor qtProcessor2(11, ranges2, lowPtPrecisionFunction);
    qtProcessor2.provideItemsSet(m_axialWireHits);

    // actual filling of the hits into the quad tree structure
    qtProcessor2.fillGivenTree(lmdProcessor, 30);
    auto later = std::chrono::high_resolution_clock::now();

    ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

    std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
    B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidates.size() << " candidates");

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].size(), 30);
    EXPECT_GE(candidates[1].size(), 30);
  }
}
