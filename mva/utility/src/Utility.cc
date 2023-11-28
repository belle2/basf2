/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/utility/Utility.h>
#include <mva/utility/DataDriven.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Reweighter.h>
#include <mva/methods/Trivial.h>
#include <mva/methods/Combination.h>

#include <framework/logging/Logger.h>

#include <framework/utilities/MakeROOTCompatible.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <string>
#include <regex>
#include <fstream>

using namespace Belle2::MVA;

void Utility::download(const std::string& identifier, const std::string& filename, int experiment, int run, int event)
{
  Belle2::EventMetaData emd(event, run, experiment);
  Belle2::MVA::Weightfile weightfile = Belle2::MVA::Weightfile::loadFromDatabase(identifier, emd);
  if (boost::ends_with(filename, ".root")) {
    Belle2::MVA::Weightfile::saveToROOTFile(weightfile, filename);
  } else if (boost::ends_with(filename, ".xml")) {
    Belle2::MVA::Weightfile::saveToXMLFile(weightfile, filename);
  } else {
    std::cerr << "Unknown file extension, fallback to xml" << std::endl;
    Belle2::MVA::Weightfile::saveToXMLFile(weightfile, filename);
  }
}

void Utility::upload(const std::string& filename, const std::string& identifier, int exp1, int run1, int exp2, int run2)
{
  Belle2::IntervalOfValidity iov(exp1, run1, exp2, run2);
  Belle2::MVA::Weightfile weightfile;
  if (boost::ends_with(filename, ".root")) {
    weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(filename);
  } else  if (boost::ends_with(filename, ".xml")) {
    weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
  } else {
    std::cerr << "Unknown file extension, fallback to xml" << std::endl;
    weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
  }
  Belle2::MVA::Weightfile::saveToDatabase(weightfile, identifier, iov);
}

void Utility::upload_array(const std::vector<std::string>& filenames, const std::string& identifier, int exp1, int run1, int exp2,
                           int run2)
{
  Belle2::IntervalOfValidity iov(exp1, run1, exp2, run2);

  std::vector<Belle2::MVA::Weightfile> weightfiles;
  for (const auto& filename : filenames) {

    Belle2::MVA::Weightfile weightfile;
    if (boost::ends_with(filename, ".root")) {
      weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(filename);
    } else  if (boost::ends_with(filename, ".xml")) {
      weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
    } else {
      std::cerr << "Unknown file extension, fallback to xml" << std::endl;
      weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
    }
    weightfiles.push_back(weightfile);
  }
  Belle2::MVA::Weightfile::saveArrayToDatabase(weightfiles, identifier, iov);
}

void Utility::extract(const std::string& filename, const std::string& directory)
{

  AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
  auto weightfile = Weightfile::load(filename);
  weightfile.setRemoveTemporaryDirectories(false);
  setenv("TMPDIR", directory.c_str(), 1);
  GeneralOptions general_options;
  weightfile.getOptions(general_options);
  auto expertLocal = supported_interfaces[general_options.m_method]->getExpert();
  expertLocal->load(weightfile);

}

std::string Utility::info(const std::string& filename)
{

  AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
  auto weightfile = Weightfile::load(filename);
  GeneralOptions general_options;
  weightfile.getOptions(general_options);

  auto specific_options = supported_interfaces[general_options.m_method]->getOptions();
  specific_options->load(weightfile.getXMLTree());

  boost::property_tree::ptree temp_tree;
  general_options.save(temp_tree);
  specific_options->save(temp_tree);
  std::ostringstream oss;

#if BOOST_VERSION < 105600
  boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#else
  boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#endif
  boost::property_tree::xml_parser::write_xml(oss, temp_tree, settings);;

  return oss.str();

}

bool Utility::available(const std::string& filename, int experiment, int run, int event)
{

  try {
    auto weightfile = Weightfile::load(filename, Belle2::EventMetaData(event, run, experiment));
    return true;
  } catch (...) {
    return false;
  }

}

