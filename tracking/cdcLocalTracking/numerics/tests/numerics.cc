/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/cdcLocalTracking/numerics/SinEqLine.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>

#include <tracking/cdcLocalTracking/geometry/Line2D.h>

#include <framework/gearbox/Unit.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTest, numerics_sign)
{
  EXPECT_EQ(PLUS, sign(INFINITY));
  EXPECT_EQ(PLUS, sign(2));
  EXPECT_EQ(PLUS, sign(0.0));
  EXPECT_EQ(MINUS, sign(-0.0));
  EXPECT_EQ(MINUS, sign(-2));
  EXPECT_EQ(MINUS, sign(-INFINITY));
  EXPECT_EQ(INVALID_SIGN, sign(NAN));

  EXPECT_TRUE(isValidSign(PLUS));
  EXPECT_TRUE(isValidSign(MINUS));
  EXPECT_TRUE(isValidSign(ZERO));

  EXPECT_FALSE(isValidSign(INVALID_SIGN));
  EXPECT_FALSE(isValidSign(7));

  EXPECT_EQ(MINUS, oppositeSign(PLUS));
  EXPECT_EQ(PLUS, oppositeSign(MINUS));
  EXPECT_EQ(ZERO, oppositeSign(ZERO));
  EXPECT_EQ(INVALID_SIGN, oppositeSign(INVALID_SIGN));
}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_isIncreasing)
{

  Vector2D lower(0.0, 1.0);
  Vector2D upper(1.0, 2.0);

  EXPECT_EQ(INCREASING, SinEqLine::getIncDecInfo(lower, upper));

}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_getIPeriodFromIHalfPeriod)
{
  Index iHalfPeriod = -1;
  Index iPeriod = SinEqLine::getIPeriodFromIHalfPeriod(iHalfPeriod);
  EXPECT_EQ(-1, iPeriod);
}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootInInterval_simple)
{

  // Simple sin.
  SinEqLine sinEqLine(0.0, 0.0);

  FloatType rootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  FloatType rootY = sinEqLine.map(rootX);

  EXPECT_NEAR(PI, rootX, 10e-7);
  EXPECT_NEAR(0.0, rootY, 10e-7);

}



TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootInInterval_const)
{

  // Constant sin.
  SinEqLine sinEqLine(0.0, 1.0 / 2.0);

  FloatType rootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  FloatType rootY = sinEqLine.map(rootX);

  EXPECT_NEAR(150.0 * Unit::deg, rootX, 10e-7);
  EXPECT_NEAR(0.0, rootY, 10e-7);

}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootInInterval_complex)
{

  // Setup a line that is a
  FloatType rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootLargerThanExtemumInHalfPeriod_simple)
{

  // Setup a line that is a
  FloatType rootX = 0.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(-1.0 / 2.0, 0);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeRootLargerThanExtemumInHalfPeriod(-1);
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootLargerThanExtemumInHalfPeriod)
{

  // Setup a line that is a
  FloatType rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeRootLargerThanExtemumInHalfPeriod(0);
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeSmallestPositiveRoot)
{

  // Setup a line that is a
  FloatType rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeSmallestPositiveRoot();
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}



TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeSmallestPositiveRoot_largeSlope)
{

  // Setup a line that is a
  FloatType rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeSmallestPositiveRoot();
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}



TEST(CDCLocalTrackingTest, numerics_SinEqLine_computeRootForLargeSlope)
{
  // Setup a line that is a
  FloatType rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  FloatType solvedRootX = sinEqLine.computeRootForLargeSlope();
  FloatType solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);
}
