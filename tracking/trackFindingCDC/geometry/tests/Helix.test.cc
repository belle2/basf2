/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/Helix.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_Helix_closest)
{
  // Tests the calculation of points of closest approach under various orientation of the helix.
  // Here this is done by moving the coordinate system and thereby changing the perigee point to
  // different representations.
  // The point of closest approach should always lie at the position of the original origin / its
  // transformed counterpart.
  std::vector<Vector3D> bys = {Vector3D(0.0, 0.0, 0.0),
                               Vector3D(0.0, 0.0, 5.0),
                               Vector3D(0.0, 3.0, 0.0),
                               Vector3D(3.0, 0.0, 0.0),
                               Vector3D(-1.0, -2.0, 3.0)
                              };

  for (const Vector3D& by : bys) {
    double curvature = +1.0 / 2.0;
    double phi0 = -M_PI / 2.0;
    double impact = +1.0;
    double tanLambda = 1.0 / 2.0;
    double z0 = 0.0;

    Helix helix(curvature, phi0, impact, tanLambda, z0);
    Vector3D point(0.0, 0.0, 0.0);

    Vector3D expectedClosest = helix.perigee();

    helix.passiveMoveBy(by);
    expectedClosest.passiveMoveBy(by);
    point.passiveMoveBy(by);
    {
      Vector3D realClosest = helix.closest(point, true);
      EXPECT_NEAR(expectedClosest.x(), realClosest.x(), 10e-7) << "Test for displacement by " << by;
      EXPECT_NEAR(expectedClosest.y(), realClosest.y(), 10e-7) << "Test for displacement by " << by;
      EXPECT_NEAR(expectedClosest.z(), realClosest.z(), 10e-7) << "Test for displacement by " << by;
    }

    helix.shiftPeriod(2);
    {
      Vector3D realClosest = helix.closest(point, false);
      EXPECT_NEAR(expectedClosest.x(), realClosest.x(), 10e-7) << "Test for displacement by " << by;
      EXPECT_NEAR(expectedClosest.y(), realClosest.y(), 10e-7) << "Test for displacement by " << by;
      EXPECT_NEAR(expectedClosest.z(), realClosest.z(), 10e-7) << "Test for displacement by " << by;
    }
  }
}

TEST(TrackFindingCDCTest, geometry_Helix_arcLength2DToCylndricalR)
{
  double curvature = -1.0;
  double phi0 = +M_PI / 2.0;
  double impact = -1.0;
  double tanLambda = 1.0 / 2.0;
  double z0 = 3.0;

  Helix helix(curvature, phi0, impact, tanLambda, z0);
  EXPECT_EQ(-1, helix.radiusXY());

  double closestArcLength2D = helix.arcLength2DToCylindricalR(1);
  EXPECT_NEAR(0, closestArcLength2D, 10e-7);

  double widestArcLength2D = helix.arcLength2DToCylindricalR(3);
  EXPECT_NEAR(M_PI, widestArcLength2D, 10e-7);

  double halfArcLength2D = helix.arcLength2DToCylindricalR(sqrt(5.0));
  EXPECT_NEAR(M_PI / 2, halfArcLength2D, 10e-7);

  double unreachableHighArcLength2D = helix.arcLength2DToCylindricalR(4);
  EXPECT_TRUE(std::isnan(unreachableHighArcLength2D));

  double unreachableLowArcLength2D = helix.arcLength2DToCylindricalR(0.5);
  EXPECT_TRUE(std::isnan(unreachableLowArcLength2D));
}

TEST(TrackFindingCDCTest, geometry_Helix_arcLength2DToXY)
{
  double curvature = -1.0;
  double phi0 = +M_PI / 2.0;
  double impact = -1.0;
  double tanLambda = 1.0 / 2.0;
  double z0 = 3.0;

  Helix helix(curvature, phi0, impact, tanLambda, z0);

  Vector2D origin(0.0, 0.0);
  double closestArcLength2D = helix.arcLength2DToXY(origin);
  EXPECT_NEAR(0, closestArcLength2D, 10e-7);

  double widestArcLength2D = helix.arcLength2DToXY(Vector2D(5.0, 0.0));
  EXPECT_NEAR(M_PI, widestArcLength2D, 10e-7);

  double halfArcLength2D = helix.arcLength2DToXY(Vector2D(2.0, 5.0));
  EXPECT_NEAR(M_PI / 2, halfArcLength2D, 10e-7);

  double otherHalfArcLength2D = helix.arcLength2DToXY(Vector2D(2.0, -5.0));
  EXPECT_NEAR(-M_PI / 2, otherHalfArcLength2D, 10e-7);
}
