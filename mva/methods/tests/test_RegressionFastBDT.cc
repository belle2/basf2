/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/RegressionFastBDT.h>
#include <mva/interface/Interface.h>
#include <mva/interface/Dataset.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {
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
    virtual unsigned int getNumberOfSpectators() const override { return 0; }
    virtual unsigned int getNumberOfEvents() const override { return m_data.size(); }
    virtual void loadEvent(unsigned int iEvent) override
    {
      m_input[0] = m_data[iEvent]; m_target = 1.0 * iEvent / 10; m_isSignal = m_target == 1;
    };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;
  };


  TEST(RegressionFastBDTTest, RegressionFastBDTInterface)
  {
    MVA::Interface<MVA::RegressionFastBDTOptions, MVA::RegressionFastBDTTeacher, MVA::RegressionFastBDTExpert> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    MVA::RegressionFastBDTOptions specific_options;
    specific_options.setMaximalBinNumber(3);
    TestDataset dataset({1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    EXPECT_EQ(probabilities.size(), 10);

    for (unsigned int i = 0; i < 5; ++i) {
      EXPECT_LE(probabilities[i], 0.5);
    }
    for (unsigned int i = 5; i < 10; ++i) {
      EXPECT_GE(probabilities[i], 0.5);
    }
  }
}
