/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Dataset.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <boost/filesystem/operations.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <numeric>

using namespace Belle2;

namespace {

  class TestDataset : public MVA::Dataset {
  public:
    explicit TestDataset(MVA::GeneralOptions& general_options) : MVA::Dataset(general_options)
    {
      m_input = {1.0, 2.0, 3.0, 4.0, 5.0};
      m_target = 3.0;
      m_isSignal = true;
      m_weight = -3.0;
    }

    virtual unsigned int getNumberOfFeatures() const override { return 5; }
    virtual unsigned int getNumberOfSpectators() const override { return 2; }
    virtual unsigned int getNumberOfEvents() const override { return 20; }
    virtual void loadEvent(unsigned int iEvent) override
    {
      float f = static_cast<float>(iEvent);
      m_input = {f + 1, f + 2, f + 3, f + 4, f + 5};
      m_spectators = {f + 6, f + 7};
    };
    virtual float getSignalFraction() override { return 0.1; };
    virtual std::vector<float> getFeature(unsigned int iFeature) override
    {
      std::vector<float> a(20, 0.0);
      std::iota(a.begin(), a.end(), iFeature + 1);
      return a;
    }
    virtual std::vector<float> getSpectator(unsigned int iSpectator) override
    {
      std::vector<float> a(20, 0.0);
      std::iota(a.begin(), a.end(), iSpectator + 6);
      return a;
    }

  };

  TEST(DatasetTest, SingleDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"a", "b", "c"};
    general_options.m_spectators = {"e", "f"};
    general_options.m_signal_class = 2;
    std::vector<float> input = {1.0, 2.0, 3.0};
    std::vector<float> spectators = {4.0, 5.0};

