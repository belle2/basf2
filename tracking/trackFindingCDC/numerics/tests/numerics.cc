/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/SinEqLine.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <framework/gearbox/Unit.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, numerics_sign)
{
  EXPECT_EQ(ESign::c_Plus, sign(INFINITY));
  EXPECT_EQ(ESign::c_Plus, sign(2));
  EXPECT_EQ(ESign::c_Plus, sign(0.0));
  EXPECT_EQ(ESign::c_Minus, sign(-0.0));
  EXPECT_EQ(ESign::c_Minus, sign(-2));
  EXPECT_EQ(ESign::c_Minus, sign(-INFINITY));
  EXPECT_EQ(ESign::c_Invalid, sign(NAN));

  EXPECT_TRUE(isValidSign(ESign::c_Plus));
  EXPECT_TRUE(isValidSign(ESign::c_Minus));
  EXPECT_TRUE(isValidSign(ESign::c_Zero));

  EXPECT_FALSE(isValidSign(ESign::c_Invalid));
  EXPECT_FALSE(isValidSign(static_cast<ESign>(7)));

  EXPECT_EQ(ESign::c_Minus, oppositeSign(ESign::c_Plus));
  EXPECT_EQ(ESign::c_Plus, oppositeSign(ESign::c_Minus));
  EXPECT_EQ(ESign::c_Zero, oppositeSign(ESign::c_Zero));
  EXPECT_EQ(ESign::c_Invalid, oppositeSign(ESign::c_Invalid));
}


TEST(TrackFindingCDCTest, numerics_SinEqLine_isIncreasing)
{

  Vector2D lower(0.0, 1.0);
  Vector2D upper(1.0, 2.0);

  EXPECT_EQ(INCREASING, SinEqLine::getIncDecInfo(lower, upper));

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_getIPeriodFromIHalfPeriod)
{
  Index iHalfPeriod = -1;
  Index iPeriod = SinEqLine::getIPeriodFromIHalfPeriod(iHalfPeriod);
  EXPECT_EQ(-1, iPeriod);
}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootInInterval_simple)
{

  // Simple sin.
  SinEqLine sinEqLine(0.0, 0.0);

  double rootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  double rootY = sinEqLine.map(rootX);

  EXPECT_NEAR(PI, rootX, 10e-7);
  EXPECT_NEAR(0.0, rootY, 10e-7);

}



TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootInInterval_const)
{

  // Constant sin.
  SinEqLine sinEqLine(0.0, 1.0 / 2.0);

  double rootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  double rootY = sinEqLine.map(rootX);

  EXPECT_NEAR(150.0 * Unit::deg, rootX, 10e-7);
  EXPECT_NEAR(0.0, rootY, 10e-7);

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootInInterval_complex)
{

  // Setup a line that is a
  double rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeRootInInterval(PI / 2, 3 * PI / 2);
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootLargerThanExtemumInHalfPeriod_simple)
{

  // Setup a line that is a
  double rootX = 0.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(-1.0 / 2.0, 0);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeRootLargerThanExtemumInHalfPeriod(-1);
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootLargerThanExtemumInHalfPeriod)
{

  // Setup a line that is a
  double rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeRootLargerThanExtemumInHalfPeriod(0);
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeSmallestPositiveRoot)
{

  // Setup a line that is a
  double rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(1.0 / 2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeSmallestPositiveRoot();
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}



TEST(TrackFindingCDCTest, numerics_SinEqLine_computeSmallestPositiveRoot_largeSlope)
{

  // Setup a line that is a
  double rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeSmallestPositiveRoot();
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);

}



TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootForLargeSlope)
{
  // Setup a line that is a
  double rootX = 150.0 * Unit::deg;
  Line2D line = Line2D::fromSlopeIntercept(2.0, 1.0 / 2.0);
  line.moveAlongFirst(rootX);

  SinEqLine sinEqLine(line);

  double solvedRootX = sinEqLine.computeRootForLargeSlope();
  double solvedRootY = sinEqLine.map(rootX);

  EXPECT_NEAR(rootX, solvedRootX, 10e-7);
  EXPECT_NEAR(0.0, solvedRootY, 10e-7);
}
