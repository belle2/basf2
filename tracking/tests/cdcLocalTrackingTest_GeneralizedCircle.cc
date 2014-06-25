/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include "cdcLocalTrackingTest.h"

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;



TEST_F(CDCLocalTrackingTest, GeneralizedCircle_orientation)
{
  GeneralizedCircle circle(0.0, 0.0, 0.0, 1.0);
  EXPECT_EQ(CCW, circle.orientation());

  GeneralizedCircle reversedCircle(1.0, 0.0, 0.0, -1.0);
  EXPECT_EQ(CW, reversedCircle.orientation());

  GeneralizedCircle line(0.0, 0.0, 0.0, 0.0);
  EXPECT_EQ(CCW, line.orientation());

  GeneralizedCircle reversedLine(0.0, 0.0, 0.0, -0.0);
  EXPECT_EQ(CW, reversedLine.orientation());
}



TEST_F(CDCLocalTrackingTest, GeneralizedCircle_closest)
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



TEST_F(CDCLocalTrackingTest, GeneralizedCircle_arcLengthFactor)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  FloatType smallAngle = PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));

  FloatType expectedArcLengthFactor = smallAngle / near.polarR();
  EXPECT_NEAR(expectedArcLengthFactor, circle.arcLengthFactor(near.polarR()), 10e-7);

}





TEST_F(CDCLocalTrackingTest, GeneralizedCircle_lengthOnCurve)
{
  GeneralizedCircle circle(0.0, -1.0, 0.0, 1.0 / 2.0);
  Vector2D origin(0.0, 0.0);
  Vector2D up(1.0, 2.0);
  Vector2D down(1.0, -2.0);
  Vector2D far(4.0, 0.0);

  FloatType smallAngle = PI / 100;
  Vector2D close(1.0 - cos(smallAngle), sin(smallAngle));

  EXPECT_NEAR(-PI / 2.0, circle.lengthOnCurve(origin, up), 10e-7);
  EXPECT_NEAR(PI / 2.0, circle.lengthOnCurve(origin, down), 10e-7);

  EXPECT_NEAR(PI / 2.0, circle.lengthOnCurve(up, origin), 10e-7);
  EXPECT_NEAR(-PI / 2.0, circle.lengthOnCurve(down, origin), 10e-7);

  // Sign of the length at the far end is unstable, which is why fabs is taken here
  EXPECT_NEAR(PI, fabs(circle.lengthOnCurve(origin, far)), 10e-7);

  EXPECT_NEAR(-smallAngle, circle.lengthOnCurve(origin, close), 10e-7);


  GeneralizedCircle line(0.0, -1.0, 0.0, 0.0);
  EXPECT_NEAR(-2, line.lengthOnCurve(origin, up), 10e-7);
  EXPECT_NEAR(2, line.lengthOnCurve(origin, down), 10e-7);
  EXPECT_NEAR(0, line.lengthOnCurve(origin, far), 10e-7);

  GeneralizedCircle reverseLine(0.0, 1.0, 0.0, -0.0);
  EXPECT_NEAR(2, reverseLine.lengthOnCurve(origin, up), 10e-7);
  EXPECT_NEAR(-2, reverseLine.lengthOnCurve(origin, down), 10e-7);
  EXPECT_NEAR(0, reverseLine.lengthOnCurve(origin, far), 10e-7);

}



TEST_F(CDCLocalTrackingTest, GeneralizedCircle_passiveMoveBy)
{
  Vector2D center(4.0, 2.0);
  FloatType radius = 5.0;
  GeneralizedCircle circle = GeneralizedCircle::fromCenterAndRadius(center, radius);

  circle.passiveMoveBy(Vector2D(3.0, 3.0));

  EXPECT_NEAR(5.0, circle.radius(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().x(), 10e-7);
  EXPECT_NEAR(-1.0, circle.center().y(), 10e-7);

}



TEST_F(CDCLocalTrackingTest, GeneralizedCircle_intersections)
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






/**

  B2INFO("##### Test generalized circle");
  GeneralizedCircle circle(Vector2D(0.5, 0.0), 1.5);
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0.5,2.5 to  3,0 " << circle.openingAngle(Vector2D(0.5, 2.5) , Vector2D(3, 0)));
  B2INFO("arc length from 0.5,2.5 to  3,0 " << circle.lengthOnCurve(Vector2D(0.5, 2.5) , Vector2D(3, 0)));


  circle.reverse();
  B2INFO("Reversed");
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0.5,2,5 to  3,0 " << circle.openingAngle(Vector2D(0.5, 2.5) , Vector2D(3, 0)));
  B2INFO("arc length from 0.5,2.5 to  3,0 " << circle.lengthOnCurve(Vector2D(0.5, 2.5) , Vector2D(3, 0)));


  circle.setN(1, 1, -1, 0);
  circle.normalize();
  B2INFO("Line");
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0,0 to  3,0 " << circle.openingAngle(Vector2D(0, 0) , Vector2D(3, 0)));
  B2INFO("arc length from 0,0 to  3,0 " << circle.lengthOnCurve(Vector2D(0, 0) , Vector2D(3, 0)));

  B2INFO("Reversed line");
  circle.reverse();
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0,0 to  3,0 " << circle.openingAngle(Vector2D(0, 0) , Vector2D(3, 0)));
  B2INFO("arc length from 0,0 to  3,0 " << circle.lengthOnCurve(Vector2D(0, 0) , Vector2D(3, 0)));
*/
