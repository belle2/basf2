/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/PDF.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(PDFTest, PDFOptions)
  {

    MVA::PDFOptions specific_options;

    EXPECT_EQ(specific_options.m_mode, "probability");
    EXPECT_EQ(specific_options.m_binning, "frequency");
    EXPECT_EQ(specific_options.m_nBins, 100);

    specific_options.m_mode = "mode";
    specific_options.m_binning = "binning";
    specific_options.m_nBins = 3;

    boost::property_tree::ptree pt;
    specific_options.save(pt);
    EXPECT_EQ(pt.get<std::string>("PDF_mode"), "mode");
    EXPECT_EQ(pt.get<std::string>("PDF_binning"), "binning");
    EXPECT_EQ(pt.get<unsigned int>("PDF_nBins"), 3);


    MVA::PDFOptions specific_options2;
    specific_options2.load(pt);

    EXPECT_EQ(specific_options2.m_mode, "mode");
    EXPECT_EQ(specific_options2.m_binning, "binning");
    EXPECT_EQ(specific_options2.m_nBins, 3);

    EXPECT_EQ(specific_options.getMethod(), std::string("PDF"));

    // Test if po::options_description is created without crashing
    auto description = specific_options.getDescription();
    EXPECT_EQ(description.options().size(), 3);

    // Check for B2ERROR and throw if version is wrong
    // we try with version 100, surely we will never reach this!
    pt.put("PDF_version", 100);
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


  TEST(PDFTest, PDFInterface)
  {
    MVA::Interface<MVA::PDFOptions, MVA::PDFTeacher, MVA::PDFExpert> interface;

    MVA::GeneralOptions general_options;
    MVA::PDFOptions specific_options;
    specific_options.m_nBins = 4;
    TestDataset dataset({1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 2.0, 3.0});

    auto teacher = interface.getTeacher(general_options, specific_options);
    auto weightfile = teacher->train(dataset);

    auto expert = interface.getExpert();
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_EQ(probabilities.size(), dataset.getNumberOfEvents());
    EXPECT_FLOAT_EQ(probabilities[0], 0.5);
    EXPECT_FLOAT_EQ(probabilities[1], 0.5);
    EXPECT_FLOAT_EQ(probabilities[2], 0.5);
    EXPECT_FLOAT_EQ(probabilities[3], 0.5);
    EXPECT_FLOAT_EQ(probabilities[4], 0.0);
    EXPECT_FLOAT_EQ(probabilities[5], 1.0);
    EXPECT_FLOAT_EQ(probabilities[6], 0.0);
    EXPECT_FLOAT_EQ(probabilities[7], 1.0);

  }

}
