/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Method.h>
#include <analysis/TMVAInterface/Expert.h>
#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>
#include <fstream>

namespace Belle2 {

  namespace TMVAInterface {

    TEST(TMVAInterface, BuiltinMethodIsConstructedCorrectly)
    {
      auto method = Method("BoostedDecisionTrees", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"}));
      EXPECT_EQ(method.getName(), "BoostedDecisionTrees");
      EXPECT_EQ(method.getType(), TMVA::Types::kBDT);
      EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs:NTrees=100");
      EXPECT_EQ(method.getVariables().size(), 3u);
      EXPECT_EQ(method.getVariables()[0]->name, "p");
      EXPECT_EQ(method.getVariables()[1]->name, "pt");
      EXPECT_EQ(method.getVariables()[2]->name, "eid");
    }

    TEST(TMVAInterface, BuiltinMethodIsLoadedCorrectly)
    {
      std::ifstream stream(FileSystem::findFile("/analysis/tests/weights/builtin_correct.weights.xml"));
      auto method = Method(stream);
      EXPECT_EQ(method.getName(), "BoostedDecisionTrees");
      EXPECT_EQ(method.getType(), TMVA::Types::kBDT);
      EXPECT_EQ(method.getConfig(), std::string());
      EXPECT_EQ(method.getVariables().size(), 3u);
      EXPECT_EQ(method.getVariables()[0]->name, "p");
      EXPECT_EQ(method.getVariables()[1]->name, "pt");
      EXPECT_EQ(method.getVariables()[2]->name, "eid");
    }

    TEST(TMVAInterface, BuiltinMethodFailsCorrectly)
    {
      std::ifstream stream_method(FileSystem::findFile("/analysis/tests/weights/builtin_incorrect_method.weights.xml"));
      std::ifstream stream_variable(FileSystem::findFile("/analysis/tests/weights/builtin_incorrect_variable.weights.xml"));
      EXPECT_DEATH(Method {stream_method}, ".*");
      EXPECT_B2ERROR(Method {stream_variable});
      EXPECT_B2ERROR(Method("BoostedDecisionTree", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
      EXPECT_DEATH(Method("BoostedDecisionTree", "DOES_NOT_EXIST", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"})), ".*");
    }

    TEST(TMVAInterface, PluginMethodIsConstructedCorrectly)
    {
      auto method = Method("MyOwnBDT", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"}));
      EXPECT_EQ(method.getName(), "MyOwnBDT");
      EXPECT_EQ(method.getType(), TMVA::Types::kPlugins);
      EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs:NTrees=100");
      EXPECT_EQ(method.getVariables().size(), 3u);
      EXPECT_EQ(method.getVariables()[0]->name, "p");
      EXPECT_EQ(method.getVariables()[1]->name, "pt");
      EXPECT_EQ(method.getVariables()[2]->name, "eid");
    }


    TEST(TMVAInterface, PluginMethodIsLoadedCorrectly)
    {
      std::ifstream stream(FileSystem::findFile("/analysis/tests/weights/plugin_correct.weights.xml"));
      auto method = Method(stream);
      EXPECT_EQ(method.getName(), "OwnBDT");
      EXPECT_EQ(method.getType(), TMVA::Types::kPlugins);
      EXPECT_EQ(method.getConfig(), std::string());
      EXPECT_EQ(method.getVariables().size(), 3u);
      EXPECT_EQ(method.getVariables()[0]->name, "p");
      EXPECT_EQ(method.getVariables()[1]->name, "pt");
      EXPECT_EQ(method.getVariables()[2]->name, "eid");
    }

    TEST(TMVAInterface, PluginMethodFailsCorrectly)
    {
      std::ifstream stream_method(FileSystem::findFile("/analysis/tests/weights/plugin_incorrect_method.weights.xml"));
      std::ifstream stream_variable(FileSystem::findFile("/analysis/tests/weights/plugin_incorrect_variable.weights.xml"));
      EXPECT_B2ERROR(Method {stream_variable});
      EXPECT_B2ERROR(Method("MyOwnBDT", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
      // TODO Currently TPluginManager doesn't fail here if Method can't be loaed
      // EXPECT_DEATH(Method {stream_method}, ".*");
      // EXPECT_DEATH(Method("DOES_NOT_EXIST", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"})),".*");
    }

    TEST(TMVAInterface, ExpertRunAnalysisCorrectly)
    {

      //Expert expert(FileSystem::findFile("/analysis/tests/weights/electron.weights.xml"));
      //Particle particle({ 1.1 , 1.0, 0.0, 0.0 }, 11);
      //EXPECT_DOUBLE_EQ(expert.analyse(&particle), 1.0537381456288131e-07);

    }

    TEST(TMVAInterface, TeacherTrainsCorrectly)
    {

      // TODO Hwot test the teacher?

    }

  }
}