void Utility::expert(const std::vector<std::string>& filenames, const std::vector<std::string>& datafiles,
                     const std::string& treename,
                     const std::string& outputfile, int experiment, int run, int event, bool copy_target)
{

  TFile file(outputfile.c_str(), "RECREATE");
  file.cd();
  TTree tree("variables", "variables");

  AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = AbstractInterface::getSupportedInterfaces();

  for (auto& filename : filenames) {

    Belle2::EventMetaData emd(event, run, experiment);
    auto weightfile = Weightfile::load(filename, emd);

    GeneralOptions general_options;
    weightfile.getOptions(general_options);

    general_options.m_treename = treename;
    // Override possible restriction of number of events in training
    // otherwise this would apply to the expert as well.
    general_options.m_max_events = 0;

    auto expertLocal = supported_interfaces[general_options.m_method]->getExpert();
    expertLocal->load(weightfile);

    bool isMulticlass = general_options.m_nClasses > 2;

    // define if target variables should be copied
    if (not copy_target) {
      general_options.m_target_variable = std::string();
    }

    general_options.m_datafiles = datafiles;
    ROOTDataset data(general_options);

    std::vector<TBranch*> branches;
    //create the output branches
    if (not isMulticlass) {
      float result = 0;
      std::string branchname = Belle2::MakeROOTCompatible::makeROOTCompatible(filename);
      branches.push_back(tree.Branch(branchname.c_str(), &result, (branchname + "/F").c_str()));
      std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

      auto results = expertLocal->apply(data);
      std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> application_time = stop - start;
      B2INFO("Elapsed application time in ms " << application_time.count() << " for " << general_options.m_identifier);
      for (auto& r : results) {
        result = r;
        branches[0]->Fill();
      }

    } else {
      float result = 0;
      for (unsigned int iClass = 0; iClass < general_options.m_nClasses; ++iClass) {
        std::string branchname = Belle2::MakeROOTCompatible::makeROOTCompatible(filename + "_" + std::to_string(iClass));
        branches.push_back(tree.Branch(branchname.c_str(), &result, (branchname + "/F").c_str()));
      }
      std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
      auto results = expertLocal->applyMulticlass(data);
      std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> application_time = stop - start;
      B2INFO("Elapsed application time in ms " << application_time.count() << " for " << general_options.m_identifier);
      for (auto& r : results) {
        for (unsigned int iClass = 0; iClass < general_options.m_nClasses; ++iClass) {
          result = r[iClass];
          branches[iClass]->Fill();
        }
      }

    }


    if (not general_options.m_target_variable.empty()) {
      std::string branchname = Belle2::MakeROOTCompatible::makeROOTCompatible(filename + "_" +
                               general_options.m_target_variable);
      float target = 0;
      auto target_branch = tree.Branch(branchname.c_str(), &target, (branchname + "/F").c_str());
      auto targets = data.getTargets();
      for (auto& t : targets) {
        target = t;
        target_branch->Fill();
      }
    }
  }

  tree.SetEntries();
  file.Write("variables");

}

void Utility::save_custom_weightfile(const GeneralOptions& general_options, const SpecificOptions& specific_options,
                                     const std::string& custom_weightfile, const std::string& output_identifier)
{
  std::ifstream ifile(custom_weightfile);
  if (!(bool)ifile) {
    B2FATAL("Input weight file: " << custom_weightfile << " does not exist!");
  }

  Weightfile weightfile;
  weightfile.addOptions(general_options);
  weightfile.addOptions(specific_options);
  weightfile.addFile(general_options.m_identifier + "_Weightfile", custom_weightfile);
  std::string output_weightfile(custom_weightfile);
  if (!output_identifier.empty()) {
    std::regex to_replace("(\\.\\S+$)");
    std::string replacement = "_" + output_identifier + "$0";
    output_weightfile = std::regex_replace(output_weightfile, to_replace, replacement);
  }
  Weightfile::save(weightfile, output_weightfile);
}

