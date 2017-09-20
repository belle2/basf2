/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/DataDriven.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>
#include <numeric>

using namespace Belle2;

namespace {

  class TestDataset : public MVA::Dataset {
  public:
    explicit TestDataset(MVA::GeneralOptions& general_options) : MVA::Dataset(general_options)
    {
      m_input = {0.0, 0.0};
      m_target = 0.0;
      m_isSignal = false;
      m_weight = 1.0;
      // Suppress cppcheck remark
      // performance: Variable 'm_a' is assigned in constructor body. Consider performing initialization in initialization list.
      // Initializing this vector in the initialization list is not readable, and performance is negligible here
      // cppcheck-suppress *
      m_a = {1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 3.0, 4.0, 3.0, 4.0, 4.0, 4.0, 4.0};
    }

    virtual unsigned int getNumberOfFeatures() const override { return 1; }
    virtual unsigned int getNumberOfSpectators() const override { return 0; }
    virtual unsigned int getNumberOfEvents() const override { return 20; }
    virtual void loadEvent(unsigned int iEvent) override { m_input[0] = m_a[iEvent]; m_target = iEvent % 2; m_isSignal = m_target == 1; };
    virtual float getSignalFraction() override { return 0.5; };
    virtual std::vector<float> getFeature(unsigned int) override { return m_a; }

    std::vector<float> m_a;

  };

  TEST(SPlotTest, SPlotDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    TestDataset dataset(general_options);

    std::vector<float> weights(40);
    std::iota(weights.begin(), weights.end(), 0.0);
    MVA::SPlotDataset splot_dataset(general_options, dataset, weights, 0.5);

    EXPECT_EQ(splot_dataset.getNumberOfFeatures(), 1);
    EXPECT_EQ(splot_dataset.getNumberOfEvents(), 40);
    EXPECT_EQ(splot_dataset.getSignalFraction(), 0.5);

    auto feature = dataset.getFeature(0);
    for (unsigned int i = 0; i < 40; ++i) {
      splot_dataset.loadEvent(i);
      EXPECT_FLOAT_EQ(splot_dataset.m_input[0], feature[i / 2]);
      EXPECT_FLOAT_EQ(splot_dataset.m_weight, 1.0 * i);
      EXPECT_EQ(splot_dataset.m_isSignal, (i % 2) == 0);
    }

  }

  TEST(ReweightingTest, ReweightingDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    TestDataset dataset(general_options);

    std::vector<float> weights(20);
    std::iota(weights.begin(), weights.end(), 0.0);
    MVA::ReweightingDataset reweighting_dataset(general_options, dataset, weights);

    EXPECT_EQ(reweighting_dataset.getNumberOfFeatures(), 1);
    EXPECT_EQ(reweighting_dataset.getNumberOfEvents(), 20);

    auto feature = dataset.getFeature(0);
    for (unsigned int i = 0; i < 20; ++i) {
      reweighting_dataset.loadEvent(i);
      EXPECT_FLOAT_EQ(reweighting_dataset.m_input[0], feature[i]);
      EXPECT_FLOAT_EQ(reweighting_dataset.m_weight, 1.0 * i);
      EXPECT_EQ(reweighting_dataset.m_isSignal, (i % 2) == 1);
    }

  }

  TEST(SPlotTest, GetSPlotWeights)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A", "D"};
    TestDataset dataset(general_options);

    MVA::Binning binning = MVA::Binning::CreateEquidistant(dataset.getFeature(0), dataset.getWeights(), dataset.getSignals(), 4);

    EXPECT_EQ(binning.m_boundaries.size(), 5);
    EXPECT_FLOAT_EQ(binning.m_boundaries[0], 1.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[1], 1.75);
    EXPECT_FLOAT_EQ(binning.m_boundaries[2], 2.5);
    EXPECT_FLOAT_EQ(binning.m_boundaries[3], 3.25);
    EXPECT_FLOAT_EQ(binning.m_boundaries[4], 4.0);

