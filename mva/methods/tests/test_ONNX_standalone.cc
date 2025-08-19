/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/ONNX.h>
#include <framework/utilities/FileSystem.h>

#include <gtest/gtest.h>

using namespace Belle2::MVA::ONNX;

namespace {
  TEST(ONNXStandaloneTest, TensorIndexing)
  {
    auto t = Tensor<int>::make_shared({1, 2, 3, 4, 5, 6}, {2, 3});
    EXPECT_EQ(t->at({0, 2}), 3);
    EXPECT_EQ(t->at({1, 2}), 6);
    t->at({1, 1}) = 42;
    EXPECT_EQ(t->at(4), 42);
    EXPECT_EQ(t->at({1, 1}), 42);
  }
  TEST(ONNXStandaloneTest, BoundsCheck)
  {
    auto t = Tensor<int>::make_shared({1, 3});
    EXPECT_THROW(t->at({2, 0}), std::out_of_range);
  }
  TEST(ONNXStandaloneTest, SizeCheck)
  {
    EXPECT_THROW(Tensor<int>({1, 2, 3}, {2, 2}), std::length_error);
  }
}
