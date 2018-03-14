/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/PlainMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/PlainMatrix.h>

#include <tracking/trackFindingCDC/numerics/EigenView.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, PlainMatrix_zero)
{
  /// Implicit zero
  {
    PlainMatrix<double, 5, 5> result{{}};
    for (int i = 0; i < result.rows(); ++i) {
      for (int j = 0; j < result.cols(); ++j) {
        EXPECT_EQ(0, (result(i, j)));
      }
    }
  }

  /// Explicit zero
  {
    auto result = PlainMatrix<double, 5, 5>::Zero();
    for (int i = 0; i < result.rows(); ++i) {
      for (int j = 0; j < result.cols(); ++j) {
        EXPECT_EQ(0, (result(i, j)));
      }
    }
  }
}


TEST(TrackFindingCDCTest, PlainMatrix_id)
{
  auto result = PlainMatrix<double, 5, 5>::Identity();
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      if (i == j) {
        EXPECT_EQ(1, (result(i, j)));
      } else {
        EXPECT_EQ(0, (result(i, j)));
      }
    }
  }
}


TEST(TrackFindingCDCTest, PlainMatrix_diag)
{
  PlainMatrix<double, 5, 1> diagElements{{0, 1, 2, 3, 4}};
  auto result = PlainMatrixUtil::Diag(diagElements);

  EXPECT_EQ(5, result.cols());
  EXPECT_EQ(5, result.rows());

  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      if (i == j) {
        EXPECT_EQ(i, result(i, j));
      } else {
        EXPECT_EQ(0, result(i, j));
      }
    }
  }
}


TEST(TrackFindingCDCTest, PlainMatrix_constant)
{
  auto result = PlainMatrix<double, 5, 5>::Constant(3);
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(3, (result(i, j)));
    }
  }
}


TEST(TrackFindingCDCTest, PlainMatrix_aggregate_initialization)
{
  auto result = PlainMatrix<double, 2, 1>({ -1.0, 1.0});
  EXPECT_EQ(-1, result(0, 0));
  EXPECT_EQ(1, result(1, 0));

  auto result2 = []() {
    return PlainMatrix<double, 2, 1>({ -1.0, 1.0});
  }();

  EXPECT_EQ(-1, result2(0, 0));
  EXPECT_EQ(1, result2(1, 0));
}


TEST(TrackFindingCDCTest, PlainMatrix_vstack)
{

  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 3, 1>({3, 4, 5});

  auto result = PlainMatrixUtil::VStack(a, b);

  EXPECT_EQ(1, result.cols());
  EXPECT_EQ(5, result.rows());

  EXPECT_EQ(1, result(0, 0));
  EXPECT_EQ(2, result(1, 0));
  EXPECT_EQ(3, result(2, 0));
  EXPECT_EQ(4, result(3, 0));
  EXPECT_EQ(5, result(4, 0));
}

TEST(TrackFindingCDCTest, PlainMatrix_hstack)
{

  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 2, 1>({3, 4});

  auto result = PlainMatrixUtil::HStack(a, b);

  EXPECT_EQ(2, result.cols());
  EXPECT_EQ(2, result.rows());

  EXPECT_EQ(1, result(0, 0));
  EXPECT_EQ(2, result(1, 0));
  EXPECT_EQ(3, result(0, 1));
  EXPECT_EQ(4, result(1, 1));
}



TEST(TrackFindingCDCTest, PlainMatrix_blockstack)
{

  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 3, 1>({3, 4, 5});

  auto result = PlainMatrixUtil::BlockStack(a, b);

  EXPECT_EQ(2, result.cols());
  EXPECT_EQ(5, result.rows());
  EXPECT_EQ(1, result(0, 0));
  EXPECT_EQ(2, result(1, 0));
  EXPECT_EQ(0, result(2, 0));
  EXPECT_EQ(0, result(3, 0));
  EXPECT_EQ(0, result(4, 0));

  EXPECT_EQ(0, result(0, 1));
  EXPECT_EQ(0, result(1, 1));
  EXPECT_EQ(3, result(2, 1));
  EXPECT_EQ(4, result(3, 1));
  EXPECT_EQ(5, result(4, 1));
}

