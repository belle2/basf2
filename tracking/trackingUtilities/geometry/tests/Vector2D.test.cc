/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackingUtilities/geometry/Vector2D.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackingUtilities;

TEST(TrackingUtilitiesTest, geometry_Vector2D_flippedOver)
{
  Vector2D diag(0.5, 0.5);
  Vector2D test(1, 0);
  Vector2D flippedOver = test.flippedOver(diag);

  EXPECT_EQ(0, flippedOver.x());
  EXPECT_EQ(1, flippedOver.y());
}

TEST(TrackingUtilitiesTest, geometry_Vector2D_flippedAlong)
{
  Vector2D diag(0.5, 0.5);
  Vector2D test(1, 0);
  Vector2D flippedAlong = test.flippedAlong(diag);

  EXPECT_EQ(0, flippedAlong.x());
  EXPECT_EQ(-1, flippedAlong.y());
}
