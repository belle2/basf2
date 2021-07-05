/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/baseelements/LinearDivision.h>
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <framework/logging/Logger.h>

#include <gtest/gtest.h>
#include <cmath>


using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, SumInfinities)
{
  EXPECT_EQ(INFINITY, INFINITY + INFINITY);

  std::array<Weight, 4> infinities{{INFINITY, INFINITY, INFINITY, INFINITY}};
  Weight weightSum = std::accumulate(std::begin(infinities), std::end(infinities), Weight(0));
  EXPECT_EQ(INFINITY, weightSum);
}


TEST(TrackFindingCDCTest, LinearDivision_createBoxes)
{
  Box<float, float> box({{0, 3}}, {{0, 2}});

  size_t nSubNodes =  LinearDivision<Box<float, float>, 3, 3>::s_nSubBoxes;
  EXPECT_EQ(9, nSubNodes);

  //LinearDivision<Box<float, float>, 3, 2> subBoxFactory{Box<float, float>::Delta{0.0, 0.0}};
  LinearDivision<Box<float, float>, 3, 2> subBoxFactory{};
  std::array<Box<float, float>, 6 > subBoxes = subBoxFactory(box);

  size_t i = 0;
  for (Box<float, float>& subBox : subBoxes) {
    B2INFO("Lower bounds: " << subBox.getLowerBound<0>() << ", " << subBox.getLowerBound<1>());
    B2INFO("Upper bounds: " << subBox.getUpperBound<0>() << ", " << subBox.getUpperBound<1>());

    EXPECT_EQ(i % 3 , subBox.getLowerBound<0>());
    EXPECT_EQ(i / 3, subBox.getLowerBound<1>());

    EXPECT_EQ(i % 3 + 1, subBox.getUpperBound<0>());
    EXPECT_EQ(i / 3 + 1, subBox.getUpperBound<1>());
    ++i;
  }

  /// Expect 9 subnode;
  EXPECT_EQ(6, i);
}