void Utility::teacher(const GeneralOptions& general_options, const SpecificOptions& specific_options,
                      const MetaOptions& meta_options)
{
  unsigned int number_of_enabled_meta_trainings = 0;
  if (meta_options.m_use_splot)
    number_of_enabled_meta_trainings++;
  if (meta_options.m_use_sideband_subtraction)
    number_of_enabled_meta_trainings++;
  if (meta_options.m_use_reweighting)
    number_of_enabled_meta_trainings++;

  if (number_of_enabled_meta_trainings > 1) {
    B2ERROR("You enabled more than one meta training option. You can only use one (sPlot, SidebandSubstraction or Reweighting)");
    return;
  }

  if (meta_options.m_use_splot) {
    teacher_splot(general_options, specific_options, meta_options);
  } else if (meta_options.m_use_sideband_subtraction) {
    teacher_sideband_subtraction(general_options, specific_options, meta_options);
  } else if (meta_options.m_use_reweighting) {
    teacher_reweighting(general_options, specific_options, meta_options);
  } else {
    ROOTDataset data(general_options);
    teacher_dataset(general_options, specific_options, data);
  }
}


std::unique_ptr<Belle2::MVA::Expert> Utility::teacher_dataset(GeneralOptions general_options,
    const SpecificOptions& specific_options,
    Dataset& data)
{
  if (general_options.m_method.empty()) {
    general_options.m_method = specific_options.getMethod();
  } else {
    if (general_options.m_method != specific_options.getMethod()) {
      B2ERROR("The method specified in the general options is in conflict with the provided specific option:" << general_options.m_method
              << " " << specific_options.getMethod());
    }
  }
  AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
  if (supported_interfaces.find(general_options.m_method) != supported_interfaces.end()) {
    auto teacherLocal = supported_interfaces[general_options.m_method]->getTeacher(general_options, specific_options);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto weightfile = teacherLocal->train(data);
    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> training_time = stop - start;
    B2INFO("Elapsed training time in ms " << training_time.count() << " for " << general_options.m_identifier);
    Weightfile::save(weightfile, general_options.m_identifier);
    auto expertLocal = supported_interfaces[general_options.m_method]->getExpert();
    expertLocal->load(weightfile);
    return expertLocal;
  } else {
    B2ERROR("Interface doesn't support chosen method" << general_options.m_method);
    throw std::runtime_error("Interface doesn't support chosen method" + general_options.m_method);
  }
}

