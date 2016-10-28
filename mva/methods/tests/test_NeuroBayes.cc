/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <mva/methods/NeuroBayes.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(NeuroBayesTest, NeuroBayesOptions)
  {

    MVA::NeuroBayesOptions specific_options;

    EXPECT_EQ(specific_options.m_lossFunction, "ENTROPY");
    EXPECT_EQ(specific_options.m_regularization, "REG");
    EXPECT_EQ(specific_options.m_preprocessing, 112);
    EXPECT_EQ(specific_options.m_pruning, 0);
    EXPECT_EQ(specific_options.m_shapeTreat, "DIA");
    EXPECT_FLOAT_EQ(specific_options.m_momentum, 0.0);
    EXPECT_EQ(specific_options.m_weightUpdate, 200);
    EXPECT_FLOAT_EQ(specific_options.m_trainTestRatio, 0.5);
    EXPECT_EQ(specific_options.m_nIterations, 100);
    EXPECT_FLOAT_EQ(specific_options.m_learningSpeed, 1.0);
    EXPECT_FLOAT_EQ(specific_options.m_limitLearningSpeed, 1.0);
    EXPECT_EQ(specific_options.m_method, "BFGS");
    EXPECT_EQ(specific_options.m_weightfile, "NeuroBayes.nb");

    specific_options.m_lossFunction = "LOSS";
    specific_options.m_regularization = "REG";
    specific_options.m_preprocessing = 112;
    specific_options.m_pruning = 23;
    specific_options.m_shapeTreat = "SHAPE";
    specific_options.m_momentum = 42;
    specific_options.m_weightUpdate = 100;
    specific_options.m_trainTestRatio = 0.9;
    specific_options.m_nIterations = 7;
    specific_options.m_learningSpeed = 3.14;
    specific_options.m_limitLearningSpeed = 2.718;
    specific_options.m_method = "BFGS";
    specific_options.m_weightfile = "weightfile";

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_EQ(pt.get<std::string>("NeuroBayes_lossFunction"), "LOSS");
    EXPECT_EQ(pt.get<std::string>("NeuroBayes_regularization"), "REG");
    EXPECT_EQ(pt.get<int>("NeuroBayes_preprocessing"), 112);
    EXPECT_EQ(pt.get<int>("NeuroBayes_pruning"), 23);
    EXPECT_EQ(pt.get<std::string>("NeuroBayes_shapeTreat"), "SHAPE");
    EXPECT_FLOAT_EQ(pt.get<float>("NeuroBayes_momentum"), 42.0);
    EXPECT_FLOAT_EQ(pt.get<int>("NeuroBayes_weightUpdate"), 100);
    EXPECT_FLOAT_EQ(pt.get<float>("NeuroBayes_trainTestRatio"), 0.9);
    EXPECT_EQ(pt.get<int>("NeuroBayes_nIterations"), 7);
    EXPECT_FLOAT_EQ(pt.get<float>("NeuroBayes_learningSpeed"), 3.14);
    EXPECT_FLOAT_EQ(pt.get<float>("NeuroBayes_limitLearningSpeed"), 2.718);
    EXPECT_EQ(pt.get<std::string>("NeuroBayes_method"), "BFGS");
    EXPECT_EQ(pt.get<std::string>("NeuroBayes_weightfile"), "weightfile");

    MVA::NeuroBayesOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_lossFunction, "LOSS");
    EXPECT_EQ(specific_options2.m_regularization, "REG");
    EXPECT_EQ(specific_options2.m_preprocessing, 112);
    EXPECT_EQ(specific_options2.m_pruning, 23);
    EXPECT_EQ(specific_options2.m_shapeTreat, "SHAPE");
    EXPECT_FLOAT_EQ(specific_options2.m_momentum, 42.0);
    EXPECT_EQ(specific_options2.m_weightUpdate, 100);
    EXPECT_FLOAT_EQ(specific_options2.m_trainTestRatio, 0.9);
    EXPECT_EQ(specific_options2.m_nIterations, 7);
    EXPECT_FLOAT_EQ(specific_options2.m_learningSpeed, 3.14);
    EXPECT_FLOAT_EQ(specific_options2.m_limitLearningSpeed, 2.718);
    EXPECT_EQ(specific_options2.m_method, "BFGS");
    EXPECT_EQ(specific_options2.m_weightfile, "weightfile");

    EXPECT_EQ(specific_options.getMethod(), std::string("NeuroBayes"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 14);

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("NeuroBayes_version", 100);
    try {
      EXPECT_B2ERROR(specific_options2.load(pt));
    } catch (...) {

    }
    EXPECT_THROW(specific_options2.load(pt), std::runtime_error);
  }

  class TestDataset : public MVA::Dataset {
  public:
    explicit TestDataset(const std::vector<float>& data) : MVA::Dataset(MVA::GeneralOptions()), m_data(data)
    {
      m_input = {0.0};
      m_target = 0.0;
      m_isSignal = false;
      m_weight = 1.0;
    }

    virtual unsigned int getNumberOfFeatures() const override { return 1; }
    virtual unsigned int getNumberOfEvents() const override { return m_data.size(); }
    virtual void loadEvent(unsigned int iEvent) override { m_input[0] = m_data[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;

  };




  TEST(NeuroBayesTTest, NeuroBayesInterface)
  {
#ifndef DEACTIVATE_NEUROBAYES

    if (MVA::IsNeuroBayesAvailable()) {
      MVA::Interface<MVA::NeuroBayesOptions, MVA::NeuroBayesTeacher, MVA::NeuroBayesExpert> interface;

      MVA::GeneralOptions general_options;
      general_options.m_variables = {"A"};
      MVA::NeuroBayesOptions specific_options;
      std::vector<float> data;
      std::vector<float> template_data = {1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 2.0, 3.0};
      for (unsigned int i = 0; i < 1000; ++i) {
        data.push_back(template_data[i % template_data.size()]);
      }
      TestDataset dataset(data);

      auto teacher = interface.getTeacher(general_options, specific_options);
      auto weightfile = teacher->train(dataset);

      auto expert = interface.getExpert();
      expert->load(weightfile);
      auto probabilities = expert->apply(dataset);
      EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
      for (unsigned int i = 0; i < 4; ++i) {
        EXPECT_LE(probabilities[i], 0.2);
        EXPECT_GE(probabilities[i], -0.2);
      }
      EXPECT_LE(probabilities[4], -0.6);
      EXPECT_GE(probabilities[5], 0.6);
      EXPECT_LE(probabilities[6], -0.6);
      EXPECT_GE(probabilities[7], 0.6);
    }
#endif

  }



}
