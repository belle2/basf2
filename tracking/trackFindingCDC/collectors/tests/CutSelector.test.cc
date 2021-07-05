/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/selectors/CutSelector.h>

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Test for the CutSelector
  TEST(TrackFindingCDCTest, cut_selector)
  {
    CutSelector<int, double> selector;

    int a = 1;
    double b = 2, c = 3, d = 4, e = 5;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &b),
      WeightedRelation<int, const double>(&a, 1.0, &c),
      WeightedRelation<int, const double>(&a, 0.8, &d),
      WeightedRelation<int, const double>(&a, NAN, &e)
    };

    selector.setCutValue(1.0);

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 2);
    ASSERT_EQ(relations[0].getWeight(), 1.1);
    ASSERT_EQ(relations[1].getWeight(), 1.0);
  }
}