std::unique_ptr<Belle2::MVA::Expert> Utility::teacher_splot(const GeneralOptions& general_options,
                                                            const SpecificOptions& specific_options,
                                                            const MetaOptions& meta_options)
{

  GeneralOptions data_general_options = general_options;
  data_general_options.m_target_variable = "";
  if (meta_options.m_splot_combined)
    data_general_options.m_identifier = general_options.m_identifier + "_splot.xml";
  ROOTDataset data_dataset(data_general_options);
  // Reset target variable so that it shows up in the weightfile at the end
  data_general_options.m_target_variable = general_options.m_target_variable;

  GeneralOptions discriminant_general_options = general_options;
  discriminant_general_options.m_target_variable = "";
  discriminant_general_options.m_variables = {meta_options.m_splot_variable};
  ROOTDataset discriminant_dataset(discriminant_general_options);
  // Reset target variable so that it shows up in the weightfile at the end
  discriminant_general_options.m_target_variable = general_options.m_target_variable;

  GeneralOptions mc_general_options = general_options;
  mc_general_options.m_datafiles = meta_options.m_splot_mc_files;
  mc_general_options.m_variables = {meta_options.m_splot_variable};
  ROOTDataset mc_dataset(mc_general_options);

  auto mc_signals = mc_dataset.getSignals();
  auto mc_weights = mc_dataset.getWeights();
  auto mc_feature = mc_dataset.getFeature(0);
  auto data_feature = discriminant_dataset.getFeature(0);
  auto data_weights = discriminant_dataset.getWeights();

  Binning binning = Binning::CreateEqualFrequency(mc_feature, mc_weights, mc_signals, 100);

  float signalFraction = binning.m_signal_yield / (binning.m_signal_yield + binning.m_bckgrd_yield);

  std::vector<double> data(100, 0);
  double total_data = 0.0;
  for (unsigned int iEvent = 0; iEvent < data_dataset.getNumberOfEvents(); ++iEvent) {
    data[binning.getBin(data_feature[iEvent])] += data_weights[iEvent];
    total_data += data_weights[iEvent];
  }

  // We do a simple fit here to estimate the signal and background yields
  // We could use RooFit here to avoid using custom code,
  // but I found RooFit to be difficult and unstable ...

  float best_yield = 0.0;
  double best_chi2 = 1000000000.0;
  bool empty_bin = false;
  for (double yield = 0; yield < total_data; yield += 1) {
    double chi2 = 0.0;
    for (unsigned int iBin = 0; iBin < 100; ++iBin) {
      double deviation = (data[iBin] - (yield * binning.m_signal_pdf[iBin] + (total_data - yield) * binning.m_bckgrd_pdf[iBin]) *
                          (binning.m_boundaries[iBin + 1] - binning.m_boundaries[iBin]) / (binning.m_boundaries[100] - binning.m_boundaries[0]));
      if (data[iBin] > 0)
        chi2 += deviation * deviation / data[iBin];
      else
        empty_bin = true;
    }
    if (chi2 < best_chi2) {
      best_chi2 = chi2;
      best_yield = yield;
    }
  }

  if (empty_bin) {
    B2WARNING("Encountered empty bin in data histogram during fit of the components for sPlot");
  }

  B2INFO("sPlot best yield " << best_yield);
  B2INFO("sPlot Yields On MC " << binning.m_signal_yield << " " << binning.m_bckgrd_yield);

  binning.m_signal_yield = best_yield;
  binning.m_bckgrd_yield = (total_data - best_yield);

  B2INFO("sPlot Yields Fitted On Data " << binning.m_signal_yield << " " << binning.m_bckgrd_yield);

  if (meta_options.m_splot_boosted) {
    GeneralOptions boost_general_options = data_general_options;
    boost_general_options.m_identifier = general_options.m_identifier + "_boost.xml";
    SPlotDataset splot_dataset(boost_general_options, data_dataset, getBoostWeights(discriminant_dataset, binning), signalFraction);
    auto boost_expert = teacher_dataset(boost_general_options, specific_options, splot_dataset);

    SPlotDataset aplot_dataset(data_general_options, data_dataset, getAPlotWeights(discriminant_dataset, binning,
                               boost_expert->apply(data_dataset)), signalFraction);
    auto splot_expert = teacher_dataset(data_general_options, specific_options, aplot_dataset);
    if (not meta_options.m_splot_combined)
      return splot_expert;
  } else {
    SPlotDataset splot_dataset(data_general_options, data_dataset, getSPlotWeights(discriminant_dataset, binning), signalFraction);
    auto splot_expert = teacher_dataset(data_general_options, specific_options, splot_dataset);
    if (not meta_options.m_splot_combined)
      return splot_expert;
  }

  mc_general_options.m_identifier = general_options.m_identifier + "_pdf.xml";
  mc_general_options.m_method = "PDF";
  PDFOptions pdf_options;
  // cppcheck-suppress unreadVariable
  auto pdf_expert = teacher_dataset(mc_general_options, pdf_options, mc_dataset);

  GeneralOptions combination_general_options = general_options;
  combination_general_options.m_method = "Combination";
  combination_general_options.m_variables.push_back(meta_options.m_splot_variable);
  CombinationOptions combination_options;
  combination_options.m_weightfiles = {data_general_options.m_identifier, mc_general_options.m_identifier};
  auto combination_expert = teacher_dataset(combination_general_options, combination_options, data_dataset);

  return combination_expert;
}

