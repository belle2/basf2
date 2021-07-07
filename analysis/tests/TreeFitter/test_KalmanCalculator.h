/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "analysis/VertexFitting/TreeFitter/FitParams.h"
#include "analysis/VertexFitting/TreeFitter/KalmanCalculator.h"

namespace {

  /** Test fixture. */
  class TreeFitterKalmanCalculatorTest : public ::testing::Test {
  protected:
  };

  /** The KalmanCalculator test itself. */
  TEST_F(TreeFitterKalmanCalculatorTest, Functions)
  {
    TreeFitter::KalmanCalculator kalman(3, 6);
    TreeFitter::FitParams fitParDim6(6);

    fitParDim6.getStateVector() << Eigen::Matrix<double, 6, 1>::Zero(6, 1);

    fitParDim6.getCovariance()  = 2 * Eigen::Matrix<double, 6, 6>::Identity(6, 6);

    Eigen::Matrix<double, 3, 6> G;

    // cppcheck-suppress constStatement
    G << 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1;

    const  Eigen::Matrix<double, 3, 6>& c_G = G;

    //  The kalman matrix m_K should be this after calculation:
    //  0.5 0    0
    //  0   0.5  0
    //  0   0    0.5
    //  0.5 0    0
    //  0   0.5  0
    //  0   0    0.5
    //
    Eigen::Matrix<double, 3, 1> residuals;
    // cppcheck-suppress constStatement
    residuals << .1, .2, .3;
    const Eigen::Matrix<double, 3, 1>& c_r = residuals;

    kalman.calculateGainMatrix(c_r, c_G, fitParDim6, nullptr, 0);

    kalman.updateState(fitParDim6);

    Eigen::Matrix<double, 6, 1> expectedUpdatedFitpars;
    // cppcheck-suppress constStatement
    expectedUpdatedFitpars << -0.05, -0.1, -0.15, -0.05, -0.1, -0.15;

    EXPECT_TRUE(expectedUpdatedFitpars.isApprox(fitParDim6.getStateVector().segment(0, 6))) << "fitpar update failed.";

    Eigen::Matrix<double, 6, 6> expectedUpdatedCov = Eigen::Matrix<double, 6, 6>::Identity(6, 6);
    // cppcheck-suppress constStatement
    expectedUpdatedCov.diagonal < -3 > () << -1, -1, -1;
    // cppcheck-suppress constStatement
    expectedUpdatedCov.diagonal<3>() << -1, -1, -1;

    kalman.updateCovariance(fitParDim6);
    Eigen::Matrix<double, 6, 6> updatedCov = fitParDim6.getCovariance().selfadjointView<Eigen::Lower>();


    EXPECT_TRUE(updatedCov.isApprox(expectedUpdatedCov)) << "covariance update failed.";

  }

}  // namespace
