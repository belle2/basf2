/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine)
{
  Eigen::Array<double, 3, 1> weights = Eigen::Array<double, 3, 1>::Constant(1.0 / 4.0);
  Eigen::Matrix<double, 3, 3> xyl = Eigen::Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = -1;
  xyl(0, 1) = 1.1;
  xyl(0, 2) = -0.05;

  xyl(1, 0) = 0;
  xyl(1, 1) = 0.8;
  xyl(1, 2) = 0.1;

  xyl(2, 0) = 1;
  xyl(2, 1) = 1.1;
  xyl(2, 2) = -0.05;

  double chi2 = 0;
  Line2D fittedLine = FacetFitter::fit(xyl, weights, chi2);

  EXPECT_NEAR(1, fittedLine.n0(), 1e-6);
  EXPECT_NEAR(0, fittedLine.n1(), 1e-6);
  EXPECT_NEAR(-1, fittedLine.n2(), 1e-6);
  EXPECT_NEAR(0.015 * 1.0 / 4.0, chi2, 1e-6);
}


TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine_alongYAxes)
{
  Eigen::Array<double, 3, 1> weights = Eigen::Array<double, 3, 1>::Ones();
  Eigen::Matrix<double, 3, 3> xyl = Eigen::Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = 1.1;
  xyl(0, 1) = -1;
  xyl(0, 2) = 0.05;

  xyl(1, 0) = 0.8;
  xyl(1, 1) = 0;
  xyl(1, 2) = -0.1;

  xyl(2, 0) = 1.1;
  xyl(2, 1) = 1;
  xyl(2, 2) = 0.05;

  double chi2 = 0;
  Line2D fittedLine = FacetFitter::fit(xyl, weights, chi2);

  EXPECT_NEAR(-1, fittedLine.n0(), 1e-6);
  EXPECT_NEAR(1, fittedLine.n1(), 1e-6);
  EXPECT_NEAR(0, fittedLine.n2(), 1e-6);
  EXPECT_NEAR(0.015, chi2, 1e-6);
}



TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine_sameSide)
{
  Eigen::Array<double, 3, 1> weights = Eigen::Array<double, 3, 1>::Ones();
  Eigen::Matrix<double, 3, 3> xyl = Eigen::Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = 1;
  xyl(0, 1) = 0.8;
  xyl(0, 2) = -0.1;

  xyl(1, 0) = 0;
  xyl(1, 1) = 0.8;
  xyl(1, 2) = -0.4;

  xyl(2, 0) = -1;
  xyl(2, 1) = 0.8;
  xyl(2, 2) = -0.1;

  double chi2 = 0;
  Line2D fittedLine = FacetFitter::fit(xyl, weights, chi2);

  EXPECT_NEAR(-1, fittedLine.n0(), 1e-6);
  EXPECT_NEAR(0, fittedLine.n1(), 1e-6);
  EXPECT_NEAR(1, fittedLine.n2(), 1e-6);
  EXPECT_NEAR(0.06, chi2, 1e-6);
}
