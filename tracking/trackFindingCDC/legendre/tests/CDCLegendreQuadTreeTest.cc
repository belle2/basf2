/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>

#include <set>
#include <cmath>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(CDCLegendreTestFixture, legendre_QuadTreeTest)
{
  QuadTreeLegendre qt(0, std::pow(2, 13), -1.5, 1.5, 0, nullptr);

  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();

  QuadTreeLegendre::NodeList candidateNodes;

  qt.provideItemsSet<QuadTreeProcessor>(hits_set);
  qt.setLastLevel(13);
  qt.setNItemsThreshold(50);

  QuadTreeLegendre::CandidateProcessorLambda lmdProcessor = [&candidateNodes](QuadTreeLegendre * qt) {
    std::for_each(qt->getItemsVector().begin(), qt->getItemsVector().end(), [](TrackHit * th) {th->setHitUsage(TrackHit::used_in_track);});
    candidateNodes.push_back(qt);
  };

  auto now = std::chrono::high_resolution_clock::now();

  // actual filling of the hits into the quad tree structure
  qt.startFillingTree<QuadTreeProcessor>(lmdProcessor);
  auto later = std::chrono::high_resolution_clock::now();

  EXPECT_EQ(numberOfPossibleTrackCandidate, candidateNodes.size());

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
  B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidateNodes.size() << " candidates");

  // Check for the parameters of the track candidates
  // The actual hit numbers are more than 50, but this is somewhat a lower bound
  EXPECT_GE(candidateNodes[0]->getNItems(), 50);
  EXPECT_GE(candidateNodes[1]->getNItems(), 50);
}

