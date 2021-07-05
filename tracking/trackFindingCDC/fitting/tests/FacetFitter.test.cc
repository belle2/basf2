/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/LineParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/Matrix.h>

#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine)
{
  Matrix<double, 3, 1> weights = Matrix<double, 3, 1>::Constant(1.0 / 4.0);
  Matrix<double, 3, 3> xyl = Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = -1;
  xyl(0, 1) = 1.1;
  xyl(0, 2) = -0.05;

  xyl(1, 0) = 0;
  xyl(1, 1) = 0.8;
  xyl(1, 2) = 0.1;

  xyl(2, 0) = 1;
  xyl(2, 1) = 1.1;
  xyl(2, 2) = -0.05;

  for (int nSteps : {100, 1, 0}) {
    UncertainParameterLine2D fittedLine = FacetFitter::fit(xyl, weights, nSteps);

    EXPECT_NEAR(1, fittedLine->tangential().x(), 1e-6);
    EXPECT_NEAR(0, fittedLine->tangential().y(), 1e-6);
    EXPECT_NEAR(0, fittedLine->support().x(), 1e-6);
    EXPECT_NEAR(1, fittedLine->support().y(), 1e-6);
    EXPECT_NEAR(0.015 * 1.0 / 4.0, fittedLine.chi2(), 1e-6);
    B2INFO(fittedLine.lineCovariance());
  }
}


TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine_alongYAxes)
{
  Matrix<double, 3, 1> weights = Matrix<double, 3, 1>::Constant(1.0);
  Matrix<double, 3, 3> xyl = Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = 1.1;
  xyl(0, 1) = -1;
  xyl(0, 2) = 0.05;

  xyl(1, 0) = 0.8;
  xyl(1, 1) = 0;
  xyl(1, 2) = -0.1;

  xyl(2, 0) = 1.1;
  xyl(2, 1) = 1;
  xyl(2, 2) = 0.05;

  for (int nSteps : {100, 1, 0}) {
    UncertainParameterLine2D fittedLine = FacetFitter::fit(xyl, weights, nSteps);

    EXPECT_NEAR(0, fittedLine->tangential().x(), 1e-6);
    EXPECT_NEAR(1, fittedLine->tangential().y(), 1e-6);
    EXPECT_NEAR(1, fittedLine->support().x(), 1e-6);
    EXPECT_NEAR(0, fittedLine->support().y(), 1e-6);
    EXPECT_NEAR(0.015, fittedLine.chi2(), 1e-6);
    B2INFO(fittedLine.lineCovariance());
  }
}



TEST(TrackFindingCDCTest, fitting_FacetFitter_fitLine_sameSide)
{
  Matrix<double, 3, 1> weights = Matrix<double, 3, 1>::Constant(1);
  Matrix<double, 3, 3> xyl = Matrix<double, 3, 3>::Zero();

  xyl(0, 0) = 1;
  xyl(0, 1) = 0.8;
  xyl(0, 2) = -0.1;

  xyl(1, 0) = 0;
  xyl(1, 1) = 0.8;
  xyl(1, 2) = -0.4;

  xyl(2, 0) = -1;
  xyl(2, 1) = 0.8;
  xyl(2, 2) = -0.1;

  for (int nSteps : {100, 1, 0}) {
    UncertainParameterLine2D fittedLine = FacetFitter::fit(xyl, weights, nSteps);

    EXPECT_NEAR(-1, fittedLine->tangential().x(), 1e-6);
    EXPECT_NEAR(0, fittedLine->tangential().y(), 1e-6);
    EXPECT_NEAR(0, fittedLine->support().x(), 1e-6);
    EXPECT_NEAR(1, fittedLine->support().y(), 1e-6);
    EXPECT_NEAR(0.06, fittedLine.chi2(), 1e-6);
    B2INFO(fittedLine.lineCovariance());
  }
}
