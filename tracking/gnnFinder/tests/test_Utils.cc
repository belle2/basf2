/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/gnnFinder/Utils.h>

#include <cmath>
#include <numbers>

#include <gtest/gtest.h>

using namespace Belle2;

namespace Belle2::GNNFinder::Utils {

  TEST(HitOrdererTest, OrderSingleHit)
  {
    HitOrderer orderer;

    std::vector<KDTHit> hits = {
      KDTHit{1.0, 1.0, 5},
    };

    auto result = orderer.orderHits(2.0, 1.0, hits);

    ASSERT_EQ(result.size(), hits.size());
    EXPECT_EQ(result[0], 5);
  }

  TEST(HitOrdererTest, OrderMultipleHits)
  {
    HitOrderer orderer;

    std::vector<KDTHit> hits = {
      KDTHit{-2.0, -2.0, 0},
      KDTHit{2.0, 1.0, 1},
      KDTHit{1.0, 0.5, 2},
      KDTHit{0.0, 0.0, 3}
    };

    auto result = orderer.orderHits(0.1, 0.0, hits);

    ASSERT_EQ(result.size(), hits.size());
    EXPECT_EQ(result[0], 3);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 1);
    EXPECT_EQ(result[3], 0);
  }

  TEST(HitOrdererTest, OrderHelicalPath)
  {
    HitOrderer orderer;

    // Hits are placed along a helix projected onto the xy-plane sampled at regular angular
    // steps. The expected order follows the arc from the innermost starting point outward.
    // Angular step is large enough that each hit is strictly closer to its arc-neighbour
    // than to any other hit, making the result deterministic.
    const double R    = 5.0;
    const int    N    = 14;
    const double step = 2.0 * std::numbers::pi / N;
    const double startX = R * std::cos(-0.1 * step);
    const double startY = R * std::sin(-0.1 * step);

    std::vector<KDTHit> hits;
    hits.reserve(N);
    for (int i = 0; i < N; ++i)
      hits.push_back({R * std::cos(i * step), R * std::sin(i * step), i});

    auto result = orderer.orderHits(startX, startY, hits);

    // The hit ordering must recover the arc order 0, 1, 2, ..., N-1 exactly,
    // because each consecutive arc-neighbour is the unique nearest unused hit
    ASSERT_EQ(result.size(), hits.size());
    for (int i = 0; i < N; ++i)
      EXPECT_EQ(result[i], i);
  }

  TEST(HitOrdererTest, OrderTrickyPath)
  {
    HitOrderer orderer;

    std::vector<KDTHit> hits = {
      {0.0, 0.0, 0},
      {1.0, 1.0, 1},
      {2.0, 0.0, 2},
      {3.0, 1.0, 3},
      {4.0, 0.0, 4},
      {2.0, 3.5, 5},
      {2.5, 4.0, 6},
      {3.0, 3.2, 7},
      {2.0, -3.0, 8},
      {2.5, -3.5, 9},
      {3.0, -2.8, 10},
      {4.5, 2.0, 11},
      {6.0, 0.0, 12},
      {7.0, 1.0, 13}
    };
    // Here we hard-code the expected ordering.
    const std::vector<int> expected = {6, 5, 7, 11, 3, 4, 12, 13, 2, 1, 0, 8, 9, 10};

    auto result = orderer.orderHits(2.4, 3.8, hits);

    ASSERT_EQ(result.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
      EXPECT_EQ(result[i], expected[i]);
  }

}
