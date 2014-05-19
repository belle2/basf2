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
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>

#include <gtest/gtest.h>

#include <TRandom3.h>

#include <fstream>

using namespace Belle2;
using namespace Belle2::TMVAInterface;

namespace {
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


  TEST(TMVAInterface, BuiltinMethodFailsCorrectly)
  {
    EXPECT_B2ERROR(Method("BoostedDecisionTree", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
    EXPECT_DEATH(Method("BoostedDecisionTree", "DOES_NOT_EXIST", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"})), ".*");
  }

  TEST(TMVAInterface, PluginMethodIsConstructedCorrectly)
  {
    auto method = Method("FastBDT", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"}));
    EXPECT_EQ(method.getName(), "FastBDT");
    EXPECT_EQ(method.getType(), TMVA::Types::kPlugins);
    EXPECT_EQ(method.getTypeAsString(), "Plugin");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs:NTrees=100");
    EXPECT_EQ(method.getVariables().size(), 3u);
    EXPECT_EQ(method.getVariables()[0]->name, "p");
    EXPECT_EQ(method.getVariables()[1]->name, "pt");
    EXPECT_EQ(method.getVariables()[2]->name, "eid");
  }


  TEST(TMVAInterface, PluginMethodFailsCorrectly)
  {
    EXPECT_B2ERROR(Method("MyOwnBDT", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
    // TODO Currently TPluginManager doesn't fail here if Method can't be loaed
    // EXPECT_DEATH(Method {stream_method}, ".*");
    // EXPECT_DEATH(Method("DOES_NOT_EXIST", "Plugin", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"})),".*");
  }

  TEST(TMVAInterface, ExpertErrorHandling)
  {

    EXPECT_B2FATAL(Expert expert("electron", FileSystem::findFile("/analysis/tests/"), "THISISNOTAVALIDMETHOD", 1));
    EXPECT_B2FATAL(Expert expert("electron", FileSystem::findFile("/analysis/tests/"), "BDTGradient", 123));
    EXPECT_B2FATAL(Expert expert("NONEXISTINGPREFIX", FileSystem::findFile("/analysis/tests/"), "BDTGradient", 1));

  }
  TEST(TMVAInterface, ExpertRunAnalysisCorrectly)
  {
    //network only expects 'eid' and 'p' as input
    //eid isn't set (-> 0.5), we'll just change p

    Expert expert("electron", FileSystem::findFile("/analysis/tests/"), "BDTGradient", 1);
    Particle p({ 1.1 , 1.0, 0.0, 0.0 }, 11);
    EXPECT_DOUBLE_EQ(0.11980155110359192, expert.analyse(&p));
    Particle q({ 0.0 , 0.2, 0.0, 0.0 }, 11);
    EXPECT_DOUBLE_EQ(0.048413272947072983, expert.analyse(&q));

  }

  TEST(TMVAInterface, TeacherTrainsCorrectly)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
    DataStore::Instance().setInitializeActive(false);

    std::vector<std::string> variables = {"p", "getExtraInfo(someInput)"};
    std::string target = "getExtraInfo(target)";

    std::vector<TMVAInterface::Method> methods;
    //methods.push_back(TMVAInterface::Method("NeuroBayes", "Plugin", "!H:CreateMVAPdfs:V:NTrainingIter=50:TrainingMethod=BFGS", variables));
    //methods.push_back(TMVAInterface::Method("FastBDT", "Plugin", "!H:CreateMVAPdfs:V", variables));
    methods.push_back(TMVAInterface::Method("BDTGradient", "BDT", "!H:CreateMVAPdfs:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=10:MaxDepth=2", variables));
    TMVAInterface::Teacher teacher("unittest", ".", target, methods);

    std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "SplitMode=random:!V";

    gRandom->SetSeed(23);

    //add sample
    int n = 20000;
    for (int i = 0; i < n; i++) {
      double target = (i < (n / 2) or (i % 13 == 0)) * 1.0;
      TLorentzVector v;
      v.SetPtEtaPhiM((i % 900) * 0.1, 0.1, i * 0.1, 0.139);
      Particle p(v, 211);
      p.addExtraInfo("someInput", gRandom->Gaus() + target * 0.5);

      p.addExtraInfo("target", target);
      teacher.addSample(&p);
    }

    teacher.train(factoryOption, prepareOption);

    Expert expert("unittest", ".", methods[0].getName(), 1);
    {
      Particle p({ 1.1 , 1.0, 0.0, 0.0 }, 211);
      p.addExtraInfo("someInput", -3.0);
      EXPECT_TRUE(expert.analyse(&p) < 0.4);
    }
    {
      //different p, should be similar to last one
      Particle p({ 2.1 , 1.0, 0.0, 0.0 }, 211);
      p.addExtraInfo("someInput", -3.0);
      EXPECT_TRUE(expert.analyse(&p) < 0.4);
    }
    {
      Particle p({ 1.1 , 1.0, 0.0, 0.0 }, 211);
      p.addExtraInfo("someInput", 3.0);
      EXPECT_TRUE(expert.analyse(&p) > 0.75);
    }
    {
      Particle p({ 2.1 , 1.0, 0.0, 0.0 }, 211);
      p.addExtraInfo("someInput", 3.0);
      EXPECT_TRUE(expert.analyse(&p) > 0.75);
    }
  }
}
