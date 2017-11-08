/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Test for the BestMatchSelector
  TEST(TrackFindingCDCTest, best_match_selector)
  {
    BestMatchSelector<int, double> selector;

    int a = 1, b = 2;
    double c = 3, d = 4, e = 5;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.0, &d),
      WeightedRelation<int, const double>(&a, 1.1, &c),
      WeightedRelation<int, const double>(&b, 0.8, &c),
      WeightedRelation<int, const double>(&b, 0.6, &e)
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 2);
    ASSERT_EQ(relations[0].getWeight(), 1.1);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &c);
    ASSERT_EQ(relations[1].getWeight(), 0.6);
    ASSERT_EQ(relations[1].getFrom(), &b);
    ASSERT_EQ(relations[1].getTo(), &e);
  }
}
