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
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2::MVA;

namespace {
  Interface<ONNXOptions, ONNXTeacher, ONNXExpert> interface;

  /**
   * Test files ONNX.xml and ONNX_multiclass.xml have been created via
   * mva/methods/tests/test_write_onnx.py
   */
  TEST(ONNXTest, ONNXExpert)
  {
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

  TEST(ONNXTest, ONNXExpertTwoClassUseSingleOutput)
  {
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
    // Running apply (not applyMulticlass) on this is supposed to assume a binary classifier and pick output index 1
    auto probabilities = expert->apply(dataset);
    EXPECT_NEAR(probabilities[0], -0.5373, 0.0001);
    EXPECT_NEAR(probabilities[1], -0.3697, 0.0001);
  }

  TEST(ONNXTest, ONNXExpertMulticlassThreeClasses)
  {
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

  Weightfile getONNXWeightfile(std::string modelFilenameONNX, std::string outputName = "")
  {
    Weightfile weightfile;
    GeneralOptions general_options;
    ONNXOptions specific_options;
    if (!outputName.empty()) {
      specific_options.m_outputName = outputName;
    }
    general_options.m_method = specific_options.getMethod();
    weightfile.addOptions(general_options);
    weightfile.addOptions(specific_options);
    weightfile.addFile("ONNX_Modelfile", modelFilenameONNX);
    return weightfile;
  }

  TEST(ONNXTest, ONNXFatalMultipleInputs)
  {
    // The following modelfile has multiple inputs
    // should fail when using with the ONNX MVA method
    // (onnx file created with mva/examples/onnx/write_test_files.py)
    auto weightfile = getONNXWeightfile(Belle2::FileSystem::findFile("mva/methods/tests/ModelABToAB.onnx"));
    auto expert = interface.getExpert();
    EXPECT_B2FATAL(expert->load(weightfile));
  }

  TEST(ONNXTest, ONNXFatalMultipleOutputs)
  {
    // The following modelfile has multiple outputs ("output_a", "output_twice_a"), so none of them named "output"
    // should fail when using with the ONNX MVA method
    // (onnx file created with mva/examples/onnx/write_test_files.py)
    auto weightfile = getONNXWeightfile(Belle2::FileSystem::findFile("mva/methods/tests/ModelAToATwiceA.onnx"));
    auto expert = interface.getExpert();
    EXPECT_B2FATAL(expert->load(weightfile));
  }

  TEST(ONNXTest, ONNXMultipleOutputsOK)
  {
    // explicitly choosing to use "output_twice_a" should work
    // (onnx file created with mva/examples/onnx/write_test_files.py)
    auto weightfile = getONNXWeightfile(Belle2::FileSystem::findFile("mva/methods/tests/ModelAToATwiceA.onnx"), "output_twice_a");
    auto expert = interface.getExpert();
    expert->load(weightfile);
  }
}
