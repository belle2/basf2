/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>

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

TEST(TrackFindingCDCTest, legendre_QuadTreeTest)
{
  QuadTree qt(-1.0, 1.0, 0, std::pow(2, 13), 0, nullptr);

  std::set<TrackHit*> hits_set;
  std::vector < std::shared_ptr< TrackHit > > hits;

  QuadTree::NodeList candidateNodes;

  // create hits for three sraight-line tracks
  // going outwards from the center
  for (int i = 0; i < 100; i++) {
    TrackHit* hit1 = new TrackHit(i, i, 1.0f, 0.2f,
                                  500, i % 5, i % 6,
                                  TVector3(i * 1.0f, i * 2.0f, 0.0f)
                                 );
    hits_set.insert(hit1);

    TrackHit* hit2 = new TrackHit(i, i, 1.0f, 0.2f,
                                  500, i % 5, i % 6,
                                  TVector3(i * 0.0f, i * -2.0f, 0.0f)
                                 );
    hits_set.insert(hit2);

    TrackHit* hit3 = new TrackHit(i, i, 1.0f, 0.2f,
                                  500, i % 5, i % 6,
                                  TVector3(i * 1.0f, i * -1.0f, 0.0f)
                                 );
    hits_set.insert(hit3);

  }

  qt.provideHitSet(hits_set);
  qt.setLastLevel(13);

  qt.setRThreshold(1.0f);
  qt.setHitsThreshold(50);

  QuadTree::CandidateProcessorLambda lmdProcessor = [&candidateNodes](QuadTree * qt) {
    std::for_each(qt->getHits().begin(), qt->getHits().end(), [](TrackHit * th) {th->setHitUsage(TrackHit::used_in_track);});
    candidateNodes.push_back(qt);
  };

  auto now = std::chrono::high_resolution_clock::now();

  // actual filling of the hits into the quad tree structure
  qt.startFillingTree(lmdProcessor);
  auto later = std::chrono::high_resolution_clock::now();

  EXPECT_EQ(3, candidateNodes.size());

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
  B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidateNodes.size() << " candidates");

  // release TrackHits
  std::for_each(hits_set.begin(), hits_set.end(), [](TrackHit * ht) { delete ht;});
}

