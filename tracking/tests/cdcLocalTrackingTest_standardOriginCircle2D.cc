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
#include <tracking/cdcLocalTracking/geometry/StandardOriginCircle2D.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;



TEST_F(CDCLocalTrackingTest, StandardOriginCircle2D_orientation)
{
  StandardOriginCircle2D  circle(1.0);
  EXPECT_EQ(CCW, circle.orientation());

  StandardOriginCircle2D  reversedCircle(-1.0);
  EXPECT_EQ(CW, reversedCircle.orientation());

  StandardOriginCircle2D  line(0.0);
  EXPECT_EQ(CCW, line.orientation());

  StandardOriginCircle2D  reversedLine(-0.0);
  EXPECT_EQ(CW, reversedLine.orientation());
}



TEST_F(CDCLocalTrackingTest, StandardOriginCircle2D_closest)
{
  StandardOriginCircle2D  circle(1.0);
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



TEST_F(CDCLocalTrackingTest, StandardOriginCircle2D_arcLengthFactor)
{
  StandardOriginCircle2D  circle(1.0);

  FloatType smallAngle = PI / 100;
  Vector2D near(1.0 - cos(smallAngle), sin(smallAngle));

  FloatType expectedArcLengthFactor = smallAngle / near.polarR();
  EXPECT_NEAR(expectedArcLengthFactor, circle.arcLengthFactor(near.polarR()), 10e-7);

}





TEST_F(CDCLocalTrackingTest, StandardOriginCircle2D_lengthOnCurve)
{
  StandardOriginCircle2D  circle(1.0);

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


  StandardOriginCircle2D line(0.0);
  EXPECT_NEAR(-2, line.lengthOnCurve(origin, up), 10e-7);
  EXPECT_NEAR(2, line.lengthOnCurve(origin, down), 10e-7);
  EXPECT_NEAR(0, line.lengthOnCurve(origin, far), 10e-7);


  StandardOriginCircle2D reverseLine(-0.0);
  EXPECT_NEAR(2, reverseLine.lengthOnCurve(origin, up), 10e-7);
  EXPECT_NEAR(-2, reverseLine.lengthOnCurve(origin, down), 10e-7);
  EXPECT_NEAR(0, reverseLine.lengthOnCurve(origin, far), 10e-7);

}
