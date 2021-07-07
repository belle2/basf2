/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/FastBDT.h>
#include <mva/interface/Interface.h>
#include <mva/interface/Dataset.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(FastBDTTest, FastBDTOptions)
  {
    MVA::FastBDTOptions specific_options;

    EXPECT_EQ(specific_options.m_nTrees, 200);
    EXPECT_EQ(specific_options.m_nCuts, 8);
    EXPECT_EQ(specific_options.m_nLevels, 3);
    EXPECT_FLOAT_EQ(specific_options.m_shrinkage, 0.1);
    EXPECT_FLOAT_EQ(specific_options.m_randRatio, 0.5);
#if FastBDT_VERSION_MAJOR >= 5
    EXPECT_EQ(specific_options.m_sPlot, false);
    EXPECT_EQ(specific_options.m_individual_nCuts.size(), 0);
    EXPECT_EQ(specific_options.m_individualPurityTransformation.size(), 0);
    EXPECT_EQ(specific_options.m_purityTransformation, false);
    EXPECT_FLOAT_EQ(specific_options.m_flatnessLoss, -1.0);
#endif

    specific_options.m_nTrees = 100;
    specific_options.m_nCuts = 10;
    specific_options.m_nLevels = 2;
    specific_options.m_shrinkage = 0.2;
    specific_options.m_randRatio = 0.8;
#if FastBDT_VERSION_MAJOR >= 5
    specific_options.m_individual_nCuts = {2, 3, 4};
    specific_options.m_flatnessLoss = 0.3;
    specific_options.m_sPlot = true;
    specific_options.m_purityTransformation = true;
    specific_options.m_individualPurityTransformation = {true, false, true};
#endif

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nTrees"), 100);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nCuts"), 10);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_nLevels"), 2);
    EXPECT_FLOAT_EQ(pt.get<double>("FastBDT_shrinkage"), 0.2);
    EXPECT_FLOAT_EQ(pt.get<double>("FastBDT_randRatio"), 0.8);
#if FastBDT_VERSION_MAJOR >= 5
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_number_individual_nCuts"), 3);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_individual_nCuts0"), 2);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_individual_nCuts1"), 3);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_individual_nCuts2"), 4);
    EXPECT_EQ(pt.get<bool>("FastBDT_sPlot"), true);
    EXPECT_FLOAT_EQ(pt.get<double>("FastBDT_flatnessLoss"), 0.3);
    EXPECT_EQ(pt.get<bool>("FastBDT_purityTransformation"), true);
    EXPECT_EQ(pt.get<unsigned int>("FastBDT_number_individualPurityTransformation"), 3);
    EXPECT_EQ(pt.get<bool>("FastBDT_individualPurityTransformation0"), true);
    EXPECT_EQ(pt.get<bool>("FastBDT_individualPurityTransformation1"), false);
    EXPECT_EQ(pt.get<bool>("FastBDT_individualPurityTransformation2"), true);
#endif

    MVA::FastBDTOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_nTrees, 100);
    EXPECT_EQ(specific_options2.m_nCuts, 10);
    EXPECT_EQ(specific_options2.m_nLevels, 2);
    EXPECT_FLOAT_EQ(specific_options2.m_shrinkage, 0.2);
    EXPECT_FLOAT_EQ(specific_options2.m_randRatio, 0.8);
#if FastBDT_VERSION_MAJOR >= 5
    EXPECT_EQ(specific_options2.m_sPlot, true);
    EXPECT_FLOAT_EQ(specific_options2.m_flatnessLoss, 0.3);
    EXPECT_EQ(specific_options2.m_purityTransformation, true);
    EXPECT_EQ(specific_options2.m_individualPurityTransformation.size(), 3);
    EXPECT_EQ(specific_options2.m_individualPurityTransformation[0], true);
    EXPECT_EQ(specific_options2.m_individualPurityTransformation[1], false);
    EXPECT_EQ(specific_options2.m_individualPurityTransformation[2], true);
    EXPECT_EQ(specific_options2.m_individual_nCuts.size(), 3);
    EXPECT_EQ(specific_options2.m_individual_nCuts[0], 2);
    EXPECT_EQ(specific_options2.m_individual_nCuts[1], 3);
    EXPECT_EQ(specific_options2.m_individual_nCuts[2], 4);
