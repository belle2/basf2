/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include <tracking/ckf/utilities/EigenHelper.h>

#include <TMatrixD.h>
#include <TVectorD.h>

using namespace Belle2;

TEST(TrackingCKFTest, eigen_matrix_conversion_TMatrixD)
{
  const double rootMatrixContent[2 * 5] = {1, 2, 3, 4, 5,
                                           6, 7, 8, 9, 10
                                          };

  const TMatrixD rootMatrix(2, 5, rootMatrixContent);

  const auto& eigenMatrix = convertToEigen<2, 5>(rootMatrix);
  for (unsigned int row = 0; row < 2; row++) {
    for (unsigned int col = 0; col < 5; col++) {
      ASSERT_EQ(eigenMatrix(row, col), rootMatrix(row, col));
    }
  }
}
TEST(TrackingCKFTest, eigen_matrix_conversion_TVectorD)
{
  const double rootMatrixContent[2 * 5] = {1, 2, 3, 4, 5};
  const TVectorD rootMatrix(5, rootMatrixContent);

  const auto& eigenMatrix = convertToEigen<5>(rootMatrix);
  for (unsigned int row = 0; row < 5; row++) {
    ASSERT_EQ(eigenMatrix(row, 0), rootMatrix(row));
  }
}
