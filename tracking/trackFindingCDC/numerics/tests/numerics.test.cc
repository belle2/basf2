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

#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <tracking/trackFindingCDC/numerics/CovarianceMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/CovarianceMatrix.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrix.h>
#include <tracking/trackFindingCDC/numerics/ParameterVector.h>

#include <tracking/trackFindingCDC/numerics/Median.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <framework/gearbox/Unit.h>

#include <gtest/gtest.h>


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

  EXPECT_TRUE(ESignUtil::isValid(ESign::c_Plus));
  EXPECT_TRUE(ESignUtil::isValid(ESign::c_Minus));
  EXPECT_TRUE(ESignUtil::isValid(ESign::c_Zero));

  EXPECT_FALSE(ESignUtil::isValid(ESign::c_Invalid));
  EXPECT_FALSE(ESignUtil::isValid(static_cast<ESign>(7)));

  EXPECT_EQ(ESign::c_Minus, ESignUtil::opposite(ESign::c_Plus));
  EXPECT_EQ(ESign::c_Plus, ESignUtil::opposite(ESign::c_Minus));
  EXPECT_EQ(ESign::c_Zero, ESignUtil::opposite(ESign::c_Zero));
  EXPECT_EQ(ESign::c_Invalid, ESignUtil::opposite(ESign::c_Invalid));
}


TEST(TrackFindingCDCTest, numerics_SinEqLine_isIncreasing)
{

  Vector2D lower(0.0, 1.0);
  Vector2D upper(1.0, 2.0);

  EXPECT_EQ(EIncDec::c_Increasing, SinEqLine::getEIncDec(lower, upper));

}


TEST(TrackFindingCDCTest, numerics_SinEqLine_getIPeriodFromIHalfPeriod)
{
  int iHalfPeriod = -1;
  int iPeriod = SinEqLine::getIPeriodFromIHalfPeriod(iHalfPeriod);
  EXPECT_EQ(-1, iPeriod);
}


TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootInInterval_simple)
{

  // Simple sin.
  SinEqLine sinEqLine(0.0, 0.0);

  double rootX = sinEqLine.computeRootInInterval(M_PI / 2, 3 * M_PI / 2);
  double rootY = sinEqLine.map(rootX);

  EXPECT_NEAR(M_PI, rootX, 10e-7);
  EXPECT_NEAR(0.0, rootY, 10e-7);

}



TEST(TrackFindingCDCTest, numerics_SinEqLine_computeRootInInterval_const)
{

  // Constant sin.
  SinEqLine sinEqLine(0.0, 1.0 / 2.0);

  double rootX = sinEqLine.computeRootInInterval(M_PI / 2, 3 * M_PI / 2);
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

  double solvedRootX = sinEqLine.computeRootInInterval(M_PI / 2, 3 * M_PI / 2);
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

TEST(TrackFindingCDCTest, numerics_median)
{
  std::vector<double> fourValues{0.0, 3.0, 5.0, 100};
  EXPECT_EQ(4.0, median(std::move(fourValues)));

  std::vector<double> threeValues{0.0, 3.0, 5.0, 100.0, 1000.0};
  EXPECT_EQ(5.0, median(std::move(threeValues)));

  std::vector<WithWeight<double>> weightedValues{{0.0, 0.2},
    {3.0, 0.1},
    {5.0, 0.1},
    {100.0, 0.3},
    {1000.0, 0.3}};
  EXPECT_EQ(100.0, weightedMedian(std::move(weightedValues)));
}

TEST(TrackFindingCDCTest, covariance_simple_average)
{
  // Simple average of two scalar values
  const ParameterVector<1> par1{ -1};
  const CovarianceMatrix<1> cov1{2};

  const ParameterVector<1> par2{1};
  const CovarianceMatrix<1> cov2{2};

  ParameterVector<1> par{};
  CovarianceMatrix<1> cov{};

  double chi2 = CovarianceMatrixUtil::average(par1, cov1, par2, cov2, par, cov);
  EXPECT_NEAR(0, par[0], 1E-8);
  EXPECT_NEAR(1, cov[0], 1E-8);
  EXPECT_NEAR(1, chi2, 1E-8);
}

TEST(TrackFindingCDCTest, covariance_half_projection_average)
{
  // Same average of two scalar values but one is 'projected' stretched by a factor of 2
  const JacobianMatrix<1, 1> ambi1{2};// Ambiguity is 2.
  const ParameterVector<1> par1{ -2}; // Parameter scales with 2
  const CovarianceMatrix<1> cov1{8};  // Covariance scales with 2 * 2

  const ParameterVector<1> par2{1};
  const CovarianceMatrix<1> cov2{2};

  ParameterVector<1> par{};
  CovarianceMatrix<1> cov{};

  double chi2 = CovarianceMatrixUtil::average(par1, cov1, ambi1, par2, cov2, par, cov);
  EXPECT_NEAR(0, par[0], 1E-8);
  EXPECT_NEAR(1, cov[0], 1E-8);
  EXPECT_NEAR(1, chi2, 1E-8);
}

TEST(TrackFindingCDCTest, covariance_kalman_update_average)
{
  // Same average of two scalar values but one is 'projected' / stretched by a factor of 2
  // Now as an inplace update using the Kalman formula.
  const JacobianMatrix<1, 1> ambi1{2}; // Ambiguity is 2.
  const ParameterVector<1> par1{ -2};  // Parameter scales with 2
  const CovarianceMatrix<1> cov1{8};   // Covariance scales with 2 * 2

  ParameterVector<1> par{1};
  CovarianceMatrix<1> cov{2};

  double chi2 = CovarianceMatrixUtil::kalmanUpdate(par1, cov1, ambi1, par, cov);
  EXPECT_NEAR(0, par[0], 1E-8);
  EXPECT_NEAR(1, cov[0], 1E-8);
  EXPECT_NEAR(1, chi2, 1E-8);
}