std::unique_ptr<Belle2::MVA::Expert> Utility::teacher_reweighting(const GeneralOptions& general_options,
    const SpecificOptions& specific_options,
    const MetaOptions& meta_options)
{
  if (std::find(general_options.m_variables.begin(), general_options.m_variables.end(),
                meta_options.m_reweighting_variable) != general_options.m_variables.end()) {
    B2ERROR("You cannot use the reweighting variable as a feature in your training");
    return nullptr;
  }

  GeneralOptions data_general_options = general_options;
  data_general_options.m_target_variable = "";
  data_general_options.m_datafiles = meta_options.m_reweighting_data_files;
  ROOTDataset data_dataset(data_general_options);

  GeneralOptions mc_general_options = general_options;
  mc_general_options.m_datafiles = meta_options.m_reweighting_mc_files;
  ROOTDataset mc_dataset(mc_general_options);

  CombinedDataset boost_dataset(general_options, data_dataset, mc_dataset);

  GeneralOptions boost_general_options = general_options;
  boost_general_options.m_identifier = general_options.m_identifier + "_boost.xml";
  // cppcheck-suppress unreadVariable
  auto boost_expert = teacher_dataset(boost_general_options, specific_options, boost_dataset);

  GeneralOptions reweighter_general_options = general_options;
  reweighter_general_options.m_identifier = meta_options.m_reweighting_identifier;
  reweighter_general_options.m_method = "Reweighter";
  ReweighterOptions reweighter_specific_options;
  reweighter_specific_options.m_weightfile = boost_general_options.m_identifier;
  reweighter_specific_options.m_variable = meta_options.m_reweighting_variable;

  if (meta_options.m_reweighting_variable != "") {
    if (std::find(reweighter_general_options.m_spectators.begin(), reweighter_general_options.m_spectators.end(),
                  meta_options.m_reweighting_variable) == reweighter_general_options.m_spectators.end() and
        std::find(reweighter_general_options.m_variables.begin(), reweighter_general_options.m_variables.end(),
                  meta_options.m_reweighting_variable) == reweighter_general_options.m_variables.end() and
        reweighter_general_options.m_target_variable != meta_options.m_reweighting_variable and
        reweighter_general_options.m_weight_variable != meta_options.m_reweighting_variable) {
      reweighter_general_options.m_spectators.push_back(meta_options.m_reweighting_variable);
    }
  }

  ROOTDataset dataset(reweighter_general_options);
  auto reweight_expert = teacher_dataset(reweighter_general_options, reweighter_specific_options, dataset);
  auto weights = reweight_expert->apply(dataset);
  ReweightingDataset reweighted_dataset(general_options, dataset, weights);
  auto expertLocal = teacher_dataset(general_options, specific_options, reweighted_dataset);

  return expertLocal;
}

std::unique_ptr<Belle2::MVA::Expert> Utility::teacher_sideband_subtraction(const GeneralOptions& general_options,
    const SpecificOptions& specific_options,
    const MetaOptions& meta_options)
{

  if (std::find(general_options.m_variables.begin(), general_options.m_variables.end(),
                meta_options.m_sideband_variable) != general_options.m_variables.end()) {
    B2ERROR("You cannot use the sideband variable as a feature in your training");
    return nullptr;
  }

  GeneralOptions data_general_options = general_options;
  if (std::find(data_general_options.m_spectators.begin(), data_general_options.m_spectators.end(),
                meta_options.m_sideband_variable) == data_general_options.m_spectators.end()) {
    data_general_options.m_spectators.push_back(meta_options.m_sideband_variable);
  }
  ROOTDataset data_dataset(data_general_options);

  GeneralOptions mc_general_options = general_options;
  mc_general_options.m_datafiles = meta_options.m_sideband_mc_files;
  if (std::find(mc_general_options.m_spectators.begin(), mc_general_options.m_spectators.end(),
                meta_options.m_sideband_variable) == mc_general_options.m_spectators.end()) {
    mc_general_options.m_spectators.push_back(meta_options.m_sideband_variable);
  }
  ROOTDataset mc_dataset(mc_general_options);

  GeneralOptions sideband_general_options = general_options;
  SidebandDataset sideband_dataset(sideband_general_options, data_dataset, mc_dataset, meta_options.m_sideband_variable);
  auto expertLocal = teacher_dataset(general_options, specific_options, sideband_dataset);

  return expertLocal;
}
