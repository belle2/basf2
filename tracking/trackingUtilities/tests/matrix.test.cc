/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/*
This file contains some basic investigation on numerically matrix algorithms.
*/

#include <gtest/gtest.h>

#include <vector>
#include <Eigen/Dense>


// 1. Study of matrix inversions

// TL;DR use a rank revealing matrix inversion (usually involving pivoting)
// * pivoted householder QR decomposition
// * full pivoted LU

// no good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, TMatrixDSym_invert)
// {
//   TMatrixDSym weight(2);
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   EXPECT_EQ(2, weight.GetNrows());
//   EXPECT_EQ(2, weight.GetNcols());

//   TMatrixDSym cov = weight;
//   cov.Invert();

//   EXPECT_EQ(2, weight.GetNrows());
//   EXPECT_EQ(2, weight.GetNcols());

//   // Already fails here
//   EXPECT_EQ(2, cov.GetNrows());
//   EXPECT_EQ(2, cov.GetNcols());

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// no good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, SMatrixDSym_inverse)
// {
//   using CovarianceMatrix = ROOT::Math::SMatrix< double, 2, 2, ROOT::Math::MatRepSym< double, 2>
//   >;
//   using WeightMatrix = ROOT::Math::SMatrix< double, 2, 2, ROOT::Math::MatRepSym< double, 2> >;

//   WeightMatrix weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   int state = 0;
//   CovarianceMatrix cov = weight.Inverse(state);

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// no good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, SMatrixDSym_inverseChol)
// {
//   using CovarianceMatrix = ROOT::Math::SMatrix< double, 2, 2, ROOT::Math::MatRepSym< double, 2>
//   >;
//   using WeightMatrix = ROOT::Math::SMatrix< double, 2, 2, ROOT::Math::MatRepSym< double, 2> >;

//   WeightMatrix weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   int state = 0;
//   CovarianceMatrix cov = weight.InverseChol(state);

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// no good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, Eigen_inverse)
// {
//   Eigen::Matrix<double, 2, 2> weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   Eigen::Matrix<double, 2, 2> cov = weight.inverse();

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// no good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, Eigen_llt_inverse)
// {
//   Eigen::Matrix<double, 2, 2> weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   Eigen::Matrix<double, 2, 2> cov = weight.llt().inverse();

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// No good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, Eigen_lldt_inverse)
// {
//   Eigen::Matrix<double, 2, 2> weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   Eigen::Matrix<double, 2, 2> cov = weight.lldt().inverse();

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

TEST(TrackFindingCDCTest, Eigen_fullPivLu_inverse)
{
  Eigen::Matrix<double, 2, 2> weight;
  weight(0, 0) = 2;
  weight(0, 1) = 0;
  weight(1, 0) = 0;
  weight(1, 1) = 0;

  Eigen::Matrix<double, 2, 2> cov = weight.fullPivLu().inverse();

  EXPECT_NEAR(1.0 / 2.0, cov(0, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(0, 1), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 1), 10e-7);
}

// No good for inversion if rank of the matrix is not full.
// TEST(TrackFindingCDCTest, Eigen_partialPivLu_inverse)
// {
//   Eigen::Matrix<double, 2, 2> weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   Eigen::Matrix<double, 2, 2> cov = weight.partialPivLu().inverse();

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

// Can not do inversion
// TEST(TrackFindingCDCTest, Eigen_householderQr_inverse)
// {
//   Eigen::Matrix<double, 2, 2> weight;
//   weight(0,0) = 2;
//   weight(0,1) = 0;
//   weight(1,0) = 0;
//   weight(1,1) = 0;

//   Eigen::Matrix<double, 2, 2> cov = weight.householderQr().inverse();

//   EXPECT_NEAR(1.0 / 2.0, cov(0,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(0,1), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,0), 10e-7);
//   EXPECT_NEAR(0.0, cov(1,1), 10e-7);
// }

/* Method of choice for covariance <-> weight matrices */
TEST(TrackFindingCDCTest, Eigen_colPivHouseholderQr_inverse)
{
  Eigen::Matrix<double, 2, 2> weight;
  weight(0, 0) = 2;
  weight(0, 1) = 0;
  weight(1, 0) = 0;
  weight(1, 1) = 0;

  Eigen::Matrix<double, 2, 2> cov = weight.colPivHouseholderQr().inverse();

  EXPECT_NEAR(1.0 / 2.0, cov(0, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(0, 1), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 1), 10e-7);
}

TEST(TrackFindingCDCTest, Eigen_fullPivHouseholderQr_inverse)
{
  Eigen::Matrix<double, 2, 2> weight;
  weight(0, 0) = 2;
  weight(0, 1) = 0;
  weight(1, 0) = 0;
  weight(1, 1) = 0;

  Eigen::Matrix<double, 2, 2> cov = weight.fullPivHouseholderQr().inverse();

  EXPECT_NEAR(1.0 / 2.0, cov(0, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(0, 1), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 0), 10e-7);
  EXPECT_NEAR(0.0, cov(1, 1), 10e-7);
}
