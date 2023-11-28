/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/ConditionalGaussGenerator.h>
#include <framework/utilities/MultivariateNormalGenerator.h>

#include <TRandom.h>
#include <gtest/gtest.h>
#include <cmath>
#include <iostream>

using namespace Belle2;

namespace {

  /** Tests for classical and conditional Gaussian random number generator */
  class GaussGeneratorsTests : public ::testing::Test {
  protected:

    /** dummy setup */
    virtual void SetUp() { }

    /** dummy */
    virtual void TearDown() { }

  };

  /** Tests the classical generator on 6D vector */
  TEST_F(GaussGeneratorsTests, Generator6DTest)
  {
    Eigen::MatrixXd cov(6, 6);

    // generate random matrix
    for (int i = 0; i < 6; ++i)
      for (int j = 0; j < 6; ++j)
        cov(i, j) = gRandom->Poisson(20) - 20;

    // symmetrise it to obtain covariance
    cov *= cov.transpose();

    // some random central values
    Eigen::VectorXd mu(6);
    mu << 3, 0, -2,  7, -5, -1; // cppcheck-suppress constStatement

    // init the generator
    MultivariateNormalGenerator gen(mu, cov);

    // calculate the mean and cov of the generated events
    int N = 50000000;
    Eigen::VectorXd muS  = Eigen::VectorXd::Zero(6);
    Eigen::MatrixXd covS = Eigen::MatrixXd::Zero(6, 6);
    for (int i = 0; i < N; ++i) {
      // generate whole random vector
      Eigen::VectorXd x = gen.generate();
      muS  += x / N;
      covS += (x - mu) * (x - mu).transpose() / N;
    }

    // check that mean vector and calculated cov agree with expectation
    EXPECT_NEAR((muS  - mu).squaredNorm(), 0.0, 1e-2);
    EXPECT_NEAR((covS - cov).squaredNorm(), 0.0, 5e-2);

  }


  /** Tests the conditional generator on 2D vector */
  TEST_F(GaussGeneratorsTests, ConditionalGenerator2DTest)
  {
    //some random cov matrix
    Eigen::MatrixXd cov(2, 2);
    cov <<  9, -4,
        -4,  4; // cppcheck-suppress constStatement

    // some mean
    Eigen::VectorXd mu(2);
    mu << 5., 9.; // cppcheck-suppress constStatement

    // init the generator
    ConditionalGaussGenerator gen(mu, cov);

    // generate the events using conditional generator
    int N = 10000000;
    Eigen::VectorXd muS  = Eigen::VectorXd::Zero(2);
    Eigen::MatrixXd covS = Eigen::MatrixXd::Zero(2, 2);
    for (int i = 0; i < N; ++i) {
      // generate x0 using "external" random generator
      double x0 = gRandom->Gaus(mu[0], sqrt(cov(0, 0)));

      // generate whole random vector based on x0
      Eigen::VectorXd x = gen.generate(x0);
      muS  += x / N;
      covS += (x - mu) * (x - mu).transpose() / N;
    }

    // check that mean vector and calculated cov agree with expectation
    EXPECT_NEAR((muS  - mu).squaredNorm(), 0.0, 1e-3);
    EXPECT_NEAR((covS - cov).squaredNorm(), 0.0, 1e-3);

  }

  /** Tests the conditional generator on 6D vector */
  TEST_F(GaussGeneratorsTests, ConditionalGenerator6DTest)
  {
    Eigen::MatrixXd cov(6, 6);

    // generate random matrix
    for (int i = 0; i < 6; ++i)
      for (int j = 0; j < 6; ++j)
        cov(i, j) = gRandom->Poisson(20) - 20;

    // symmetrise it to obtain covariance
    cov *= cov.transpose();

    // some random central values
    Eigen::VectorXd mu(6);
    mu << 3, 0, -2,  7, -5, -1; // cppcheck-suppress constStatement

    // init the generator
    ConditionalGaussGenerator gen(mu, cov);

    // calculate the mean and cov of the generated events
    int N = 50000000;
    Eigen::VectorXd muS  = Eigen::VectorXd::Zero(6);
    Eigen::MatrixXd covS = Eigen::MatrixXd::Zero(6, 6);
    for (int i = 0; i < N; ++i) {
      // generate x0 using "external" random generator
      double x0 = gRandom->Gaus(mu[0], sqrt(cov(0, 0)));

      // generate whole random vector based on x0
      Eigen::VectorXd x = gen.generate(x0);
      muS  += x / N;
      covS += (x - mu) * (x - mu).transpose() / N;
    }

    // check that mean vector and calculated cov agree with expectation
    EXPECT_NEAR((muS  - mu).squaredNorm(), 0.0, 1e-2);
    EXPECT_NEAR((covS - cov).squaredNorm(), 0.0, 5e-2);

  }

}
