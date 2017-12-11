#include "analysis/VertexFitting/TreeFitter/FitParams.h"
#include "analysis/VertexFitting/TreeFitter/KalmanCalculator.h"
#include <Eigen/Dense>

#include <gtest/gtest.h>


namespace Belle2 {
  /** Test fixture. */
  class TreeFitterKalmanCalculatorTest : public ::testing::Test {
  protected:
  };

  TEST_F(TreeFitterKalmanCalculatorTest, Functions)
  {
    TreeFitter::KalmanCalculator kalman;
    TreeFitter::FitParams fitParDim6(6);

    fitParDim6.getStateVector() << Eigen::Matrix<double, 6, 1>::Zero(6, 1);

    fitParDim6.getCovariance()  = 2 * Eigen::Matrix<double, 6, 6>::Identity(6, 6);
    const Eigen::Matrix < double, -1, -1 > * V = NULL;
    Eigen::Matrix<double, 3, 6> G;
    G << 1, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 1;
    const  Eigen::Matrix<double, 3, 6>& c_G = G;
    //  The kalman matrix m_K should be this after calculation:
    //  0.5 0    0
    //  0   0.5  0
    //  0   0    0.5
    //  0.5 0    0
    //  0   0.5  0
    //  0   0    0.5
    //
    Eigen::Matrix<double, 1, 3> residuals;
    residuals << .1, .2, .3;
    const Eigen::Matrix<double, 1, 3>& c_r = residuals;


    kalman.calculateGainMatrix(c_r, c_G, &fitParDim6, V, 0);

    kalman.updateState(&fitParDim6);

    Eigen::Matrix<double, 6, 1> expectedUpdatedFitpars;
    expectedUpdatedFitpars << -0.05, -0.1, -0.15, -0.05, -0.1, -0.15;

    EXPECT_TRUE(expectedUpdatedFitpars.isApprox(fitParDim6.getStateVector().segment(0, 6)));

    Eigen::Matrix<double, 6, 6> expectedUpdatedCov = Eigen::Matrix<double, 6, 6>::Identity(6, 6);
    expectedUpdatedCov.diagonal < -3 > () << -1, -1, -1;
    expectedUpdatedCov.diagonal<3>() << -1, -1, -1;

    kalman.updateCovariance(&fitParDim6);
    Eigen::Matrix<double, 6, 6> updatedCov = fitParDim6.getCovariance().selfadjointView<Eigen::Lower>();


    EXPECT_TRUE(updatedCov.isApprox(expectedUpdatedCov));

  }

}  // namespace
