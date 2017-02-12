/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Options.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <boost/property_tree/ptree.hpp>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(OptionsTest, GeneralOptions)
  {

    MVA::GeneralOptions general_options;

    EXPECT_EQ(general_options.m_method, "");
    EXPECT_EQ(general_options.m_identifier, "");
    EXPECT_EQ(general_options.m_datafiles.size(), 0);
    EXPECT_EQ(general_options.m_treename, "ntuple");
    EXPECT_EQ(general_options.m_variables.size(), 0);
    EXPECT_EQ(general_options.m_spectators.size(), 0);
    EXPECT_EQ(general_options.m_signal_class, 1);
    EXPECT_EQ(general_options.m_target_variable, "isSignal");
    EXPECT_EQ(general_options.m_weight_variable, "__weight__");
    EXPECT_EQ(general_options.m_max_events, 0u);

    general_options.m_method = "Method";
    general_options.m_identifier = "Weightfile";
    general_options.m_datafiles = {"Datafile"};
    general_options.m_treename = "Tree";
    general_options.m_variables = {"v", "a", "r", "s"};
    general_options.m_spectators = {"x", "M"};
    general_options.m_signal_class = 2;
    general_options.m_max_events = 100;
    general_options.m_target_variable = "Target";
    general_options.m_weight_variable = "Weight";

    boost::property_tree::ptree pt;
    general_options.save(pt);
    EXPECT_EQ(pt.get<std::string>("method"), "Method");
    EXPECT_EQ(pt.get<std::string>("weightfile"), "Weightfile");
    EXPECT_EQ(pt.get<unsigned int>("number_data_files"), 1);
    EXPECT_EQ(pt.get<std::string>("datafile0"), "Datafile");
    EXPECT_EQ(pt.get<std::string>("treename"), "Tree");
    EXPECT_EQ(pt.get<std::string>("target_variable"), "Target");
    EXPECT_EQ(pt.get<std::string>("weight_variable"), "Weight");
    EXPECT_EQ(pt.get<int>("signal_class"), 2);
    EXPECT_EQ(pt.get<unsigned int>("max_events"), 100u);
    EXPECT_EQ(pt.get<unsigned int>("number_feature_variables"), 4);
    EXPECT_EQ(pt.get<std::string>("variable0"), "v");
    EXPECT_EQ(pt.get<std::string>("variable1"), "a");
    EXPECT_EQ(pt.get<std::string>("variable2"), "r");
    EXPECT_EQ(pt.get<std::string>("variable3"), "s");
    EXPECT_EQ(pt.get<unsigned int>("number_spectator_variables"), 2);
    EXPECT_EQ(pt.get<std::string>("spectator0"), "x");
    EXPECT_EQ(pt.get<std::string>("spectator1"), "M");

    MVA::GeneralOptions general_options2;
    general_options2.load(pt);

    EXPECT_EQ(general_options2.m_method, "Method");
    EXPECT_EQ(general_options2.m_identifier, "Weightfile");
    EXPECT_EQ(general_options2.m_datafiles.size(), 1);
    EXPECT_EQ(general_options2.m_datafiles[0], "Datafile");
    EXPECT_EQ(general_options2.m_treename, "Tree");
    EXPECT_EQ(general_options2.m_variables.size(), 4);
    EXPECT_EQ(general_options2.m_variables[0], "v");
    EXPECT_EQ(general_options2.m_variables[1], "a");
    EXPECT_EQ(general_options2.m_variables[2], "r");
    EXPECT_EQ(general_options2.m_variables[3], "s");
    EXPECT_EQ(general_options2.m_spectators.size(), 2);
    EXPECT_EQ(general_options2.m_spectators[0], "x");
    EXPECT_EQ(general_options2.m_spectators[1], "M");
    EXPECT_EQ(general_options2.m_signal_class, 2);
    EXPECT_EQ(general_options2.m_max_events, 100u);
    EXPECT_EQ(general_options2.m_target_variable, "Target");
    EXPECT_EQ(general_options2.m_weight_variable, "Weight");

    // Test if po::options_description is created without crashing
    auto description = general_options.getDescription();
    EXPECT_EQ(description.options().size(), 11);
  }

  TEST(OptionsTest, MetaOptions)
  {
    MVA::MetaOptions meta_options;
    EXPECT_EQ(meta_options.m_use_multiclass, false);
    EXPECT_EQ(meta_options.m_use_hyperparameter, false);
    EXPECT_EQ(meta_options.m_use_splot, false);
    EXPECT_EQ(meta_options.m_hyperparameters.size(), 0);
    EXPECT_EQ(meta_options.m_hyperparameter_metric, "AUC");
    EXPECT_EQ(meta_options.m_splot_variable, "M");
    EXPECT_EQ(meta_options.m_splot_mc_files.size(), 0);
    EXPECT_EQ(meta_options.m_splot_combined, false);
    EXPECT_EQ(meta_options.m_splot_boosted, false);

    meta_options.m_use_multiclass = true;
    meta_options.m_use_hyperparameter = true;
    meta_options.m_use_splot = true;
    meta_options.m_hyperparameters = {"x", "y"};
    meta_options.m_hyperparameter_metric = "SEP";
    meta_options.m_splot_variable = "Q";
    meta_options.m_splot_mc_files = {"mc.root"};
    meta_options.m_splot_combined = true;
    meta_options.m_splot_boosted = true;

    boost::property_tree::ptree pt;
    meta_options.save(pt);
    EXPECT_EQ(pt.get<bool>("use_multiclass"), true);
    EXPECT_EQ(pt.get<bool>("use_hyperparameter"), true);
    EXPECT_EQ(pt.get<bool>("use_splot"), true);
    EXPECT_EQ(pt.get<bool>("splot_combined"), true);
    EXPECT_EQ(pt.get<bool>("splot_boosted"), true);
    EXPECT_EQ(pt.get<std::string>("hyperparameter_metric"), "SEP");
    EXPECT_EQ(pt.get<unsigned int>("number_of_hyperparameters"), 2);
    EXPECT_EQ(pt.get<std::string>("hyperparameter0"), "x");
    EXPECT_EQ(pt.get<std::string>("hyperparameter1"), "y");
    EXPECT_EQ(pt.get<unsigned int>("number_of_mcfiles"), 1);
    EXPECT_EQ(pt.get<std::string>("splot_mc_file0"), "mc.root");
    EXPECT_EQ(pt.get<std::string>("splot_variable"), "Q");

    MVA::MetaOptions meta_options2;
    meta_options2.load(pt);

    EXPECT_EQ(meta_options2.m_use_multiclass, true);
    EXPECT_EQ(meta_options2.m_use_hyperparameter, true);
    EXPECT_EQ(meta_options2.m_use_splot, true);
    EXPECT_EQ(meta_options2.m_hyperparameter_metric, "SEP");
    EXPECT_EQ(meta_options2.m_hyperparameters.size(), 2);
    EXPECT_EQ(meta_options2.m_hyperparameters[0], "x");
    EXPECT_EQ(meta_options2.m_hyperparameters[1], "y");
    EXPECT_EQ(meta_options2.m_splot_variable, "Q");
    EXPECT_EQ(meta_options2.m_splot_mc_files.size(), 1);
    EXPECT_EQ(meta_options2.m_splot_mc_files[0], "mc.root");
    EXPECT_EQ(meta_options2.m_splot_combined, true);
    EXPECT_EQ(meta_options2.m_splot_boosted, true);

    // Test if po::options_description is created without crashing
    auto description = meta_options.getDescription();
    EXPECT_EQ(description.options().size(), 9);

  }

}
