/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/FastBDT.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(FastBDTTest, FastBDTOptions)
  {
    MVA::FastBDTOptions specific_options;

    EXPECT_EQ(specific_options.m_nTrees, 400);
    EXPECT_EQ(specific_options.m_nCuts, 8);
    EXPECT_EQ(specific_options.m_nLevels, 3);
    EXPECT_FLOAT_EQ(specific_options.m_shrinkage, 0.1);
    EXPECT_FLOAT_EQ(specific_options.m_randRatio, 0.5);

    specific_options.m_nTrees = 100;
    specific_options.m_nCuts = 10;
    specific_options.m_nLevels = 2;
    specific_options.m_shrinkage = 0.2;
    specific_options.m_randRatio = 0.8;

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nTrees"), 100);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nCuts"), 10);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nLevels"), 2);
    EXPECT_FLOAT_EQ(pt.get<double>("FastBDT_shrinkage"), 0.2);
    EXPECT_FLOAT_EQ(pt.get<double>("FastBDT_randRatio"), 0.8);

    MVA::FastBDTOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_nTrees, 100);
    EXPECT_EQ(specific_options2.m_nCuts, 10);
    EXPECT_EQ(specific_options2.m_nLevels, 2);
    EXPECT_FLOAT_EQ(specific_options2.m_shrinkage, 0.2);
    EXPECT_FLOAT_EQ(specific_options2.m_randRatio, 0.8);

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


  TEST(FastBDTTest, FastBDTInterface)
  {
    MVA::Interface<MVA::FastBDTOptions, MVA::FastBDTTeacher, MVA::FastBDTExpert> interface("TestFastBDT");

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    MVA::FastBDTOptions specific_options;
    TestDataset dataset({1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 2.0, 3.0});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < 4; ++i) {
      EXPECT_LE(probabilities[i], 0.6);
      EXPECT_GE(probabilities[i], 0.4);
    }
    EXPECT_LE(probabilities[4], 0.2);
    EXPECT_GE(probabilities[5], 0.8);
    EXPECT_LE(probabilities[6], 0.2);
    EXPECT_GE(probabilities[7], 0.8);


  }

}
