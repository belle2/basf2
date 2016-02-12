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
#include <analysis/VariableManager/Utility.h>

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

  TEST(TeacherTest, TeacherTrainsCorrectly)
  {
    TestHelpers::TempDirCreator tempdir;

    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();

    std::string target = "extraInfo(target)";
    std::string weight = "extraInfo(weight)";
    std::vector<std::string> variables = {"p", "extraInfo(someInput)"};
    std::vector<std::string> spectators = {"M", weight, target};

    std::vector<TMVAInterface::Method> methods;
    methods.push_back(TMVAInterface::Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs"));

    TeacherConfig config("unittest", "unittest_tree", ".", variables, spectators, methods);
    Teacher teacher(config);

    std::string factoryOption = "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "!V:SplitMode=block:NormMode=None";

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
      float weight = i * 3;
      p.addExtraInfo("someInput", someInput);
      p.addExtraInfo("target", target);
      p.addExtraInfo("weight", weight);
      teacher.addSample(&p);
      if (i < n / 2) {
        given_samples.insert({momentum, someInput, target, weight});
      }
    }

    teacher.trainClassification(factoryOption, prepareOption, Variable::makeROOTCompatible(target),
                                Variable::makeROOTCompatible(weight));
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    EXPECT_EQ(n / 2, inspector.GetTrainEvents().size());
    std::set<std::vector<float>> received_samples;
    unsigned int i = 0;
    for (auto event : inspector.GetTrainEvents()) {
      event.push_back(inspector.GetTrainWeight(i++));
      received_samples.insert(event);
    }

    EXPECT_EQ(given_samples, received_samples);
  }

  TEST(TeacherTest, TestFastBDTLoading)
  {
    // Test if the FastBDT TMVA Plugin can be properly loaded
    // A fail in this test might indicate a problem with header
    // distribution in via the externals

    TestHelpers::TempDirCreator tempdir;

    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();

    std::string target = "extraInfo(target)";
    std::string weight = "extraInfo(weight)";
    std::vector<std::string> variables = {"p", "extraInfo(someInput)"};
    std::vector<std::string> spectators = {"M", weight, target};

    std::vector<TMVAInterface::Method> methods;
    methods.push_back(TMVAInterface::Method("FastBDT", "Plugin", "!H:!V:CreateMVAPdfs"));

    TeacherConfig config("unittest_plugin_load", "unittest_tree_plugin_load", ".", variables, spectators, methods);
    Teacher teacher(config);

    std::string factoryOption = "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "!V:SplitMode=block:NormMode=None";

    gRandom->SetSeed(23);

    //add sample
    unsigned int n = 1000;
    std::set<std::vector<float>> given_samples;
    for (unsigned int i = 0; i < n; i++) {
      float target = (i % 2 == 0) * 1.0;
      TLorentzVector v;
      v.SetPtEtaPhiM((i % 900) * 0.1, 0.1, i * 0.1, 0.139);
      Particle p(v, 211);
      float momentum = v.P();
      float weight = 1.0f;
      p.addExtraInfo("target", target);
      p.addExtraInfo("weight", weight);
      teacher.addSample(&p);
      if (i < n / 2) {
        given_samples.insert({momentum, target, weight});
      }
    }

    EXPECT_NO_B2ERROR(teacher.trainClassification(factoryOption, prepareOption, Variable::makeROOTCompatible(target),
                                                  Variable::makeROOTCompatible(weight)));
  }

  TEST(TMVAInterfaceTest, TeacherDropsConstantVariable)
  {
    TestHelpers::TempDirCreator tempdir;
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();

    std::string target = "extraInfo(target)";
    std::string weight = "constant(1.1)";
    std::vector<std::string> variables = {"p", "False", "extraInfo(someInput)"};
    std::vector<std::string> spectators = {"M", weight, target};

    std::vector<TMVAInterface::Method> methods;
    methods.push_back(TMVAInterface::Method("MockPlugin", "Plugin", "!H:!V:CreateMVAPdfs"));

    TeacherConfig config("unittest2", "unitest_tree", ".", variables, spectators, methods);
    Teacher teacher(config);

    std::string factoryOption = "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification";
    std::string prepareOption = "!V:SplitMode=block:NormMode=None";

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

    teacher.trainClassification(factoryOption, prepareOption, Variable::makeROOTCompatible(target),
                                Variable::makeROOTCompatible(weight));
    MockPluginInspector& inspector = MockPluginInspector::GetInstance();

    EXPECT_EQ(n / 2, inspector.GetTrainEvents().size());
    std::set<std::vector<float>> received_samples;
    for (auto event : inspector.GetTrainEvents()) {
      received_samples.insert(event);
    }

    for (auto weight : inspector.GetTrainWeights()) {
      EXPECT_FLOAT_EQ(weight, 1.1);
    }

    EXPECT_EQ(given_samples, received_samples);
  }
}
