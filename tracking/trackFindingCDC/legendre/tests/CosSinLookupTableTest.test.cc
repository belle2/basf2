/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST(CosSinLookupTableTest, compute_sin_cos)
{
  auto inst = AxialHitQuadTreeProcessor::getCosSinLookupTable();

  auto binCount = inst.getNBins();
  float binWidth = 2 * M_PI / binCount;

  // test the range of bins
  for (long i = 0; i <= binCount; i++) {
    float cs_0 = inst.at(i).x();
    EXPECT_NEAR(cos(i * binWidth - M_PI), cs_0, 0.000001);
    float ss_0 = inst.at(i).y();
    EXPECT_NEAR(sin(i * binWidth - M_PI), ss_0, 0.000001);
  }
}
