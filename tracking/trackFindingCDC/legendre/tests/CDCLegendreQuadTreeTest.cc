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
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

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
  QuadTreeLegendreTemp qt(0, std::pow(2, 13), -1.5, 1.5, 0, nullptr);

  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();
  std::set<LegendreQuadTreeItem*> items_set;

  for (TrackHit* trackHit : hits_set) {
    items_set.insert(new LegendreQuadTreeItem(trackHit));
  }

  QuadTreeLegendreTemp::NodeList candidateNodes;

  QuadTreeProcessorTemp qtProcessor(13);
  qt.provideItemsSet(qtProcessor, items_set);

  QuadTreeProcessorTemp::CandidateProcessorLambda lmdProcessor = [&candidateNodes](
  QuadTreeTemplate<int, float, QuadTreeItem<TrackHit>>* qt) {
    std::for_each(qt->getItemsVector().begin(), qt->getItemsVector().end(), [](LegendreQuadTreeItem * th) {th->setUsedFlag();});
    candidateNodes.push_back(qt);
  };

  auto now = std::chrono::high_resolution_clock::now();

  // actual filling of the hits into the quad tree structure
  qtProcessor.fillGivenTree(&qt, lmdProcessor, 55);
  auto later = std::chrono::high_resolution_clock::now();

  ASSERT_EQ(numberOfPossibleTrackCandidate, candidateNodes.size());

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
  B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidateNodes.size() << " candidates");

  // Check for the parameters of the track candidates
  // The actual hit numbers are more than 50, but this is somewhat a lower bound
  EXPECT_GE(candidateNodes[0]->getNItems(), 50);
  EXPECT_GE(candidateNodes[1]->getNItems(), 50);
}