#endif

    EXPECT_EQ(specific_options.getMethod(), std::string("FastBDT"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();

    // flatnessLoss, sPlot and individualNCuts are only activated in FastBDT version 4
#if FastBDT_VERSION_MAJOR >= 5
    EXPECT_EQ(description.options().size(), 10);
#else
    EXPECT_EQ(description.options().size(), 5);
#endif

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("FastBDT_version", 100);
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

    [[nodiscard]] unsigned int getNumberOfFeatures() const override { return 1; }
    [[nodiscard]] unsigned int getNumberOfSpectators() const override { return 0; }
    [[nodiscard]] unsigned int getNumberOfEvents() const override { return m_data.size(); }
    void loadEvent(unsigned int iEvent) override { m_input[0] = m_data[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    float getSignalFraction() override { return 0.1; };
    std::vector<float> getFeature(unsigned int) override { return m_data; }

    std::vector<float> m_data;

  };


  TEST(FastBDTTest, FastBDTInterface)
  {
    MVA::Interface<MVA::FastBDTOptions, MVA::FastBDTTeacher, MVA::FastBDTExpert> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    MVA::FastBDTOptions specific_options;
    specific_options.m_randRatio = 1.0;
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

#if FastBDT_VERSION_MAJOR >= 5
  TEST(FastBDTTest, FastBDTInterfaceWithPurityTransformation)
  {
    MVA::Interface<MVA::FastBDTOptions, MVA::FastBDTTeacher, MVA::FastBDTExpert> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    MVA::FastBDTOptions specific_options;
    specific_options.m_randRatio = 1.0;
    specific_options.m_purityTransformation = true;
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
#endif

  TEST(FastBDTTest, WeightfilesOfDifferentVersionsAreConsistent)
  {
    MVA::Interface<MVA::FastBDTOptions, MVA::FastBDTTeacher, MVA::FastBDTExpert> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"M", "p", "pt"};
    MVA::MultiDataset dataset(general_options, {{1.835127, 1.179507, 1.164944},
      {1.873689, 1.881940, 1.843310},
      {1.863657, 1.774831, 1.753773},
      {1.858293, 1.605311, 0.631336},
      {1.837129, 1.575739, 1.490166},
      {1.811395, 1.524029, 0.565220}
    },
    {}, {0.0, 1.0, 0.0, 1.0, 0.0, 1.0});

    // cppcheck-suppress unreadVariable
    auto expert = interface.getExpert();

#if FastBDT_VERSION_MAJOR >= 3
    auto weightfile_v3 = MVA::Weightfile::loadFromFile(FileSystem::findFile("mva/methods/tests/FastBDTv3.xml"));
    expert->load(weightfile_v3);
    auto probabilities_v3 = expert->apply(dataset);
    EXPECT_NEAR(probabilities_v3[0], 0.0402499, 0.0001);
    EXPECT_NEAR(probabilities_v3[1], 0.2189, 0.0001);
    EXPECT_NEAR(probabilities_v3[2], 0.264094, 0.0001);
    EXPECT_NEAR(probabilities_v3[3], 0.100049, 0.0001);
    EXPECT_NEAR(probabilities_v3[4], 0.0664554, 0.0001);
    EXPECT_NEAR(probabilities_v3[5], 0.00886221, 0.0001);
#endif

#if FastBDT_VERSION_MAJOR >= 5
    auto weightfile_v5 = MVA::Weightfile::loadFromFile(FileSystem::findFile("mva/methods/tests/FastBDTv5.xml"));
    expert->load(weightfile_v5);
    auto probabilities_v5 = expert->apply(dataset);
    EXPECT_NEAR(probabilities_v5[0], 0.0402498, 0.0001);
    EXPECT_NEAR(probabilities_v5[1], 0.218899, 0.0001);
    EXPECT_NEAR(probabilities_v5[2], 0.264093, 0.0001);
    EXPECT_NEAR(probabilities_v5[3], 0.100048, 0.0001);
    EXPECT_NEAR(probabilities_v5[4], 0.0664551, 0.0001);
    EXPECT_NEAR(probabilities_v5[5], 0.00886217, 0.0001);

    // There are small differences due to floating point precision
    // the FastBDT code is compiled with -O3 and changes slightly
    // depending on the compiler and random things.
    // Nevertheless the returned probabilities should be nearly the same!
    EXPECT_NEAR(probabilities_v5[0], probabilities_v3[0], 0.001);
    EXPECT_NEAR(probabilities_v5[1], probabilities_v3[1], 0.001);
    EXPECT_NEAR(probabilities_v5[2], probabilities_v3[2], 0.001);
    EXPECT_NEAR(probabilities_v5[3], probabilities_v3[3], 0.001);
    EXPECT_NEAR(probabilities_v5[4], probabilities_v3[4], 0.001);
    EXPECT_NEAR(probabilities_v5[5], probabilities_v3[5], 0.001);
#endif
  }

}