TEST(TrackFindingCDCTest, PlainMatrix_element_access)
{
  auto a = PlainMatrix<double, 3, 1>::Zero();
  a(0, 0) = 1;
  a(1, 0) = 2;
  a[2] = 3;

  EXPECT_EQ(1, a[0]);
  EXPECT_EQ(2, a(1, 0));
  EXPECT_EQ(3, a(2));
}

TEST(TrackFindingCDCTest, PlainMatrix_minus)
{
  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 2, 1>({2, 3});

  auto result = b - a;
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(1 , result(i, j));
    }
  }
}

TEST(TrackFindingCDCTest, PlainMatrix_plus)
{
  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 2, 1>({2, 1});

  auto result = b + a;
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(3, result(i, j));
    }
  }
}

TEST(TrackFindingCDCTest, PlainMatrix_divide)
{
  auto a = PlainMatrix<double, 2, 1>({2, 2});

  auto result = a / 2.0;
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(1, result(i, j));
    }
  }
}


TEST(TrackFindingCDCTest, PlainMatrix_inverse_divide)
{
  auto a = PlainMatrix<double, 2, 1>({2, 2});

  auto result = 2.0 / a;
  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(1, result(i, j));
    }
  }
}

TEST(TrackFindingCDCTest, PlainMatrix_muliplication)
{
  auto a = PlainMatrix<double, 2, 1>({2, 2});
  auto b = PlainMatrix<double, 1, 2>({1.0 / 4.0, 1.0 / 4.0});

  auto result = b * a;

  EXPECT_EQ(1, result.cols());
  EXPECT_EQ(1, result.rows());

  EXPECT_EQ(1, result(0, 0));

  auto result2 = a * b;

  EXPECT_EQ(2, result2.cols());
  EXPECT_EQ(2, result2.rows());

  EXPECT_EQ(1.0 / 2.0, result2(0, 0));
  EXPECT_EQ(1.0 / 2.0, result2(1, 0));
  EXPECT_EQ(1.0 / 2.0, result2(0, 1));
  EXPECT_EQ(1.0 / 2.0, result2(1, 1));
}

TEST(TrackFindingCDCTest, PlainMatrix_block)
{
  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 3, 1>({3, 4, 5});

  auto result = PlainMatrixUtil::BlockStack(a, b);

  auto aNew = result.block<2, 1>(0, 0);
  auto bNew = result.block<3, 1>(2, 1);

  EXPECT_TRUE(std::equal(a.data(), a.data() + a.size(), aNew.data()));
  EXPECT_TRUE(std::equal(b.data(), b.data() + b.size(), bNew.data()));
}

TEST(TrackFindingCDCTest, PlainMatrixEigenMap_mapToEigen)
{
  auto a = PlainMatrix<double, 2, 1>({1, 2});
  auto b = PlainMatrix<double, 3, 1>({3, 4, 5});

  auto result = PlainMatrixUtil::BlockStack(a, b);
  auto eigenMap = mapToEigen(result);

  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(result(i, j) , eigenMap(i, j));
    }
  }

  // Test write-through
  eigenMap(0, 0) = 0;
  EXPECT_EQ(0, eigenMap(0, 0)) << "write back test " << 0 << ", " << 0;
  EXPECT_EQ(0, result(0, 0)) << "write back test " << 0 << ", " << 0;

  eigenMap = Eigen::Matrix<double, 5, 2>::Zero();

  for (int i = 0; i < result.rows(); ++i) {
    for (int j = 0; j < result.cols(); ++j) {
      EXPECT_EQ(0, result(i, j)) << "write back test " << i << ", " << j;
    }
  }
}

TEST(TrackFindingCDCTest, PlainMatrixEigenMap_output_operator)
{
  auto a = PlainMatrix<double, 2, 1>({1, 2});
  std::stringstream oss;
  oss << a;
}
