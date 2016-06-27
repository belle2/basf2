/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/Combination.h>
#include <mva/methods/Trivial.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(CombinationTest, CombinationOptions)
  {

    MVA::CombinationOptions specific_options;

    EXPECT_EQ(specific_options.m_weightfiles.size(), 0);

    specific_options.m_weightfiles = {"A", "B"};

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_FLOAT_EQ(pt.get<unsigned int>("Combination_number_of_weightfiles"), 2);
    EXPECT_EQ(pt.get<std::string>("Combination_weightfile0"), "A");
    EXPECT_EQ(pt.get<std::string>("Combination_weightfile1"), "B");

    MVA::CombinationOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_weightfiles.size(), 2);
    EXPECT_EQ(specific_options2.m_weightfiles[0], "A");
    EXPECT_EQ(specific_options2.m_weightfiles[1], "B");

    EXPECT_EQ(specific_options.getMethod(), std::string("Combination"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 1);

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("Combination_version", 100);
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


  TEST(CombinationTest, CombinationInterface)
  {
    TestHelpers::TempDirCreator tmp_dir;

    MVA::GeneralOptions general_options;
    general_options.m_method = "Trivial";
    TestDataset dataset({1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 2.0, 3.0});

    MVA::Interface<MVA::TrivialOptions, MVA::TrivialTeacher, MVA::TrivialExpert> trivial;

    MVA::TrivialOptions trivial_options;
    trivial_options.m_output = 0.1;
    auto trivial_teacher1 = trivial.getTeacher(general_options, trivial_options);
    auto trivial_weightfile1 = trivial_teacher1->train(dataset);
    MVA::Weightfile::saveToXMLFile(trivial_weightfile1, "weightfile1.xml");

    trivial_options.m_output = 0.6;
    auto trivial_teacher2 = trivial.getTeacher(general_options, trivial_options);
    auto trivial_weightfile2 = trivial_teacher2->train(dataset);
    MVA::Weightfile::saveToXMLFile(trivial_weightfile2, "weightfile2.xml");

    MVA::Interface<MVA::CombinationOptions, MVA::CombinationTeacher, MVA::CombinationExpert> combination;
    general_options.m_method = "Combination";
    MVA::CombinationOptions specific_options;
    specific_options.m_weightfiles = {"weightfile1.xml", "weightfile2.xml"};
    auto teacher = combination.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = combination.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    for (unsigned int i = 0; i < dataset.getNumberOfEvents(); ++i)
      EXPECT_FLOAT_EQ(probabilities[i], (0.1 * 0.6) / (0.1 * 0.6 + (1 - 0.1) * (1 - 0.6)));

    // Error and throw runtime error if method does not exist
    trivial_weightfile2.addElement("method", "DOESNOTEXIST");
    MVA::Weightfile::saveToXMLFile(trivial_weightfile2, "weightfile2.xml");
    weightfile = teacher->train(dataset);
    try {
      EXPECT_B2ERROR(expert->load(weightfile));
    } catch (...) {

    }
    EXPECT_THROW(expert->load(weightfile), std::runtime_error);


  }

}
