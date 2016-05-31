/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/Utility.h>
#include <mva/utility/SPlot.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Trivial.h>
#include <mva/methods/Combination.h>

#include <mva/interface/Interface.h>
#include <mva/interface/Options.h>
#include <mva/interface/Weightfile.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/logging/Logger.h>

#include <analysis/VariableManager/Utility.h>

#include <boost/algorithm/string/predicate.hpp>

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cerrno>
#include <memory>

namespace Belle2 {
  namespace MVA {

    void download(const std::string& identifier, const std::string& filename, int experiment, int run, int event)
    {
      Belle2::EventMetaData emd(event, run, experiment);
      Belle2::MVA::Weightfile weightfile = Belle2::MVA::Weightfile::loadFromDatabase(identifier, emd);
      if (boost::ends_with(filename, ".root")) {
        Belle2::MVA::Weightfile::saveToROOTFile(weightfile, filename);
      } else if (boost::ends_with(filename, ".xml")) {
        Belle2::MVA::Weightfile::saveToXMLFile(weightfile, filename);
      } else {
        std::cerr << "Unkown file extension, fallback to xml" << std::endl;
        Belle2::MVA::Weightfile::saveToXMLFile(weightfile, filename);
      }
    }

    void upload(const std::string& filename, const std::string& identifier, int exp1, int run1, int exp2, int run2)
    {
      Belle2::IntervalOfValidity iov(exp1, run1, exp2, run2);
      Belle2::MVA::Weightfile weightfile;
      if (boost::ends_with(filename, ".root")) {
        weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(filename);
      } else  if (boost::ends_with(filename, ".xml")) {
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
      } else {
        std::cerr << "Unkown file extension, fallback to xml" << std::endl;
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(filename);
      }
      Belle2::MVA::Weightfile::saveToDatabase(weightfile, identifier, iov);
    }

    void expert(const std::vector<std::string>& filenames, const std::string& datafile, const std::string& treename,
                const std::string& outputfile, int experiment, int run, int event)
    {

      std::vector<Weightfile> weightfiles;
      std::vector<TBranch*> branches;

      TFile file(outputfile.c_str(), "RECREATE");
      file.cd();
      TTree tree("variables", "variables");
      float result = 0;

      for (auto& filename : filenames) {
        Belle2::EventMetaData emd(event, run, experiment);
        auto weightfile = Weightfile::load(filename, emd);
        weightfiles.push_back(weightfile);
        std::string branchname = Belle2::Variable::makeROOTCompatible(filename);
        auto branch = tree.Branch(branchname.c_str(), &result, (branchname + "/F").c_str());
        branches.push_back(branch);
      }

      std::vector<std::string> known_targets;
      AbstractInterface::initSupportedInterfaces();
      auto supported_interfaces = AbstractInterface::getSupportedInterfaces();

      for (unsigned int i = 0; i < weightfiles.size(); ++i) {
        auto& weightfile = weightfiles[i];
        auto& branch = branches[i];

        GeneralOptions general_options;
        weightfile.getOptions(general_options);
        general_options.m_datafile = datafile;
        general_options.m_treename = treename;

        auto expert = supported_interfaces[general_options.m_method]->getExpert();
        expert->load(weightfile);

        ROOTDataset data(general_options);
        auto results = expert->apply(data);
        for (auto& r : results) {
          result = r;
          branch->Fill();
        }

        std::string branchname = Belle2::Variable::makeROOTCompatible(general_options.m_target_variable);

        if (std::find(known_targets.begin(), known_targets.end(), branchname) == known_targets.end() and not branchname.empty()) {
          known_targets.push_back(branchname);
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

    void teacher(const GeneralOptions& general_options, const Options& specific_options, const MetaOptions& meta_options)
    {
      if (not meta_options.m_use_splot) {
        ROOTDataset data(general_options);
        teacher_dataset(general_options, specific_options, data);
      } else {
        teacher_splot(general_options, specific_options, meta_options);
      }
    }


    std::unique_ptr<Belle2::MVA::Expert> teacher_dataset(const GeneralOptions& general_options, const Options& specific_options,
                                                         Dataset& data)
    {
      AbstractInterface::initSupportedInterfaces();
      auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
      if (supported_interfaces.find(general_options.m_method) != supported_interfaces.end()) {
        auto teacher = supported_interfaces[general_options.m_method]->getTeacher(general_options, specific_options);
        auto weightfile = teacher->train(data);
        Weightfile::save(weightfile, general_options.m_weightfile);
        auto expert = supported_interfaces[general_options.m_method]->getExpert();
        expert->load(weightfile);
        return expert;
      } else {
        B2ERROR("Interface doesn't support chosen method" << general_options.m_method);
        throw std::runtime_error("Interface doesn't support chosen method" + general_options.m_method);
      }
    }

    std::unique_ptr<Belle2::MVA::Expert> teacher_splot(const GeneralOptions& general_options, const Options& specific_options,
                                                       const MetaOptions& meta_options)
    {

      GeneralOptions data_general_options = general_options;
      data_general_options.m_target_variable = "";
      if (meta_options.m_splot_combined)
        data_general_options.m_weightfile = general_options.m_weightfile + "_splot.xml";
      ROOTDataset data_dataset(data_general_options);

      GeneralOptions discriminant_general_options = general_options;
      discriminant_general_options.m_target_variable = "";
      discriminant_general_options.m_variables = {meta_options.m_splot_variable};
      ROOTDataset discriminant_dataset(discriminant_general_options);

      GeneralOptions mc_general_options = general_options;
      mc_general_options.m_datafile = meta_options.m_splot_mc_file;
      mc_general_options.m_variables = {meta_options.m_splot_variable};
      ROOTDataset mc_dataset(mc_general_options);

      Binning binning = Binning::CreateEqualFrequency(mc_dataset.getFeature(0) , mc_dataset.getWeights(), mc_dataset.getSignals(), 100);

      float signalFraction = binning.m_signal_yield / (binning.m_signal_yield + binning.m_bckgrd_yield);

      // Overall normalization could be different on data
      float yield_correction = data_dataset.getNumberOfEvents() / mc_dataset.getNumberOfEvents();
      binning.m_signal_yield *= yield_correction;
      binning.m_bckgrd_yield *= yield_correction;

      if (meta_options.m_splot_boosted) {
        GeneralOptions boost_general_options = data_general_options;
        boost_general_options.m_weightfile = general_options.m_weightfile + "_boost.xml";
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

      mc_general_options.m_weightfile = general_options.m_weightfile + "_pdf.xml";
      mc_general_options.m_method = "PDF";
      PDFOptions pdf_options;
      auto pdf_expert = teacher_dataset(mc_general_options, pdf_options, mc_dataset);

      GeneralOptions combination_general_options = general_options;
      combination_general_options.m_method = "Combination";
      combination_general_options.m_variables.push_back(meta_options.m_splot_variable);
      CombinationOptions combination_options;
      combination_options.m_weightfiles = {data_general_options.m_weightfile, mc_general_options.m_weightfile};
      auto combination_expert = teacher_dataset(combination_general_options, combination_options, data_dataset);

      return combination_expert;
    }


  }
}
