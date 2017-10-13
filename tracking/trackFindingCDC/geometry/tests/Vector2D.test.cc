/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_Vector2D_isBetween)
{
  {
    Vector2D lower = Vector2D::Phi(0);
    Vector2D upper = Vector2D::Phi(M_PI);

    Vector2D test = Vector2D::Phi(M_PI / 2);
    Vector2D test2 = Vector2D::Phi(3 * M_PI / 2);

    EXPECT_TRUE(test.isBetween(lower, upper));
    EXPECT_FALSE(test.isBetween(upper, lower));
    EXPECT_FALSE(test2.isBetween(lower, upper));
    EXPECT_TRUE(test2.isBetween(upper, lower));
  }

  {
    Vector2D lower = Vector2D::Phi(0);
    Vector2D upper = Vector2D::Phi(M_PI / 2);

    Vector2D test1 = Vector2D::Phi(M_PI / 4);
    Vector2D test2 = Vector2D::Phi(3 * M_PI / 4);
    Vector2D test3 = Vector2D::Phi(5 * M_PI / 4);
    Vector2D test4 = Vector2D::Phi(7 * M_PI / 4);

    EXPECT_TRUE(test1.isBetween(lower, upper));
    EXPECT_FALSE(test1.isBetween(upper, lower));

    EXPECT_FALSE(test2.isBetween(lower, upper));
    EXPECT_TRUE(test2.isBetween(upper, lower));

    EXPECT_FALSE(test3.isBetween(lower, upper));
    EXPECT_TRUE(test3.isBetween(upper, lower));

    EXPECT_FALSE(test4.isBetween(lower, upper));
    EXPECT_TRUE(test4.isBetween(upper, lower));
  }

  {
    Vector2D lower = Vector2D::Phi(-M_PI / 4);
    Vector2D upper = Vector2D::Phi(M_PI);

    Vector2D test = Vector2D::Phi(0);
    Vector2D test2 = Vector2D::Phi(M_PI / 2);

    EXPECT_TRUE(test.isBetween(lower, upper));
    EXPECT_FALSE(test.isBetween(upper, lower));
    EXPECT_TRUE(test2.isBetween(lower, upper));
    EXPECT_FALSE(test2.isBetween(upper, lower));
  }
}

TEST(TrackFindingCDCTest, geometry_Vector2D_flippedOver)
{
  Vector2D diag(0.5, 0.5);
  Vector2D test(1, 0);
  Vector2D flippedOver = test.flippedOver(diag);

  EXPECT_EQ(0, flippedOver.x());
  EXPECT_EQ(1, flippedOver.y());
}

TEST(TrackFindingCDCTest, geometry_Vector2D_flippedAlong)
{
  Vector2D diag(0.5, 0.5);
  Vector2D test(1, 0);
  Vector2D flippedAlong = test.flippedAlong(diag);

  EXPECT_EQ(0, flippedAlong.x());
  EXPECT_EQ(-1, flippedAlong.y());
}
