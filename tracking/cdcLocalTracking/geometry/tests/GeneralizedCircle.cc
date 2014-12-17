/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>
#include <tracking/cdcLocalTracking/geometry/Vector2D.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_Getters)
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

  EXPECT_NEAR(PI / 2.0, circle.tangentialPhi(), absError);

  EXPECT_NEAR(0, circle.impact(), absError);

}



TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_orientation)
{
  GeneralizedCircle circle(1.0, 0.0, 0.0, 1.0);
  EXPECT_EQ(CCW, circle.orientation());

  GeneralizedCircle reversedCircle(1.0, 0.0, 0.0, -1.0);
  EXPECT_EQ(CW, reversedCircle.orientation());

  GeneralizedCircle line(1.0, 0.0, 0.0, 0.0);
  EXPECT_EQ(CCW, line.orientation());

  GeneralizedCircle reversedLine(1.0, 0.0, 0.0, -0.0);
  EXPECT_EQ(CW, reversedLine.orientation());
}



TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_closest)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  Vector2D up(1.0, 2.0);
  Vector2D far(4.0, 0.0);

  EXPECT_EQ(Vector2D(1.0, 1.0), circle.closest(up));
  EXPECT_EQ(Vector2D(2.0, 0.0), circle.closest(far));


  // This tests for point which is on the circle
  FloatType smallAngle = PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));


  Vector2D closestOfNear = circle.closest(near);
  EXPECT_NEAR(near.x(), closestOfNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), closestOfNear.y(), 10e-7);

}



TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_arcLengthFactor)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  FloatType smallAngle = PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));

  FloatType expectedArcLengthFactor = smallAngle / near.polarR();
  EXPECT_NEAR(expectedArcLengthFactor, circle.arcLengthFactor(near.polarR()), 10e-7);

}





TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_arcLengthBetween)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  Vector2D origin(0.0, 0.0);
  Vector2D up(1.0, 2.0);
  Vector2D down(1.0, -2.0);
  Vector2D far(4.0, 0.0);

  FloatType smallAngle = PI / 100;
  Vector2D close(1.0 - cos(smallAngle), sin(smallAngle));

  EXPECT_NEAR(-PI / 2.0, circle.arcLengthBetween(origin, up), 10e-7);
  EXPECT_NEAR(PI / 2.0, circle.arcLengthBetween(origin, down), 10e-7);

  EXPECT_NEAR(PI / 2.0, circle.arcLengthBetween(up, origin), 10e-7);
  EXPECT_NEAR(-PI / 2.0, circle.arcLengthBetween(down, origin), 10e-7);

  // Sign of the length at the far end is unstable, which is why fabs is taken here
  EXPECT_NEAR(PI, fabs(circle.arcLengthBetween(origin, far)), 10e-7);

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



TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_passiveMoveBy)
{
  Vector2D center(4.0, 2.0);
  FloatType radius = 5.0;
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  circle.passiveMoveBy(Vector2D(3.0, 3.0));

  EXPECT_NEAR(5.0, circle.radius(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().x(), 10e-7);
  EXPECT_NEAR(-1.0, circle.center().y(), 10e-7);

}



TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_intersections)
{

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(Vector2D(1.0, 1.0), 1);
  GeneralizedCircle line = GeneralizedCircle(sqrt(2.0), -Vector2D(1.0, 1.0).unit());


  pair<Vector2D, Vector2D> intersections = circle.intersections(line);

  const Vector2D& intersection1 = intersections.first;
  const Vector2D& intersection2 = intersections.second;

  EXPECT_NEAR(1 - sqrt(2.0) / 2.0, intersection1.x(), 10e-7);
  EXPECT_NEAR(1 + sqrt(2.0) / 2.0, intersection1.y(), 10e-7);

  EXPECT_NEAR(1 + sqrt(2.0) / 2.0, intersection2.x(), 10e-7);
  EXPECT_NEAR(1 - sqrt(2.0) / 2.0, intersection2.y(), 10e-7);

}


TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_atArcLength)
{
  FloatType radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  FloatType smallAngle = PI / 100;
  Vector2D near(2.0 - cos(smallAngle), sin(smallAngle));

  FloatType nearArcLength = -smallAngle * radius; //Minus because of default counterclockwise orientation

  Vector2D atNear = circle.atArcLength(nearArcLength);

  EXPECT_NEAR(near.x(), atNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), atNear.y(), 10e-7);


  Vector2D down(2.0, -1.0);
  FloatType downArcLength = +PI / 2.0 * radius; //Plus because of default counterclockwise orientation

  Vector2D atDown = circle.atArcLength(downArcLength);

  EXPECT_NEAR(down.x(), atDown.x(), 10e-7);
  EXPECT_NEAR(down.y(), atDown.y(), 10e-7);

}


TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_isLine)
{
  FloatType radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  EXPECT_FALSE(circle.isLine());

  float curvature = 0;
  float phi0 = PI / 2;
  float impact = -1;
  GeneralizedCircle line = GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact);

  EXPECT_TRUE(line.isLine());
}


TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_isCircle)
{
  FloatType radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  EXPECT_TRUE(circle.isCircle());

  float curvature = 0;
  float phi0 = PI / 2;
  float impact = -1;
  GeneralizedCircle line = GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact);

  EXPECT_FALSE(line.isCircle());
}


TEST(CDCLocalTrackingTest, geometry_GeneralizedCircle_distance)
{
  float absError = 10e-6;

  float radius = 1.5;
  Vector2D center = Vector2D(0.5, 0.0);

  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  Vector2D testPoint(3, 0);

  EXPECT_NEAR(1,  circle.distance(testPoint), absError);
  EXPECT_NEAR(1,  circle.absDistance(testPoint), absError);
  // Approximated distance is already quite far of since the circle radius is of the order of the distance
  EXPECT_NEAR(1.33333,  circle.fastDistance(testPoint), absError);

  // Now clockwise orientation. All points outside the circle have negative distance
  circle.reverse();

  EXPECT_NEAR(-1,  circle.distance(testPoint), absError);
  EXPECT_NEAR(1,  circle.absDistance(testPoint), absError);
  EXPECT_NEAR(-1.33333,  circle.fastDistance(testPoint), absError);

}
