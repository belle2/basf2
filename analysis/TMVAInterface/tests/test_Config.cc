/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Config.h>
#include <analysis/TMVAInterface/Method.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace Belle2::TMVAInterface;

namespace {

  TEST(ConfigTest, ConfigGettersAreCorrect)
  {
    Config config("prefix", "workingDirectory/");

    EXPECT_EQ(config.getPrefix(), "prefix");
    EXPECT_EQ(config.getWorkingDirectory(), "workingDirectory/");
    EXPECT_EQ(config.getFileName(), "prefix.root");
    EXPECT_EQ(config.getConfigFileName(1), "prefix_1.config");
    EXPECT_EQ(config.getTreeName(), "prefix_tree");
    EXPECT_EQ(config.getVariables().size(), 0);
    EXPECT_EQ(config.getSpectators().size(), 0);

  }

  TEST(ConfigTest, ConfigAddMissingSlashInWorkingDirectory)
  {
    Config config("prefix", "workingDirectory");
    EXPECT_EQ(config.getWorkingDirectory(), "workingDirectory/");
  }

  TEST(ConfigTest, TeacherConfigGettersAreCorrect)
  {
    std::vector<Method> methods{Method("BoostedDecisionTrees", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100")};
    TeacherConfig config("prefix", "workingDirectory/", {"p", "pt"}, {"M"}, methods);

    EXPECT_EQ(config.getPrefix(), "prefix");
    EXPECT_EQ(config.getWorkingDirectory(), "workingDirectory/");
    EXPECT_EQ(config.getFileName(), "prefix.root");
    EXPECT_EQ(config.getConfigFileName(1), "prefix_1.config");
    EXPECT_EQ(config.getTreeName(), "prefix_tree");

    auto variables = config.getVariablesFromManager();
    EXPECT_EQ(variables.size(), 2);
    EXPECT_EQ(variables[0]->name, "p");
    EXPECT_EQ(variables[1]->name, "pt");

    auto spectators = config.getSpectatorsFromManager();
    EXPECT_EQ(spectators.size(), 1);
    EXPECT_EQ(spectators[0]->name, "M");

    EXPECT_EQ(config.getMethods()[0].getName(), methods[0].getName());

  }

  TEST(ConfigTest, ExpertConfigFailsCorrectly)
  {

    EXPECT_B2FATAL(ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "THIS_IS_NOT_A_VALID_METHOD",
                                       1));
    EXPECT_B2FATAL(ExpertConfig config("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 2));
    EXPECT_B2FATAL(ExpertConfig config("PREFIX_DOES_NOT_EXISTS", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin",
                                       1));

  }

  TEST(ConfigTest, ExpertConfigReadIsCorrect)
  {

    ExpertConfig read("TMVA", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);

    EXPECT_EQ(read.getPrefix(), "TMVA");
    EXPECT_EQ(read.getWorkingDirectory(), FileSystem::findFile("/analysis/TMVAInterface/tests/"));
    EXPECT_EQ(read.getFileName(), "TMVA.root");
    EXPECT_EQ(read.getConfigFileName(1), "TMVA_1.config");
    EXPECT_EQ(read.getSignalClass(), 1);
    EXPECT_FLOAT_EQ(read.getSignalFraction(), 0.2f);
    EXPECT_EQ(read.getTreeName(), "TMVA_tree");
    EXPECT_EQ(read.getMethod(), "MockPlugin");
    EXPECT_EQ(read.getWeightfile(), "weights/TMVA_1_MockPlugin.weights.xml");

    auto variables = read.getVariablesFromManager();
    EXPECT_EQ(variables.size(), 2);
    EXPECT_EQ(variables[0]->name, "eid");
    EXPECT_EQ(variables[1]->name, "p");

    auto spectators = read.getSpectatorsFromManager();
    EXPECT_EQ(spectators.size(), 2);
    EXPECT_EQ(spectators[0]->name, "muid");
    EXPECT_EQ(spectators[1]->name, "M");

  }

  TEST(ConfigTest, ExpertConfigLegacyReadIsCorrect)
  {

    ExpertConfig read("Legacy", FileSystem::findFile("/analysis/TMVAInterface/tests/"), "MockPlugin", 1);

    EXPECT_EQ(read.getPrefix(), "Legacy");
    EXPECT_EQ(read.getWorkingDirectory(), FileSystem::findFile("/analysis/TMVAInterface/tests/"));
    EXPECT_EQ(read.getFileName(), "Legacy.root");
    // Config file is actually named Legacy.config, this is handled correctly internally
    EXPECT_EQ(read.getConfigFileName(1), "Legacy_1.config");
    EXPECT_EQ(read.getSignalClass(), 1);
    EXPECT_FLOAT_EQ(read.getSignalFraction(), 0.2f);
    EXPECT_EQ(read.getTreeName(), "Legacy_tree");
    EXPECT_EQ(read.getMethod(), "MockPlugin");
    EXPECT_EQ(read.getWeightfile(), "weights/Legacy_1_MockPlugin.weights.xml");

    auto variables = read.getVariablesFromManager();
    EXPECT_EQ(variables.size(), 2);
    EXPECT_EQ(variables[0]->name, "eid");
    EXPECT_EQ(variables[1]->name, "p");

    auto spectators = read.getSpectatorsFromManager();
    EXPECT_EQ(spectators.size(), 0);

  }


  TEST(ConfigTest, ExpertConfigReadXMLAndTeacherConfigWriteXML)
  {
    TestHelpers::TempDirCreator tempDirectory;

    std::vector<Method> methods{Method("BoostedDecisionTrees", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100"),
                                Method("BoostedDecisionTrees2", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100")};

    TeacherConfig write("prefix", tempDirectory.getTempDir(), {"p", "pt"}, {"M"}, methods);
    write.save(2, 0.6f);

    ExpertConfig read("prefix", tempDirectory.getTempDir(), "BoostedDecisionTrees", 2);

    EXPECT_EQ(read.getPrefix(), "prefix");
    EXPECT_EQ(read.getWorkingDirectory(), tempDirectory.getTempDir() + "/");
    EXPECT_EQ(read.getFileName(), "prefix.root");
    EXPECT_EQ(read.getConfigFileName(2), "prefix_2.config");
    EXPECT_EQ(read.getSignalClass(), 2);
    EXPECT_FLOAT_EQ(read.getSignalFraction(), 0.6f);
    EXPECT_EQ(read.getTreeName(), "prefix_tree");
    EXPECT_EQ(read.getMethod(), "BoostedDecisionTrees");
    EXPECT_EQ(read.getWeightfile(), "weights/prefix_2_BoostedDecisionTrees.weights.xml");

    ExpertConfig read2("prefix", tempDirectory.getTempDir(), "BoostedDecisionTrees2", 2);

    EXPECT_EQ(read2.getPrefix(), "prefix");
    EXPECT_EQ(read2.getWorkingDirectory(), tempDirectory.getTempDir() + "/");
    EXPECT_EQ(read2.getFileName(), "prefix.root");
    EXPECT_EQ(read2.getConfigFileName(2), "prefix_2.config");
    EXPECT_EQ(read2.getSignalClass(), 2);
    EXPECT_FLOAT_EQ(read2.getSignalFraction(), 0.6f);
    EXPECT_EQ(read2.getTreeName(), "prefix_tree");
    EXPECT_EQ(read2.getMethod(), "BoostedDecisionTrees2");
    EXPECT_EQ(read2.getWeightfile(), "weights/prefix_2_BoostedDecisionTrees2.weights.xml");

    auto variables = read.getVariablesFromManager();
    EXPECT_EQ(variables.size(), 2);
    EXPECT_EQ(variables[0]->name, "p");
    EXPECT_EQ(variables[1]->name, "pt");

    auto spectators = read.getSpectatorsFromManager();
    EXPECT_EQ(spectators.size(), 1);
    EXPECT_EQ(spectators[0]->name, "M");

  }

}