    MVA::SingleDataset x(general_options, input, 2.0, spectators);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfSpectators(), 2);
    EXPECT_EQ(x.getNumberOfEvents(), 1);

    EXPECT_EQ(x.getFeatureIndex("a"), 0);
    EXPECT_EQ(x.getFeatureIndex("b"), 1);
    EXPECT_EQ(x.getFeatureIndex("c"), 2);
    EXPECT_B2ERROR(x.getFeatureIndex("bla"));

    EXPECT_EQ(x.getSpectatorIndex("e"), 0);
    EXPECT_EQ(x.getSpectatorIndex("f"), 1);
    EXPECT_B2ERROR(x.getSpectatorIndex("bla"));

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.0);

    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 5.0);

    EXPECT_FLOAT_EQ(x.m_weight, 1.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 1.0);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 1);
    EXPECT_FLOAT_EQ(feature[0], 2.0);

    auto spectator = x.getSpectator(1);
    EXPECT_EQ(spectator.size(), 1);
    EXPECT_FLOAT_EQ(spectator[0], 5.0);

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 1);
    EXPECT_FLOAT_EQ(feature[0], 2.0);

  }

  TEST(DatasetTest, MultiDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"a", "b", "c"};
    general_options.m_spectators = {"e", "f"};
    general_options.m_signal_class = 2;
    std::vector<std::vector<float>> matrix = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    std::vector<std::vector<float>> spectator_matrix = {{12.0, 13.0}, {15.0, 16.0}, {18.0, 19.0}};
    std::vector<float> targets = {2.0, 0.0, 2.0};
    std::vector<float> weights = {1.0, 2.0, 3.0};

    EXPECT_B2ERROR(MVA::MultiDataset(general_options, matrix, spectator_matrix, {1.0}, weights));

    EXPECT_B2ERROR(MVA::MultiDataset(general_options, matrix, spectator_matrix, targets, {1.0}));

    MVA::MultiDataset x(general_options, matrix, spectator_matrix, targets, weights);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfEvents(), 3);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.0);

    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 12.0);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 13.0);

    EXPECT_FLOAT_EQ(x.m_weight, 1.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    // Should just work
    x.loadEvent(1);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 6.0);

    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 15.0);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 16.0);

    EXPECT_FLOAT_EQ(x.m_weight, 2.0);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    // Should just work
    x.loadEvent(2);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 7.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 8.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 9.0);

    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 18.0);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 19.0);

    EXPECT_FLOAT_EQ(x.m_weight, 3.0);
    EXPECT_FLOAT_EQ(x.m_target, 2.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 4.0 / 6.0);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 3);
    EXPECT_FLOAT_EQ(feature[0], 2.0);
    EXPECT_FLOAT_EQ(feature[1], 5.0);
    EXPECT_FLOAT_EQ(feature[2], 8.0);

    auto spectator = x.getSpectator(1);
    EXPECT_EQ(spectator.size(), 3);
    EXPECT_FLOAT_EQ(spectator[0], 13.0);
    EXPECT_FLOAT_EQ(spectator[1], 16.0);
    EXPECT_FLOAT_EQ(spectator[2], 19.0);


  }

  TEST(DatasetTest, SubDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_signal_class = 3;
    general_options.m_variables = {"a", "b", "c", "d", "e"};
    general_options.m_spectators = {"f", "g"};
    TestDataset test_dataset(general_options);

    general_options.m_variables = {"a", "d", "e"};
    general_options.m_spectators = {"g"};
    std::vector<bool> events = {true, false, true, false, true, false, true, false, true, false,
                                true, false, true, false, true, false, true, false, true, false
                               };
    MVA::SubDataset x(general_options, events, test_dataset);

    EXPECT_EQ(x.getNumberOfFeatures(), 3);
    EXPECT_EQ(x.getNumberOfEvents(), 10);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 3);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.0);

    EXPECT_EQ(x.m_spectators.size(), 1);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 7.0);

    EXPECT_FLOAT_EQ(x.m_weight, -3.0);
    EXPECT_FLOAT_EQ(x.m_target, 3.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 1);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent * 2 + 4);
    };

    auto spectator = x.getSpectator(0);
    EXPECT_EQ(spectator.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], iEvent * 2 + 7);
    };

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent * 2 + 4);
    };

    spectator = x.Dataset::getSpectator(0);
    EXPECT_EQ(spectator.size(), 10);
    for (unsigned int iEvent = 0; iEvent < 10; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], iEvent * 2 + 7);
    };

    // Test without event indices
    MVA::SubDataset y(general_options, {}, test_dataset);
    feature = y.getFeature(1);
    EXPECT_EQ(feature.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent + 4);
    };

    spectator = y.getSpectator(0);
    EXPECT_EQ(spectator.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], iEvent + 7);
    };

    // Same result for mother class implementation
    feature = y.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], iEvent + 4);
    };

    spectator = y.Dataset::getSpectator(0);
    EXPECT_EQ(spectator.size(), 20);
    for (unsigned int iEvent = 0; iEvent < 20; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], iEvent + 7);
    };

    general_options.m_variables = {"a", "d", "e", "DOESNOTEXIST"};
    try {
      EXPECT_B2ERROR(MVA::SubDataset(general_options, events, test_dataset));
    } catch (...) {

    }
    EXPECT_THROW(MVA::SubDataset(general_options, events, test_dataset), std::runtime_error);

    general_options.m_variables = {"a", "d", "e"};
    general_options.m_spectators = {"DOESNOTEXIST"};
    try {
      EXPECT_B2ERROR(MVA::SubDataset(general_options, events, test_dataset));
    } catch (...) {

    }
    EXPECT_THROW(MVA::SubDataset(general_options, events, test_dataset), std::runtime_error);

  }

  TEST(DatasetTest, CombinedDataset)
  {

    MVA::GeneralOptions general_options;
    general_options.m_signal_class = 1;
    general_options.m_variables = {"a", "b", "c", "d", "e"};
    general_options.m_spectators = {"f", "g"};
    TestDataset signal_dataset(general_options);
    TestDataset bckgrd_dataset(general_options);

    MVA::CombinedDataset x(general_options, signal_dataset, bckgrd_dataset);

    EXPECT_EQ(x.getNumberOfFeatures(), 5);
    EXPECT_EQ(x.getNumberOfEvents(), 40);

    // Should just work
    x.loadEvent(0);

    EXPECT_EQ(x.m_input.size(), 5);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.0);
    EXPECT_FLOAT_EQ(x.m_input[3], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[4], 5.0);

    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 6.0);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 7.0);

    EXPECT_FLOAT_EQ(x.m_weight, -3.0);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 0.5);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 40);
    for (unsigned int iEvent = 0; iEvent < 40; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], (iEvent % 20) + 2);
    };

    auto spectator = x.getSpectator(0);
    EXPECT_EQ(spectator.size(), 40);
    for (unsigned int iEvent = 0; iEvent < 40; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], (iEvent % 20) + 6);
    };

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 40);
    for (unsigned int iEvent = 0; iEvent < 40; ++iEvent) {
      EXPECT_FLOAT_EQ(feature[iEvent], (iEvent % 20) + 2);
    };

    spectator = x.Dataset::getSpectator(0);
    EXPECT_EQ(spectator.size(), 40);
    for (unsigned int iEvent = 0; iEvent < 40; ++iEvent) {
      EXPECT_FLOAT_EQ(spectator[iEvent], (iEvent % 20) + 6);
    };

    for (unsigned int iEvent = 0; iEvent < 40; ++iEvent) {
      x.loadEvent(iEvent);
      EXPECT_EQ(x.m_isSignal, iEvent < 20);
      EXPECT_FLOAT_EQ(x.m_target, iEvent < 20 ? 1.0 : 0.0);
    }

  }

  TEST(DatasetTest, ROOTDataset)
  {

    TestHelpers::TempDirCreator tmp_dir;
    TFile file("datafile.root", "RECREATE");
    file.cd();
    TTree tree("tree", "TreeTitle");
    float a, b, c, d, e, f, g, v, w = 0;
    tree.Branch("a", &a);
    tree.Branch("b", &b);
    tree.Branch("c", &c);
    tree.Branch("d", &d);
    tree.Branch("e__bo__bc", &e);
    tree.Branch("f__bo__bc", &f);
    tree.Branch("g", &g);
    tree.Branch("__weight__", &c);
    tree.Branch("v__bo__bc", &v);
    tree.Branch("w", &w);

    for (unsigned int i = 0; i < 5; ++i) {
      a = i + 1.0;
      b = i + 1.1;
      c = i + 1.2;
      d = i + 1.3;
      e = i + 1.4;
      f = i + 1.5;
      g = i % 2 == 0;
      w = i + 1.6;
      v = i + 1.7;
      tree.Fill();
    }

    file.Write("tree");

    MVA::GeneralOptions general_options;
    // Both names with and without makeROOTCompatible should work
    general_options.m_variables = {"a", "b", "e__bo__bc", "f()"};
    general_options.m_spectators = {"w", "v()"};
    general_options.m_signal_class = 1;
    general_options.m_datafiles = {"datafile.root"};
    general_options.m_treename = "tree";
    general_options.m_target_variable = "g";
    general_options.m_weight_variable = "c";
    MVA::ROOTDataset x(general_options);

    EXPECT_EQ(x.getNumberOfFeatures(), 4);
    EXPECT_EQ(x.getNumberOfSpectators(), 2);
    EXPECT_EQ(x.getNumberOfEvents(), 5);

    // Should just work
    x.loadEvent(0);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 1.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 1.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 1.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 1.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 1.7);
    EXPECT_FLOAT_EQ(x.m_weight, 1.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(1);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 2.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 2.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 2.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 2.7);
    EXPECT_FLOAT_EQ(x.m_weight, 2.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(2);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 3.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 3.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 3.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 3.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 3.7);
    EXPECT_FLOAT_EQ(x.m_weight, 3.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(3);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 4.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 4.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 4.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 4.7);
    EXPECT_FLOAT_EQ(x.m_weight, 4.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(4);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 5.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 5.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 5.7);
    EXPECT_FLOAT_EQ(x.m_weight, 5.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 0.6);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 5);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 5);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);

    auto spectator = x.getSpectator(1);
    EXPECT_EQ(spectator.size(), 5);
    EXPECT_FLOAT_EQ(spectator[0], 1.7);
    EXPECT_FLOAT_EQ(spectator[1], 2.7);
    EXPECT_FLOAT_EQ(spectator[2], 3.7);
    EXPECT_FLOAT_EQ(spectator[3], 4.7);
    EXPECT_FLOAT_EQ(spectator[4], 5.7);

    // Same result for mother class implementation
    spectator = x.Dataset::getSpectator(1);
    EXPECT_EQ(spectator.size(), 5);
    EXPECT_FLOAT_EQ(spectator[0], 1.7);
    EXPECT_FLOAT_EQ(spectator[1], 2.7);
    EXPECT_FLOAT_EQ(spectator[2], 3.7);
    EXPECT_FLOAT_EQ(spectator[3], 4.7);
    EXPECT_FLOAT_EQ(spectator[4], 5.7);

    auto weights = x.getWeights();
    EXPECT_EQ(weights.size(), 5);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);

    // Same result for mother class implementation
    weights = x.Dataset::getWeights();
    EXPECT_EQ(weights.size(), 5);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);

    auto targets = x.getTargets();
    EXPECT_EQ(targets.size(), 5);
    EXPECT_FLOAT_EQ(targets[0], 1.0);
    EXPECT_FLOAT_EQ(targets[1], 0.0);
    EXPECT_FLOAT_EQ(targets[2], 1.0);
    EXPECT_FLOAT_EQ(targets[3], 0.0);
    EXPECT_FLOAT_EQ(targets[4], 1.0);

    auto signals = x.getSignals();
    EXPECT_EQ(signals.size(), 5);
    EXPECT_EQ(signals[0], true);
    EXPECT_EQ(signals[1], false);
    EXPECT_EQ(signals[2], true);
    EXPECT_EQ(signals[3], false);
    EXPECT_EQ(signals[4], true);

    // Using __weight__ should work as well,
    // the only difference to using _weight__ instead of g is
    // in setBranchAddresses which avoids calling makeROOTCompatible
    // So we have to check the behaviour using __weight__ as well
    general_options.m_weight_variable = "__weight__";
    MVA::ROOTDataset y(general_options);

    weights = y.getWeights();
    EXPECT_EQ(weights.size(), 5);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);

    // Check TChain expansion
    general_options.m_datafiles = {"datafile*.root"};
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 5);
    }
    boost::filesystem::copy_file("datafile.root", "datafile2.root");
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 10);
    }
    boost::filesystem::copy_file("datafile.root", "datafile3.root");
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 15);
    }
    // Test m_max_events feature
    {
      general_options.m_max_events = 10;
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 10);
      general_options.m_max_events = 0;
    }

    // Check for missing tree
    general_options.m_treename = "missing tree";
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for missing branch
    general_options.m_treename = "tree";
    general_options.m_variables = {"a", "b", "e", "f", "missing branch"};
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for missing branch
    general_options.m_treename = "tree";
    general_options.m_variables = {"a", "b", "e", "f"};
    general_options.m_spectators = {"missing branch"};
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for missing file
    general_options.m_spectators = {};
    general_options.m_datafiles = {"DOESNOTEXIST.root"};
    general_options.m_treename = "tree";
    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);

    // Check for invalid file
    general_options.m_datafiles = {"ISNotAValidROOTFile"};
    general_options.m_treename = "tree";

    {
      std::ofstream(general_options.m_datafiles[0]);
    }
    EXPECT_TRUE(boost::filesystem::exists(general_options.m_datafiles[0]));

    try {
      EXPECT_B2ERROR(MVA::ROOTDataset{general_options});
    } catch (...) {

    }
    EXPECT_THROW(MVA::ROOTDataset{general_options}, std::runtime_error);


  }

  TEST(DatasetTest, ROOTMultiDataset)
  {

    TestHelpers::TempDirCreator tmp_dir;
    TFile file("datafile.root", "RECREATE");
    file.cd();
    TTree tree("tree", "TreeTitle");
    float a, b, c, d, e, f, g, v, w = 0;
    tree.Branch("a", &a);
    tree.Branch("b", &b);
    tree.Branch("c", &c);
    tree.Branch("d", &d);
    tree.Branch("e__bo__bc", &e);
    tree.Branch("f__bo__bc", &f);
    tree.Branch("g", &g);
    tree.Branch("__weight__", &c);
    tree.Branch("v__bo__bc", &v);
    tree.Branch("w", &w);

    for (unsigned int i = 0; i < 5; ++i) {
      a = i + 1.0;
      b = i + 1.1;
      c = i + 1.2;
      d = i + 1.3;
      e = i + 1.4;
      f = i + 1.5;
      g = i % 2 == 0;
      w = i + 1.6;
      v = i + 1.7;
      tree.Fill();
    }

    file.Write("tree");

    TFile file2("datafile2.root", "RECREATE");
    file2.cd();
    TTree tree2("tree", "TreeTitle");
    tree2.Branch("a", &a);
    tree2.Branch("b", &b);
    tree2.Branch("c", &c);
    tree2.Branch("d", &d);
    tree2.Branch("e__bo__bc", &e);
    tree2.Branch("f__bo__bc", &f);
    tree2.Branch("g", &g);
    tree2.Branch("__weight__", &c);
    tree2.Branch("v__bo__bc", &v);
    tree2.Branch("w", &w);

    for (unsigned int i = 0; i < 5; ++i) {
      a = i + 1.0;
      b = i + 1.1;
      c = i + 1.2;
      d = i + 1.3;
      e = i + 1.4;
      f = i + 1.5;
      g = i % 2 == 0;
      w = i + 1.6;
      v = i + 1.7;
      tree2.Fill();
    }

    file2.Write("tree");

    MVA::GeneralOptions general_options;
    // Both names with and without makeROOTCompatible should work
    general_options.m_variables = {"a", "b", "e__bo__bc", "f()"};
    general_options.m_spectators = {"w", "v()"};
    general_options.m_signal_class = 1;
    general_options.m_datafiles = {"datafile.root", "datafile2.root"};
    general_options.m_treename = "tree";
    general_options.m_target_variable = "g";
    general_options.m_weight_variable = "c";
    MVA::ROOTDataset x(general_options);

    EXPECT_EQ(x.getNumberOfFeatures(), 4);
    EXPECT_EQ(x.getNumberOfSpectators(), 2);
    EXPECT_EQ(x.getNumberOfEvents(), 10);

    // Should just work
    x.loadEvent(0);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 1.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 1.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 1.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 1.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 1.7);
    EXPECT_FLOAT_EQ(x.m_weight, 1.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(5);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 1.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 1.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 1.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 1.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 1.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 1.7);
    EXPECT_FLOAT_EQ(x.m_weight, 1.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(1);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 2.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 2.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 2.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 2.7);
    EXPECT_FLOAT_EQ(x.m_weight, 2.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(6);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 2.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 2.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 2.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 2.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 2.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 2.7);
    EXPECT_FLOAT_EQ(x.m_weight, 2.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(2);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 3.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 3.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 3.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 3.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 3.7);
    EXPECT_FLOAT_EQ(x.m_weight, 3.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(7);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 3.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 3.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 3.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 3.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 3.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 3.7);
    EXPECT_FLOAT_EQ(x.m_weight, 3.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(3);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 4.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 4.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 4.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 4.7);
    EXPECT_FLOAT_EQ(x.m_weight, 4.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(8);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 4.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 4.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 4.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 4.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 4.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 4.7);
    EXPECT_FLOAT_EQ(x.m_weight, 4.2);
    EXPECT_FLOAT_EQ(x.m_target, 0.0);
    EXPECT_EQ(x.m_isSignal, false);

    x.loadEvent(4);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 5.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 5.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 5.7);
    EXPECT_FLOAT_EQ(x.m_weight, 5.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    x.loadEvent(9);
    EXPECT_EQ(x.m_input.size(), 4);
    EXPECT_FLOAT_EQ(x.m_input[0], 5.0);
    EXPECT_FLOAT_EQ(x.m_input[1], 5.1);
    EXPECT_FLOAT_EQ(x.m_input[2], 5.4);
    EXPECT_FLOAT_EQ(x.m_input[3], 5.5);
    EXPECT_EQ(x.m_spectators.size(), 2);
    EXPECT_FLOAT_EQ(x.m_spectators[0], 5.6);
    EXPECT_FLOAT_EQ(x.m_spectators[1], 5.7);
    EXPECT_FLOAT_EQ(x.m_weight, 5.2);
    EXPECT_FLOAT_EQ(x.m_target, 1.0);
    EXPECT_EQ(x.m_isSignal, true);

    EXPECT_FLOAT_EQ(x.getSignalFraction(), 0.6);

    auto feature = x.getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);
    EXPECT_FLOAT_EQ(feature[5], 1.1);
    EXPECT_FLOAT_EQ(feature[6], 2.1);
    EXPECT_FLOAT_EQ(feature[7], 3.1);
    EXPECT_FLOAT_EQ(feature[8], 4.1);
    EXPECT_FLOAT_EQ(feature[9], 5.1);

    // Same result for mother class implementation
    feature = x.Dataset::getFeature(1);
    EXPECT_EQ(feature.size(), 10);
    EXPECT_FLOAT_EQ(feature[0], 1.1);
    EXPECT_FLOAT_EQ(feature[1], 2.1);
    EXPECT_FLOAT_EQ(feature[2], 3.1);
    EXPECT_FLOAT_EQ(feature[3], 4.1);
    EXPECT_FLOAT_EQ(feature[4], 5.1);
    EXPECT_FLOAT_EQ(feature[5], 1.1);
    EXPECT_FLOAT_EQ(feature[6], 2.1);
    EXPECT_FLOAT_EQ(feature[7], 3.1);
    EXPECT_FLOAT_EQ(feature[8], 4.1);
    EXPECT_FLOAT_EQ(feature[9], 5.1);

    auto spectator = x.getSpectator(1);
    EXPECT_EQ(spectator.size(), 10);
    EXPECT_FLOAT_EQ(spectator[0], 1.7);
    EXPECT_FLOAT_EQ(spectator[1], 2.7);
    EXPECT_FLOAT_EQ(spectator[2], 3.7);
    EXPECT_FLOAT_EQ(spectator[3], 4.7);
    EXPECT_FLOAT_EQ(spectator[4], 5.7);
    EXPECT_FLOAT_EQ(spectator[5], 1.7);
    EXPECT_FLOAT_EQ(spectator[6], 2.7);
    EXPECT_FLOAT_EQ(spectator[7], 3.7);
    EXPECT_FLOAT_EQ(spectator[8], 4.7);
    EXPECT_FLOAT_EQ(spectator[9], 5.7);

    // Same result for mother class implementation
    spectator = x.Dataset::getSpectator(1);
    EXPECT_EQ(spectator.size(), 10);
    EXPECT_FLOAT_EQ(spectator[0], 1.7);
    EXPECT_FLOAT_EQ(spectator[1], 2.7);
    EXPECT_FLOAT_EQ(spectator[2], 3.7);
    EXPECT_FLOAT_EQ(spectator[3], 4.7);
    EXPECT_FLOAT_EQ(spectator[4], 5.7);
    EXPECT_FLOAT_EQ(spectator[5], 1.7);
    EXPECT_FLOAT_EQ(spectator[6], 2.7);
    EXPECT_FLOAT_EQ(spectator[7], 3.7);
    EXPECT_FLOAT_EQ(spectator[8], 4.7);
    EXPECT_FLOAT_EQ(spectator[9], 5.7);

    auto weights = x.getWeights();
    EXPECT_EQ(weights.size(), 10);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);
    EXPECT_FLOAT_EQ(weights[5], 1.2);
    EXPECT_FLOAT_EQ(weights[6], 2.2);
    EXPECT_FLOAT_EQ(weights[7], 3.2);
    EXPECT_FLOAT_EQ(weights[8], 4.2);
    EXPECT_FLOAT_EQ(weights[9], 5.2);

    // Same result for mother class implementation
    weights = x.Dataset::getWeights();
    EXPECT_EQ(weights.size(), 10);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);
    EXPECT_FLOAT_EQ(weights[5], 1.2);
    EXPECT_FLOAT_EQ(weights[6], 2.2);
    EXPECT_FLOAT_EQ(weights[7], 3.2);
    EXPECT_FLOAT_EQ(weights[8], 4.2);
    EXPECT_FLOAT_EQ(weights[9], 5.2);

    auto targets = x.getTargets();
    EXPECT_EQ(targets.size(), 10);
    EXPECT_FLOAT_EQ(targets[0], 1.0);
    EXPECT_FLOAT_EQ(targets[1], 0.0);
    EXPECT_FLOAT_EQ(targets[2], 1.0);
    EXPECT_FLOAT_EQ(targets[3], 0.0);
    EXPECT_FLOAT_EQ(targets[4], 1.0);
    EXPECT_FLOAT_EQ(targets[5], 1.0);
    EXPECT_FLOAT_EQ(targets[6], 0.0);
    EXPECT_FLOAT_EQ(targets[7], 1.0);
    EXPECT_FLOAT_EQ(targets[8], 0.0);
    EXPECT_FLOAT_EQ(targets[9], 1.0);

    auto signals = x.getSignals();
    EXPECT_EQ(signals.size(), 10);
    EXPECT_EQ(signals[0], true);
    EXPECT_EQ(signals[1], false);
    EXPECT_EQ(signals[2], true);
    EXPECT_EQ(signals[3], false);
    EXPECT_EQ(signals[4], true);
    EXPECT_EQ(signals[5], true);
    EXPECT_EQ(signals[6], false);
    EXPECT_EQ(signals[7], true);
    EXPECT_EQ(signals[8], false);
    EXPECT_EQ(signals[9], true);

    // Using __weight__ should work as well,
    // the only difference to using _weight__ instead of g is
    // in setBranchAddresses which avoids calling makeROOTCompatible
    // So we have to check the behaviour using __weight__ as well
    general_options.m_weight_variable = "__weight__";
    MVA::ROOTDataset y(general_options);

    weights = y.getWeights();
    EXPECT_EQ(weights.size(), 10);
    EXPECT_FLOAT_EQ(weights[0], 1.2);
    EXPECT_FLOAT_EQ(weights[1], 2.2);
    EXPECT_FLOAT_EQ(weights[2], 3.2);
    EXPECT_FLOAT_EQ(weights[3], 4.2);
    EXPECT_FLOAT_EQ(weights[4], 5.2);
    EXPECT_FLOAT_EQ(weights[5], 1.2);
    EXPECT_FLOAT_EQ(weights[6], 2.2);
    EXPECT_FLOAT_EQ(weights[7], 3.2);
    EXPECT_FLOAT_EQ(weights[8], 4.2);
    EXPECT_FLOAT_EQ(weights[9], 5.2);

    // Check TChain expansion
    general_options.m_datafiles = {"datafile*.root"};
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 10);
    }
    boost::filesystem::copy_file("datafile.root", "datafile3.root");
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 15);
    }
    boost::filesystem::copy_file("datafile.root", "datafile4.root");
    {
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 20);
    }
    // Test m_max_events feature
    {
      general_options.m_max_events = 10;
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 10);
      general_options.m_max_events = 0;
    }

    // If a file exists with the specified expansion
    // the file takes precedence over the expansion
    boost::filesystem::copy_file("datafile.root", "datafile*.root");
    {
      general_options.m_max_events = 0;
      MVA::ROOTDataset chain_test(general_options);
      EXPECT_EQ(chain_test.getNumberOfEvents(), 5);
    }

  }


}
