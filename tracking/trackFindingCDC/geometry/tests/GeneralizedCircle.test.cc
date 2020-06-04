/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_Getters)
{
  float absError = 10e-6;

  float n0 = 0.0;
  float n1 = 1.0;
  float n2 = 0.0;
  float n3 = 1.0;

  GeneralizedCircle circle(n0, n1, n2, n3);

  EXPECT_NEAR(n0, circle.n0(), absError);
  EXPECT_NEAR(n1, circle.n1(), absError);
  EXPECT_NEAR(n2, circle.n2(), absError);
  EXPECT_NEAR(n3, circle.n3(), absError);

  EXPECT_NEAR(2.0 * n3, circle.curvature(), absError);
  EXPECT_NEAR(1.0 / (2.0 * n3), circle.radius(), absError);

  EXPECT_NEAR(M_PI / 2.0, circle.tangentialPhi(), absError);

  EXPECT_NEAR(0, circle.impact(), absError);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_orientation)
{
  GeneralizedCircle circle(1.0, 0.0, 0.0, 1.0);
  EXPECT_EQ(ERotation::c_CounterClockwise, circle.orientation());

  GeneralizedCircle reversedCircle(1.0, 0.0, 0.0, -1.0);
  EXPECT_EQ(ERotation::c_Clockwise, reversedCircle.orientation());

  GeneralizedCircle line(1.0, 0.0, 0.0, 0.0);
  EXPECT_EQ(ERotation::c_CounterClockwise, line.orientation());

  GeneralizedCircle reversedLine(1.0, 0.0, 0.0, -0.0);
  EXPECT_EQ(ERotation::c_Clockwise, reversedLine.orientation());
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_conformalTranform)
{
  Vector2D center(1.0, 0.0);
  double radius = 1.0;
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  // Get two points on the circle to check for the orientation to be correct
  Vector2D firstPos = circle.atArcLength(1);
  Vector2D secondPos = circle.atArcLength(2);

  EXPECT_NEAR(1.0, circle.curvature(), 10e-7);
  EXPECT_NEAR(-M_PI / 2.0, circle.tangentialPhi(), 10e-7);
  EXPECT_NEAR(0.0, circle.impact(), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(firstPos), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(secondPos), 10e-7);

  circle.conformalTransform();
  firstPos.conformalTransform();
  secondPos.conformalTransform();

  EXPECT_NEAR(0.0, circle.curvature(), 10e-7);
  EXPECT_NEAR(M_PI / 2.0, circle.tangentialPhi(), 10e-7);
  EXPECT_NEAR(-1.0 / 2.0, circle.impact(), 10e-7);

  double firstConformalArcLength = circle.arcLengthTo(firstPos);
  double secondConformalArcLength = circle.arcLengthTo(secondPos);
  EXPECT_TRUE(firstConformalArcLength < secondConformalArcLength);
  EXPECT_LT(firstConformalArcLength, secondConformalArcLength);

  EXPECT_NEAR(0.0, circle.distance(firstPos), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(secondPos), 10e-7);

  // Another conformal transformation goes back to the original circle
  GeneralizedCircle conformalCopy = circle.conformalTransformed();
  EXPECT_NEAR(1.0, conformalCopy.curvature(), 10e-7);
  EXPECT_NEAR(-M_PI / 2.0, conformalCopy.tangentialPhi(), 10e-7);
  EXPECT_NEAR(0.0, conformalCopy.impact(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_closest)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  Vector2D up(1.0, 2.0);
  Vector2D far(4.0, 0.0);

  EXPECT_EQ(Vector2D(1.0, 1.0), circle.closest(up));
  EXPECT_EQ(Vector2D(2.0, 0.0), circle.closest(far));

  // This tests for point which is on the circle
  double smallAngle = M_PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));

  Vector2D closestOfNear = circle.closest(near);
  EXPECT_NEAR(near.x(), closestOfNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), closestOfNear.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_arcLengthFactor)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  double smallAngle = M_PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));

  double expectedArcLengthFactor = smallAngle / near.cylindricalR();
  EXPECT_NEAR(expectedArcLengthFactor, circle.arcLengthFactor(near.cylindricalR()), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_arcLengthBetween)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  Vector2D origin(0.0, 0.0);
  Vector2D up(1.0, 2.0);
  Vector2D down(1.0, -2.0);
  Vector2D far(4.0, 0.0);

  double smallAngle = M_PI / 100;
  Vector2D close(1.0 - cos(smallAngle), sin(smallAngle));

  EXPECT_NEAR(-M_PI / 2.0, circle.arcLengthBetween(origin, up), 10e-7);
  EXPECT_NEAR(M_PI / 2.0, circle.arcLengthBetween(origin, down), 10e-7);

  EXPECT_NEAR(M_PI / 2.0, circle.arcLengthBetween(up, origin), 10e-7);
  EXPECT_NEAR(-M_PI / 2.0, circle.arcLengthBetween(down, origin), 10e-7);

  // Sign of the length at the far end is unstable, which is why fabs is taken here
  EXPECT_NEAR(M_PI, fabs(circle.arcLengthBetween(origin, far)), 10e-7);

  EXPECT_NEAR(-smallAngle, circle.arcLengthBetween(origin, close), 10e-7);

  GeneralizedCircle line(0.0, -1.0, 0.0, 0.0);
  EXPECT_NEAR(-2, line.arcLengthBetween(origin, up), 10e-7);
  EXPECT_NEAR(2, line.arcLengthBetween(origin, down), 10e-7);
  EXPECT_NEAR(0, line.arcLengthBetween(origin, far), 10e-7);

  GeneralizedCircle reverseLine(0.0, 1.0, 0.0, -0.0);
  EXPECT_NEAR(2, reverseLine.arcLengthBetween(origin, up), 10e-7);
  EXPECT_NEAR(-2, reverseLine.arcLengthBetween(origin, down), 10e-7);
  EXPECT_NEAR(0, reverseLine.arcLengthBetween(origin, far), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_passiveMoveBy)
{
  Vector2D center(4.0, 2.0);
  double radius = 5.0;
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  circle.passiveMoveBy(Vector2D(3.0, 3.0));

  EXPECT_NEAR(5.0, circle.radius(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().x(), 10e-7);
  EXPECT_NEAR(-1.0, circle.center().y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_intersections)
{

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(Vector2D(1.0, 1.0), 1);
  GeneralizedCircle line = GeneralizedCircle(sqrt(2.0), -Vector2D(1.0, 1.0).unit());

  std::pair<Vector2D, Vector2D> intersections = circle.intersections(line);

  const Vector2D& intersection1 = intersections.first;
  const Vector2D& intersection2 = intersections.second;

  EXPECT_NEAR(1 - sqrt(2.0) / 2.0, intersection1.x(), 10e-7);
  EXPECT_NEAR(1 + sqrt(2.0) / 2.0, intersection1.y(), 10e-7);

  EXPECT_NEAR(1 + sqrt(2.0) / 2.0, intersection2.x(), 10e-7);
  EXPECT_NEAR(1 - sqrt(2.0) / 2.0, intersection2.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_atArcLength)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  double smallAngle = M_PI / 100;
  Vector2D near(2.0 - cos(smallAngle), sin(smallAngle));

  double nearArcLength =
    -smallAngle * radius; // Minus because of default counterclockwise orientation

  Vector2D atNear = circle.atArcLength(nearArcLength);

  EXPECT_NEAR(near.x(), atNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), atNear.y(), 10e-7);

  Vector2D down(2.0, -1.0);
  double downArcLength =
    +M_PI / 2.0 * radius; // Plus because of default counterclockwise orientation

  Vector2D atDown = circle.atArcLength(downArcLength);

  EXPECT_NEAR(down.x(), atDown.x(), 10e-7);
  EXPECT_NEAR(down.y(), atDown.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_arcLengthToCylindricalR)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);
  {
    double closestArcLength = circle.arcLengthToCylindricalR(1);
    EXPECT_NEAR(0, closestArcLength, 10e-7);

    double widestArcLength = circle.arcLengthToCylindricalR(3);
    EXPECT_NEAR(M_PI, widestArcLength, 10e-7);

    double halfArcLength = circle.arcLengthToCylindricalR(sqrt(5.0));
    EXPECT_NEAR(M_PI / 2, halfArcLength, 10e-7);

    double unreachableHighArcLength = circle.arcLengthToCylindricalR(4);
    EXPECT_TRUE(std::isnan(unreachableHighArcLength));

    double unreachableLowArcLength = circle.arcLengthToCylindricalR(0.5);
    EXPECT_TRUE(std::isnan(unreachableLowArcLength));
  }

  GeneralizedCircle reversedCircle = circle.reversed();
  {
    double closestArcLength = reversedCircle.arcLengthToCylindricalR(1);
    EXPECT_NEAR(0, closestArcLength, 10e-7);

    double widestArcLength = reversedCircle.arcLengthToCylindricalR(3);
    EXPECT_NEAR(M_PI, widestArcLength, 10e-7);

    double halfArcLength = reversedCircle.arcLengthToCylindricalR(sqrt(5.0));
    EXPECT_NEAR(M_PI / 2, halfArcLength, 10e-7);

    double unreachableHighArcLength = reversedCircle.arcLengthToCylindricalR(4);
    EXPECT_TRUE(std::isnan(unreachableHighArcLength));

    double unreachableLowArcLength = reversedCircle.arcLengthToCylindricalR(0.5);
    EXPECT_TRUE(std::isnan(unreachableLowArcLength));
  }
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_atCylindricalR)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  std::pair<Vector2D, Vector2D> solutions = circle.atCylindricalR(sqrt(5.0));

  EXPECT_NEAR(2, solutions.first.x(), 10e-7);
  EXPECT_NEAR(1, solutions.first.y(), 10e-7);

  EXPECT_NEAR(2, solutions.second.x(), 10e-7);
  EXPECT_NEAR(-1, solutions.second.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_isLine)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  EXPECT_FALSE(circle.isLine());

  float curvature = 0;
  float phi0 = M_PI / 2;
  float impact = -1;
  GeneralizedCircle line = GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact);

  EXPECT_TRUE(line.isLine());
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_isCircle)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  EXPECT_TRUE(circle.isCircle());

  float curvature = 0;
  float phi0 = M_PI / 2;
  float impact = -1;
  GeneralizedCircle line = GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact);

  EXPECT_FALSE(line.isCircle());
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_perigeeConversion)
{
  float curvature = -0.5;
  float phi0 = M_PI / 2;
  float impact = -1;
  GeneralizedCircle circle = GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact);

  EXPECT_NEAR(impact, circle.impact(), 10e-7);
  EXPECT_NEAR(phi0, circle.tangentialPhi(), 10e-7);
  EXPECT_NEAR(curvature, circle.curvature(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_GeneralizedCircle_distance)
{
  float absError = 10e-6;

  float radius = 1.5;
  Vector2D center = Vector2D(0.5, 0.0);

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  Vector2D testPoint(3, 0);

  EXPECT_NEAR(1, circle.distance(testPoint), absError);
  EXPECT_NEAR(1, circle.absDistance(testPoint), absError);
  // Approximated distance is already quite far of since the circle radius is of the order of the
  // distance
  EXPECT_NEAR(1.33333, circle.fastDistance(testPoint), absError);

  // Now clockwise orientation. All points outside the circle have negative distance
  circle.reverse();

  EXPECT_NEAR(-1, circle.distance(testPoint), absError);
  EXPECT_NEAR(1, circle.absDistance(testPoint), absError);
  EXPECT_NEAR(-1.33333, circle.fastDistance(testPoint), absError);
}
