/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/*
This file contains test to check the behaviour of ROOT.
Its purpose is mainly to asure the programmer that his assumptions about run time behaviour are correct.
*/

#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <gtest/gtest.h>

TEST(TrackFindingCDCTest, root_TMatrixD)
{
  TMatrixD jacobian(2, 2);
  jacobian(0, 0) = 0;
  jacobian(0, 1) = 1;

  jacobian(1, 0) = -1;
  jacobian(1, 1) = 0;


  TMatrixD covariance(2, 2);
  covariance(0, 0) = 1;
  covariance(0, 1) = 1;

  covariance(1, 0) = 1;
  covariance(1, 1) = 0;

  //result = J * V * J^T
  TMatrixD left(2, 2);
  left.Mult(jacobian, covariance);

  TMatrixD result(2, 2);
  result.MultT(left, jacobian);

  EXPECT_EQ(0, result(0, 0));
  EXPECT_EQ(-1, result(0, 1));
  EXPECT_EQ(-1, result(1, 0));
  EXPECT_EQ(1, result(1, 1));
}


TEST(TrackFindingCDCTest, root_TMatrixDSym_Similarity_differing_rows_columns)
{
  // Test if the number of rows gets augmented correctly,
  // when transformed with a on square jacobian matrix.

  TMatrixDSym cov2(2);
  cov2.Zero();

  cov2(0, 0) = 1;
  cov2(1, 1) = 1;

  EXPECT_EQ(2, cov2.GetNrows());
  EXPECT_EQ(2, cov2.GetNcols());


  TMatrixD jacobianInflate(3, 2);
  jacobianInflate.Zero();

  jacobianInflate(0, 0) = 1;
  jacobianInflate(1, 1) = 1;
  jacobianInflate(2, 0) = 1;
  jacobianInflate(2, 1) = 1;


  cov2.Similarity(jacobianInflate);

  EXPECT_EQ(3, cov2.GetNrows());
  EXPECT_EQ(3, cov2.GetNcols());

  EXPECT_EQ(1, cov2(0, 0));
  EXPECT_EQ(0, cov2(0, 1));
  EXPECT_EQ(1, cov2(0, 2));

  EXPECT_EQ(0, cov2(1, 0));
  EXPECT_EQ(1, cov2(1, 1));
  EXPECT_EQ(1, cov2(1, 2));

  EXPECT_EQ(1, cov2(2, 0));
  EXPECT_EQ(1, cov2(2, 1));
  EXPECT_EQ(2, cov2(2, 2));

}


// // Transform
// TMatrixDSym cov6(6);
// if (true){
//   TMatrixD jacobianInflateT(5, 6);
//   jacobianInflateT.Transpose(jacobianInflate);
//   TMatrixD cov6workaroud = jacobianInflate * cov5 * jacobianInflateT;

//   for (unsigned int i = 0; i < 6; ++i) {
//     for (unsigned int j = 0; j < 6; ++j) {
//  cov6(i, j) = cov6workaroud(i, j);
//     }
//   }
// } else {
//   cov6 = cov5; //copy
//   cov6.Similarity(jacobianInflate);
//   gfTrackCand.setCovSeed(cov6);
// }
