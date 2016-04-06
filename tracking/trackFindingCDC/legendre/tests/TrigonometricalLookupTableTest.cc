/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>

#include <gtest/gtest.h>
#include <boost/math/constants/constants.hpp>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrigonometricalLookupTableTest, compute_sin_cos)
{
  TrigonometricalLookupTable<>& inst = TrigonometricalLookupTable<>::Instance();

  auto binCount = inst.getNBinsTheta();
  double binWidth = 2.0f * boost::math::constants::pi<double>() / binCount;

  // test the range of bins
  for (unsigned long i = 0; i <= binCount; i++) {
    float cs_0 = inst.cosTheta(i);
    EXPECT_NEAR(cos(i * binWidth -  boost::math::constants::pi<double>() + binWidth / 2.), cs_0, 0.000001);
    float ss_0 = inst.sinTheta(i);
    EXPECT_NEAR(sin(i * binWidth -  boost::math::constants::pi<double>() + binWidth / 2.), ss_0, 0.000001);
  }

  // test above the bin range, must work too
  for (unsigned long i = binCount + 1; i <= binCount + 50; i++) {
    float cs_0 = inst.cosTheta(i);
    EXPECT_NEAR(cos(i * binWidth -  boost::math::constants::pi<double>() + binWidth / 2.), cs_0, 0.000001);
    float ss_0 = inst.sinTheta(i);
    EXPECT_NEAR(sin(i * binWidth -  boost::math::constants::pi<double>() + binWidth / 2.), ss_0, 0.000001);
  }
}

