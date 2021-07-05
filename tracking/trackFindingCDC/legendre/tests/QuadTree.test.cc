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
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <vector>
#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST_F(TrackFindingCDCTestWithSimpleSimulation, legendre_QuadTreeTest)
  {
    using XYSpans = AxialHitQuadTreeProcessor::XYSpans;
    const int maxTheta = std::pow(2, PrecisionUtil::getLookupGridLevel());

    // high-pt candidate
    XYSpans xySpans1({0, maxTheta}, {0., 0.15});
    PrecisionUtil::PrecisionFunction highPtPrecisionFunction = &PrecisionUtil::getOriginCurvPrecision;

    // low-pt candidate
    XYSpans xySpans2({0, maxTheta}, {0., 0.30});
    PrecisionUtil::PrecisionFunction lowPtPrecisionFunction = &PrecisionUtil::getNonOriginCurvPrecision;

    using Candidate = std::vector<const CDCWireHit*>;
    std::vector<Candidate> candidates;

    this->loadPreparedEvent();
    const int nMCTracks = m_mcTracks.size();

    auto candidateReceiver = [&candidates](const Candidate & candidate, void*) {
      candidates.push_back(candidate);
    };

    TimeItResult timeItResult = timeIt(100, true, [&]() {
      candidates.clear();
      for (const CDCWireHit* wireHit : m_axialWireHits) {
        (*wireHit)->unsetTakenFlag();
        (*wireHit)->unsetMaskedFlag();
      }

      AxialHitQuadTreeProcessor qtProcessor1(13, 4, xySpans1, highPtPrecisionFunction);
      qtProcessor1.seed(m_axialWireHits);

      // actual filling of the hits into the quad tree structure
      qtProcessor1.fill(candidateReceiver, 30);

      AxialHitQuadTreeProcessor qtProcessor2(11, 1, xySpans2, lowPtPrecisionFunction);
      qtProcessor2.seed(m_axialWireHits);

      // actual filling of the hits into the quad tree structure
      qtProcessor2.fill(candidateReceiver, 30);
    });
    timeItResult.printSummary();

    // Check that the two tracks have been found.
    ASSERT_EQ(nMCTracks, candidates.size());

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].size(), 30);
    EXPECT_GE(candidates[1].size(), 30);
  }
}
