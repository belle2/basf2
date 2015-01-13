/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/*
This file contains test to check the behaviour of ROOT.
Its purpose is mainly to asure the programmer that his assumptions about run time behaviour are correct.
*/

#include "TMatrixD.h"

#include <gtest/gtest.h>

TEST(CDCLocalTrackingTest, root_TMatrixD)
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


