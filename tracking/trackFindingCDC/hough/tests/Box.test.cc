/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/boxes/Box.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, Box_intersects)
{
  Box<float, float> box({{0, 1}}, {{0, 1}});
  EXPECT_TRUE(box.intersects(box));

  Box<float, float> box2({{0.5, 1.5}}, {{0.5, 1.5}});
  EXPECT_TRUE(box.intersects(box2));
  EXPECT_TRUE(box2.intersects(box));

  Box<float, float> box3({{1.5, 2.5}}, {{0.5, 1.5}});
  EXPECT_FALSE(box.intersects(box3));
  EXPECT_FALSE(box3.intersects(box));
}
