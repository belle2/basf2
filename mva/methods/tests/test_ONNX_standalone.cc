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
  TEST(ONNXStandaloneTest, RunStandaloneModel)
  {
    // Testfile created with mva/examples/onnx/write_test_files.py
    Session session(Belle2::FileSystem::findFile("mva/methods/tests/ModelForStandalone.onnx"));
    auto input_a = Tensor<float>::make_shared({0.5309f, 0.4930f}, {1, 2});
    auto input_b = Tensor<int64_t>::make_shared({1, 0, 1, 1, -1, 0}, {1, 2, 3});
    auto output = Tensor<float>::make_shared({1, 2});
    session.run({{"a", input_a}, {"b", input_b}}, {{"output", output}});
    EXPECT_NEAR(output->at(0), -0.0614375323, 0.000000001);
    EXPECT_NEAR(output->at(1), 0.3322576284, 0.000000001);
  }
}
