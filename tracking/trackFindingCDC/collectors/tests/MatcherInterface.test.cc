/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Test derived class from a MatcherInterface
  class SimpleMatcher : public MatcherInterface<int, double> {
    Weight match(int& a, const double& b) override
    {
      return a + b;
    }
  };
  /// Test for the MatcherInterface
  TEST(TrackFindingCDCTest, matcher_interface)
  {
    SimpleMatcher matcher;

    std::vector<WeightedRelation<int, const double>> relations;

    std::vector<int> collectorItems = {1, 2};
    std::vector<double> collectionItems = {2.1, 2.2, 2.3};

    matcher.apply(collectorItems, collectionItems, relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 6);
    ASSERT_EQ(relations[5].getWeight(), 3.1);
    ASSERT_EQ(relations[4].getWeight(), 3.2);
    ASSERT_EQ(relations[3].getWeight(), 3.3);
    ASSERT_EQ(relations[2].getWeight(), 4.1);
    ASSERT_EQ(relations[1].getWeight(), 4.2);
    ASSERT_EQ(relations[0].getWeight(), 4.3);
  }
}