    EXPECT_EQ(binning.m_signal_pdf.size(), 4);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[0], 0.2 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[1], 0.3 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[2], 0.3 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[3], 0.2 * 4.0);

    EXPECT_EQ(binning.m_bckgrd_pdf.size(), 4);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[0], 0.3 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[1], 0.2 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[2], 0.2 * 4.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[3], 0.3 * 4.0);

    EXPECT_FLOAT_EQ(binning.m_signal_yield, 10);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_yield, 10);

    auto splot_weights = MVA::getSPlotWeights(dataset, binning);

    double sum = 0;
    for (auto& s : splot_weights)
      sum += s;
    EXPECT_FLOAT_EQ(sum, 20.0);

    EXPECT_EQ(splot_weights.size(), 40);
    for (unsigned int i = 0; i < 10; i += 2) {
      EXPECT_FLOAT_EQ(splot_weights[i], -2.0);
      EXPECT_FLOAT_EQ(splot_weights[i + 1], 3.0);
    }
    for (unsigned int i = 10; i < 20; i += 2) {
      EXPECT_FLOAT_EQ(splot_weights[i], 3.0);
      EXPECT_FLOAT_EQ(splot_weights[i + 1], -2.0);
    }
    for (unsigned int i = 20; i < 28; i += 2) {
      EXPECT_FLOAT_EQ(splot_weights[i], 3.0);
      EXPECT_FLOAT_EQ(splot_weights[i + 1], -2.0);
    }
    EXPECT_FLOAT_EQ(splot_weights[28], -2.0);
    EXPECT_FLOAT_EQ(splot_weights[29], 3.0);
    EXPECT_FLOAT_EQ(splot_weights[30], 3.0);
    EXPECT_FLOAT_EQ(splot_weights[31], -2.0);
    for (unsigned int i = 32; i < 40; i += 2) {
      EXPECT_FLOAT_EQ(splot_weights[i], -2.0);
      EXPECT_FLOAT_EQ(splot_weights[i + 1], 3.0);
    }

  }

  TEST(SPlotTest, GetBoostWeights)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    TestDataset dataset(general_options);

    MVA::Binning binning = MVA::Binning::CreateEquidistant(dataset.getFeature(0), dataset.getWeights(), dataset.getSignals(), 4);

    auto boost_weights = MVA::getBoostWeights(dataset, binning);

    EXPECT_EQ(boost_weights.size(), 40);
    for (unsigned int i = 0; i < 10; i += 2) {
      EXPECT_FLOAT_EQ(boost_weights[i], 0.2 / 0.3 / 4.0);
      EXPECT_FLOAT_EQ(boost_weights[i + 1], 0.8 / 0.3 / 4.0);
    }
    for (unsigned int i = 10; i < 20; i += 2) {
      EXPECT_FLOAT_EQ(boost_weights[i], 0.5 / 0.2 / 4.0);
      EXPECT_FLOAT_EQ(boost_weights[i + 1], 0.5 / 0.2 / 4.0);
    }
    for (unsigned int i = 20; i < 28; i += 2) {
      EXPECT_FLOAT_EQ(boost_weights[i], 0.8 / 0.2 / 4.0);
      EXPECT_FLOAT_EQ(boost_weights[i + 1], 0.2 / 0.2 / 4.0);
    }
    EXPECT_FLOAT_EQ(boost_weights[28], 1.0 / 0.3 / 4.0);
    EXPECT_FLOAT_EQ(boost_weights[29], 0.0 / 0.3 / 4.0);
    EXPECT_FLOAT_EQ(boost_weights[30], 0.8 / 0.2 / 4.0);
    EXPECT_FLOAT_EQ(boost_weights[31], 0.2 / 0.2 / 4.0);
    for (unsigned int i = 32; i < 40; i += 2) {
      EXPECT_FLOAT_EQ(boost_weights[i], 1.0 / 0.3 / 4.0);
      EXPECT_FLOAT_EQ(boost_weights[i + 1], 0.0 / 0.3 / 4.0);
    }

  }

  TEST(SPlotTest, GetAPlotWeights)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"A"};
    TestDataset dataset(general_options);

    MVA::Binning binning = MVA::Binning::CreateEquidistant(dataset.getFeature(0), dataset.getWeights(), dataset.getSignals(), 4);

    std::vector<float> boost_prediction = {0.0, 0.005, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45,
                                           0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.9, 0.995, 1.0
                                          };
    auto aplot_weights = MVA::getAPlotWeights(dataset, binning, boost_prediction);

    // Regularisation
    boost_prediction[0] = 0.005;
    boost_prediction[19] = 0.995;

    auto splot_weights = MVA::getSPlotWeights(dataset, binning);

    EXPECT_EQ(aplot_weights.size(), 40);
    for (unsigned int i = 0; i < 10; i += 2) {
      double aplot = 0.1 / boost_prediction[i / 2] + 0.4 / (1 -  boost_prediction[i / 2]);
      EXPECT_FLOAT_EQ(aplot_weights[i], aplot * splot_weights[i]);
      EXPECT_FLOAT_EQ(aplot_weights[i + 1], aplot * splot_weights[i + 1]);
    }
    for (unsigned int i = 10; i < 20; i += 2) {
      double aplot = 0.25 / boost_prediction[i / 2] + 0.25 / (1 -  boost_prediction[i / 2]);
      EXPECT_FLOAT_EQ(aplot_weights[i], aplot * splot_weights[i]);
      EXPECT_FLOAT_EQ(aplot_weights[i + 1], aplot * splot_weights[i + 1]);
    }
    for (unsigned int i = 20; i < 28; i += 2) {
      double aplot = 0.4 / boost_prediction[i / 2] + 0.1 / (1 -  boost_prediction[i / 2]);
      EXPECT_FLOAT_EQ(aplot_weights[i], aplot * splot_weights[i]);
      EXPECT_FLOAT_EQ(aplot_weights[i + 1], aplot * splot_weights[i + 1]);
    }
    {
      double aplot = 0.5 / boost_prediction[14];
      EXPECT_FLOAT_EQ(aplot_weights[28], aplot * splot_weights[28]);
      EXPECT_FLOAT_EQ(aplot_weights[29], aplot * splot_weights[29]);
      aplot = 0.4 / boost_prediction[15] + 0.1 / (1 -  boost_prediction[15]);
      EXPECT_FLOAT_EQ(aplot_weights[30], aplot * splot_weights[30]);
      EXPECT_FLOAT_EQ(aplot_weights[31], aplot * splot_weights[31]);
    }
    for (unsigned int i = 32; i < 40; i += 2) {
      double aplot = 0.5 / boost_prediction[i / 2];
      EXPECT_FLOAT_EQ(aplot_weights[i], aplot * splot_weights[i]);
      EXPECT_FLOAT_EQ(aplot_weights[i + 1], aplot * splot_weights[i + 1]);
    }

  }

}
