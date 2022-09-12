/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/Trivial.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(TrivialTest, TrivialOptions)
  {

    MVA::TrivialOptions specific_options;

    EXPECT_EQ(specific_options.m_output, 0.5);

    specific_options.m_output = 0.1;
    specific_options.m_multiple_output = {1.0, 2.0, 3.0};
    specific_options.m_passthrough = true;

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_FLOAT_EQ(pt.get<double>("Trivial_output"), 0.1);
    EXPECT_FLOAT_EQ(pt.get<bool>("Trivial_passthrough"), true);

    EXPECT_FLOAT_EQ(pt.get<unsigned int>("Trivial_number_of_multiple_outputs"), 3);
    EXPECT_FLOAT_EQ(pt.get<double>("Trivial_multiple_output0"), 1.0);
    EXPECT_FLOAT_EQ(pt.get<double>("Trivial_multiple_output1"), 2.0);
    EXPECT_FLOAT_EQ(pt.get<double>("Trivial_multiple_output2"), 3.0);

    MVA::TrivialOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_output, 0.1);
    EXPECT_EQ(specific_options2.m_multiple_output.at(0), 1.0);
    EXPECT_EQ(specific_options2.m_multiple_output.at(1), 2.0);
    EXPECT_EQ(specific_options2.m_multiple_output.at(2), 3.0);

    EXPECT_EQ(specific_options2.getMethod(), std::string("Trivial"));
    EXPECT_FLOAT_EQ(specific_options2.m_passthrough, true);

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 3);

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("Trivial_version", 100);
    try {
      EXPECT_B2ERROR(specific_options2.load(pt));
    } catch (...) {

    }
    EXPECT_THROW(specific_options2.load(pt), std::runtime_error);
  }

  class TestDataset : public MVA::Dataset {
  public:
    explicit TestDataset(const std::vector<std::vector<float>>& data) : MVA::Dataset(MVA::GeneralOptions()), m_data(data),
      m_nFeatures(data[0].size())
    {
      m_input = {0.0};
      m_target = 0.0;
      m_isSignal = false;
      m_weight = 1.0;
    }

    [[nodiscard]] unsigned int getNumberOfFeatures() const override { return m_nFeatures;}
    [[nodiscard]] unsigned int getNumberOfSpectators() const override { return 0; }
    [[nodiscard]] unsigned int getNumberOfEvents() const override { return m_data.size(); }
    void loadEvent(unsigned int iEvent) override { m_input = m_data[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    float getSignalFraction() override { return 0.1; };
    std::vector<float> getFeature(unsigned int iFeature) override
    {
      std::vector<float> feature(m_data.size(), 0.0);
      for (unsigned int iEvent = 0; iEvent << m_data.size(); iEvent++) {
        feature[iEvent] = m_data[iEvent][iFeature];
      }
      return feature;
    }
    std::vector<std::vector<float>> m_data;
    unsigned int m_nFeatures;
  };

  TEST(TrivialTest, TrivialInterface)
  {
    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::TrivialOptions specific_options;
    TestDataset dataset({{1.0,}, {1.0,}, {1.0,}, {1.0,}, {2.0,}, {3.0,}, {2.0,}, {3.0,}});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i)
      EXPECT_FLOAT_EQ(probabilities[i], 0.5);
  }

  TEST(TrivialTest, TrivialPassThrough)
  {
    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::TrivialOptions specific_options;
    general_options.m_variables = {"p",};
    specific_options.m_passthrough = true;
    TestDataset dataset({{1.0,}, {1.0,}, {1.0,}, {1.0,}, {2.0,}, {3.0,}, {2.0,}, {3.0,}});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i)
      EXPECT_FLOAT_EQ(probabilities[i], dataset.m_data[i][0]);
  }

  TEST(TrivialTest, TrivialPassThroughMulticlass)
  {
    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::TrivialOptions specific_options;
    general_options.m_variables = {"p",};
    specific_options.m_passthrough = true;
    general_options.m_nClasses = 3;
    TestDataset dataset({{1.0,}, {1.0,}, {1.0,}, {1.0,}, {2.0,}, {3.0,}, {2.0,}, {3.0,}});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->applyMulticlass(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i) {
      for (unsigned int j = 0; j < probabilities[i].size(); ++ j) {
        EXPECT_FLOAT_EQ(probabilities[i][j], dataset.m_data[i][0]);
      }
    }
  }

  TEST(TrivialTest, TrivialPassThroughMulticlassMultiVariable)
  {
    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::TrivialOptions specific_options;
    general_options.m_variables = {"px", "py", "pz"};
    specific_options.m_passthrough = true;
    general_options.m_nClasses = 3;
    TestDataset dataset({{1.0, 2.0, 3.0}, {1.0, 3.0, 4.0}, {1.0, 7.0, 13.0}});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->applyMulticlass(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i) {
      for (unsigned int j = 0; j < probabilities[i].size(); ++ j) {
        EXPECT_FLOAT_EQ(probabilities[i][j], dataset.m_data[i][j]);
      }
    }
  }
}