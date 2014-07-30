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
#include <analysis/TMVAMockPlugin/MethodMockPlugin.h>
#include <analysis/TMVAMockPluginInspector/MockPluginInspector.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>

#include <gtest/gtest.h>

#include <TRandom3.h>

#include <fstream>
#include <vector>

using namespace Belle2;
using namespace Belle2::TMVAInterface;

namespace std {
  // Almost equal comparison for vector
  bool operator==(const std::vector<float>& lhs, const std::vector<float>& rhs)
  {
    assert(sizeof(float) == sizeof(int));
    if (lhs.size() != rhs.size())
      return false;

    for (unsigned int i = 0; i < lhs.size(); ++i)
      if (abs(*(int*)&lhs[i] - * (int*)&rhs[i]) > 1)
        return false;

    return true;
  }
}


namespace {

  TEST(TMVAInterfaceTest, BuiltinMethodIsConstructedCorrectly)
  {
    auto method = Method("BoostedDecisionTrees", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"}));
    EXPECT_EQ(method.getName(), "BoostedDecisionTrees");
    EXPECT_EQ(method.getType(), TMVA::Types::kBDT);
    EXPECT_EQ(method.getTypeAsString(), "BDT");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs:NTrees=100");
    EXPECT_EQ(method.getVariables().size(), 3u);
    EXPECT_EQ(method.getVariables()[0]->name, "p");
    EXPECT_EQ(method.getVariables()[1]->name, "pt");
    EXPECT_EQ(method.getVariables()[2]->name, "eid");
  }


  TEST(TMVAInterfaceTest, BuiltinMethodFailsCorrectly)
  {
    EXPECT_B2ERROR(Method("BoostedDecisionTree", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
    EXPECT_DEATH(Method("BoostedDecisionTree", "DOES_NOT_EXIST", "!H:!V:CreateMVAPdfs:NTrees=100", std::vector<std::string>({"p", "pt", "eid"})), ".*");
  }

  TEST(TMVAInterfaceTest, PluginMethodIsConstructedCorrectly)
  {
    auto method = Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs", std::vector<std::string>({"p", "pt", "eid"}));
    EXPECT_EQ(method.getName(), "MockPlugin");
    EXPECT_EQ(method.getType(), TMVA::Types::kPlugins);
    EXPECT_EQ(method.getTypeAsString(), "Plugin");
    EXPECT_EQ(method.getConfig(), "!H:!V:CreateMVAPdfs");
    EXPECT_EQ(method.getVariables().size(), 3u);
    EXPECT_EQ(method.getVariables()[0]->name, "p");
    EXPECT_EQ(method.getVariables()[1]->name, "pt");
    EXPECT_EQ(method.getVariables()[2]->name, "eid");
  }


  TEST(TMVAInterfaceTest, PluginMethodFailsCorrectly)
  {
    EXPECT_B2ERROR(Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs", std::vector<std::string>({"p", "DOES_NOT_EXIST", "eid"})));
  }

  TEST(TMVAInterfaceTest, ExpertErrorHandling)
  {

    EXPECT_B2FATAL(Expert expert("TMVA", FileSystem::findFile("/analysis/tests/"), "THIS_IS_NOT_A_VALID_METHOD", 1));
    EXPECT_B2FATAL(Expert expert("TMVA", FileSystem::findFile("/analysis/tests/"), "MockPlugin", 123));
    EXPECT_B2FATAL(Expert expert("NON_EXISTING_PREFIX", FileSystem::findFile("/analysis/tests/"), "MockPlugin", 1));

  }
  TEST(TMVAInterfaceTest, ExpertOnSignalRunAnalysisCorrectly)
  {
    //network only expects 'eid' and 'p' as input
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    Expert expert("TMVA", FileSystem::findFile("/analysis/tests/"), "MockPlugin", 1);

    // signalToBackgroundRatio default == -1 -> No Transformation
    inspector.SetMvaValue(0.9);
    EXPECT_FLOAT_EQ(0.9, expert.analyse(&p));
    EXPECT_FLOAT_EQ(0.9, expert.analyse(&p, -1));

    // Test probability transformation of mva output. Training was done with constant mva output == 0
    // therefore we expect probability == S/(S+B)== signalFraction
    // signalToBackgroundRatio == -2 -> Use training signalFraction which was 0.2
    // signalToBackgroundRatio >= 0 -> Use given signalFraction
    EXPECT_FLOAT_EQ(0.2, expert.analyse(&p, -2));
    EXPECT_FLOAT_EQ(0.0, expert.analyse(&p, 0.0));
    EXPECT_FLOAT_EQ(0.1, expert.analyse(&p, 0.1));
    EXPECT_FLOAT_EQ(0.3, expert.analyse(&p, 0.3));
    EXPECT_FLOAT_EQ(0.5, expert.analyse(&p, 0.5));
    EXPECT_FLOAT_EQ(0.7, expert.analyse(&p, 0.7));
    EXPECT_FLOAT_EQ(0.9, expert.analyse(&p, 0.9));
    EXPECT_FLOAT_EQ(1.0, expert.analyse(&p, 1.0));
    EXPECT_B2ERROR(expert.analyse(&p, 1.1));

    // Correct input values
    std::vector<float> test_event = inspector.GetTestEvent();
    EXPECT_FLOAT_EQ(0.5, test_event[0]); // eid
    EXPECT_FLOAT_EQ(1.0, test_event[1]); // p

  }

  TEST(TMVAInterfaceTest, ExpertOnBackgroundRunAnalysisCorrectly)
  {
    //network only expects 'eid' and 'p' as input
    //eid isn't set (-> 0.5)
    Particle p({ 0.0 , 1.0, 0.0, 1.1 }, 11);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    Expert expert("TMVA", FileSystem::findFile("/analysis/tests/"), "MockPlugin", 0);

    // signalToBackgroundRatio default == -1 -> No Transformation
    inspector.SetMvaValue(0.9);
    EXPECT_FLOAT_EQ(0.1, expert.analyse(&p));
    EXPECT_FLOAT_EQ(0.1, expert.analyse(&p, -1));

    // Test probability transformation of mva output. Training was done with constant mva output == 0
    // therefore we expect probability == S/(S+B)== signalFraction
    // signalToBackgroundRatio == -2 -> Use training signalFraction which was 0.8
    // signalToBackgroundRatio >= 0 -> Use given signalFraction
    EXPECT_FLOAT_EQ(0.8, expert.analyse(&p, -2));
    EXPECT_FLOAT_EQ(0.0, expert.analyse(&p, 0.0));
    EXPECT_FLOAT_EQ(0.1, expert.analyse(&p, 0.1));
    EXPECT_FLOAT_EQ(0.3, expert.analyse(&p, 0.3));
    EXPECT_FLOAT_EQ(0.5, expert.analyse(&p, 0.5));
    EXPECT_FLOAT_EQ(0.7, expert.analyse(&p, 0.7));
    EXPECT_FLOAT_EQ(0.9, expert.analyse(&p, 0.9));
    EXPECT_FLOAT_EQ(1.0, expert.analyse(&p, 1.0));
    EXPECT_B2ERROR(expert.analyse(&p, 1.1));

    // Correct input values
    std::vector<float> test_event = inspector.GetTestEvent();
    EXPECT_FLOAT_EQ(0.5, test_event[0]); // eid
    EXPECT_FLOAT_EQ(1.0, test_event[1]); // p

  }

  TEST(TMVAInterfaceTest, TeacherTrainsCorrectly)
  {
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();

    std::vector<std::string> variables = {"p", "getExtraInfo(someInput)"};
    std::string target = "getExtraInfo(target)";

    std::vector<TMVAInterface::Method> methods;
    methods.push_back(TMVAInterface::Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs", variables));

    TMVAInterface::Teacher teacher("unittest", ".", target, methods);

    std::string factoryOption = "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "!V:SplitMode=block";

    gRandom->SetSeed(23);

    //add sample
    unsigned int n = 100;
    std::set<std::vector<float>> given_samples;
    for (unsigned int i = 0; i < n; i++) {
      float target = (i % 2 == 0) * 1.0;
      TLorentzVector v;
      v.SetPtEtaPhiM((i % 900) * 0.1, 0.1, i * 0.1, 0.139);
      Particle p(v, 211);
      float momentum = v.P();
      float someInput = i;//gRandom->Gaus() + target * 1.0;
      p.addExtraInfo("someInput", someInput);
      p.addExtraInfo("target", target);
      teacher.addSample(&p);
      if (i < n / 2) {
        given_samples.insert({momentum, someInput, target});
      }
    }

    teacher.train(factoryOption, prepareOption);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    EXPECT_EQ(n / 2, inspector.GetTrainEvents().size());
    std::set<std::vector<float>> received_samples;
    for (auto event : inspector.GetTrainEvents()) {
      received_samples.insert(event);
    }

    EXPECT_EQ(given_samples, received_samples);

  }

  TEST(TMVAInterfaceTest, TeacherDropsConstantVariable)
  {
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();

    std::vector<std::string> variables = {"p", "False", "getExtraInfo(someInput)"};
    std::string target = "getExtraInfo(target)";

    std::vector<TMVAInterface::Method> methods;
    methods.push_back(TMVAInterface::Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs", variables));

    TMVAInterface::Teacher teacher("unittest2", ".", target, methods);

    std::string factoryOption = "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "!V:SplitMode=block";

    gRandom->SetSeed(23);

    //add sample
    unsigned int n = 100;
    std::set<std::vector<float>> given_samples;
    for (unsigned int i = 0; i < n; i++) {
      float target = (i % 2 == 0) * 1.0;
      TLorentzVector v;
      v.SetPtEtaPhiM((i % 900) * 0.1, 0.1, i * 0.1, 0.139);
      Particle p(v, 211);
      float momentum = v.P();
      float someInput = i;//gRandom->Gaus() + target * 1.0;
      p.addExtraInfo("someInput", someInput);
      p.addExtraInfo("target", target);
      teacher.addSample(&p);
      if (i < n / 2) {
        given_samples.insert({momentum, someInput, target});
      }
    }

    teacher.train(factoryOption, prepareOption);
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    EXPECT_EQ(n / 2, inspector.GetTrainEvents().size());
    std::set<std::vector<float>> received_samples;
    for (auto event : inspector.GetTrainEvents()) {
      received_samples.insert(event);
    }

    EXPECT_EQ(given_samples, received_samples);

  }
}
