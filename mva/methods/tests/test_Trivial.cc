/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_FLOAT_EQ(pt.get<double>("Trivial_output"), 0.1);

    MVA::TrivialOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_output, 0.1);

    EXPECT_EQ(specific_options.getMethod(), std::string("Trivial"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 1);

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
    virtual void loadEvent(unsigned int iEvent) override { m_input[0] = m_data[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;

  };


  TEST(TrivialTest, TrivialInterface)
  {
    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::TrivialOptions specific_options;
    TestDataset dataset({1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 2.0, 3.0});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i)
      EXPECT_FLOAT_EQ(probabilities[i], 0.5);

  }

}
