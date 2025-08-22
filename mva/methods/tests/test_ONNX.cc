/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/interface/Interface.h>
#include <mva/methods/ONNX.h>
#include <framework/utilities/FileSystem.h>

#include <gtest/gtest.h>

using namespace Belle2::MVA;

namespace {
  /**
   * Test files ONNX.xml and ONNX_multiclass.xml have been created via
   * mva/methods/tests/test_write_onnx.py
   */
  TEST(ONNXTest, ONNXExpert)
  {
    Interface<ONNXOptions, ONNXTeacher, ONNXExpert> interface;
    auto expert = interface.getExpert();
    auto weightfile = Weightfile::loadFromFile(Belle2::FileSystem::findFile("mva/methods/tests/ONNX.xml"));
    expert->load(weightfile);
    GeneralOptions general_options;
    weightfile.getOptions(general_options);
    MultiDataset dataset(general_options, {
      {
        0.338, 0.079, 0.16, 0.048, 0.877, 0.367, 0.5, 0.436,
        0.33, 0.76, 0.176, 0.899, 0.062, 0.794, 0.477, 0.725
      },
      {
        0.438, 0.222, 0.959, 0.551, 0.987, 0.509, 0.141, 0.005, 0.387,
        0.926, 0.099, 0.990, 0.870, 0.050, 0.924, 0.767
      }
    },
    {}, {0.0, 1.0});
    auto probabilities = expert->apply(dataset);
    EXPECT_NEAR(probabilities[0], 0.3328, 0.0001);
    EXPECT_NEAR(probabilities[1], 0.5779, 0.0001);
  }

  TEST(ONNXTest, ONNXExpertMulticlass)
  {
    Interface<ONNXOptions, ONNXTeacher, ONNXExpert> interface;
    auto expert = interface.getExpert();
    auto weightfile = Weightfile::loadFromFile(Belle2::FileSystem::findFile("mva/methods/tests/ONNX_multiclass.xml"));
    expert->load(weightfile);
    GeneralOptions general_options;
    weightfile.getOptions(general_options);
    MultiDataset dataset(general_options, {
      {
        0.338, 0.079, 0.16, 0.048, 0.877, 0.367, 0.5, 0.436,
        0.33, 0.76, 0.176, 0.899, 0.062, 0.794, 0.477, 0.725
      },
      {
        0.438, 0.222, 0.959, 0.551, 0.987, 0.509, 0.141, 0.005, 0.387,
        0.926, 0.099, 0.990, 0.870, 0.050, 0.924, 0.767
      }
    },
    {}, {0.0, 1.0});
    auto probabilities = expert->applyMulticlass(dataset);
    EXPECT_NEAR(probabilities[0][0], 0.3331, 0.0001);
    EXPECT_NEAR(probabilities[0][1], -0.5373, 0.0001);
    EXPECT_NEAR(probabilities[1][0], 0.5782, 0.0001);
    EXPECT_NEAR(probabilities[1][1], -0.3697, 0.0001);
  }

  TEST(ONNXTest, ONNXExpertMulticlassThreeClasses)
  {
    Interface<ONNXOptions, ONNXTeacher, ONNXExpert> interface;
    auto expert = interface.getExpert();
    auto weightfile = Weightfile::loadFromFile(Belle2::FileSystem::findFile("mva/methods/tests/ONNX_multiclass_3.xml"));
    expert->load(weightfile);
    GeneralOptions general_options;
    weightfile.getOptions(general_options);
    MultiDataset dataset(general_options, {
      {
        0.338, 0.079, 0.16, 0.048, 0.877, 0.367, 0.5, 0.436,
        0.33, 0.76, 0.176, 0.899, 0.062, 0.794, 0.477, 0.725
      },
      {
        0.438, 0.222, 0.959, 0.551, 0.987, 0.509, 0.141, 0.005, 0.387,
        0.926, 0.099, 0.990, 0.870, 0.050, 0.924, 0.767
      }
    },
    {}, {0.0, 1.0});
    auto probabilities = expert->applyMulticlass(dataset);
    EXPECT_NEAR(probabilities[0][0], -0.5394, 0.0001);
    EXPECT_NEAR(probabilities[0][1], 0.0529, 0.0001);
    EXPECT_NEAR(probabilities[0][2], -0.0598, 0.0001);
    EXPECT_NEAR(probabilities[1][0], -0.5089, 0.0001);
    EXPECT_NEAR(probabilities[1][1], 0.0060, 0.0001);
    EXPECT_NEAR(probabilities[1][2], 0.0132, 0.0001);
  }
}
