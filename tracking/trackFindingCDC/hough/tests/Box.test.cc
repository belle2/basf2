/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
