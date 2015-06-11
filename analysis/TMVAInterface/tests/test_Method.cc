/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Method.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace Belle2::TMVAInterface;

namespace {

  TEST(MethodTest, BuiltinMethodIsConstructedCorrectly)
  {
    auto method = Method("BoostedDecisionTrees", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100");
    EXPECT_EQ(method.getName(), "BoostedDecisionTrees");
    EXPECT_EQ(method.getTypeAsEnum(), TMVA::Types::kBDT);
    EXPECT_EQ(method.getTypeAsString(), "BDT");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs:NTrees=100");
  }

  TEST(MethodTest, PluginMethodIsConstructedCorrectly)
  {
    auto method = Method("MockPlugin", "Plugins", "!H:!V:CreateMVAPdfs");
    EXPECT_EQ(method.getName(), "MockPlugin");
    EXPECT_EQ(method.getTypeAsEnum(), TMVA::Types::kPlugins);
    EXPECT_EQ(method.getTypeAsString(), "Plugins");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs");
  }

  TEST(MethodTest, PluginMethodIsConstructedCorrectlyEvenWithSuffix)
  {
    auto method = Method("MockPluginSuffix", "Plugins", "!H:!V:CreateMVAPdfs");
    EXPECT_EQ(method.getName(), "MockPluginSuffix");
    EXPECT_EQ(method.getTypeAsEnum(), TMVA::Types::kPlugins);
    EXPECT_EQ(method.getTypeAsString(), "Plugins");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs");
  }

  TEST(MethodTest, FastBDTIsAvailable)
  {
    auto method = Method("FastBDT", "Plugins", "!H:!V:CreateMVAPdfs");
    EXPECT_EQ(method.getName(), "FastBDT");
    EXPECT_EQ(method.getTypeAsEnum(), TMVA::Types::kPlugins);
    EXPECT_EQ(method.getTypeAsString(), "Plugins");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs");
  }

  TEST(MethodTest, BuiltinMethodFailsCorrectly)
  {
    EXPECT_DEATH(Method("BoostedDecisionTree", "DOES_NOT_EXIST", "!H:!V:CreateMVAPdfs:NTrees=100"), ".*");
  }

}
