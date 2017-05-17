/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <mva/interface/Options.h>
#include <boost/property_tree/ptree.hpp>

namespace Belle2 {
  namespace MVA {

    po::options_description GeneralOptions::getDescription()
    {
      po::options_description description("General options");
      description.add_options()
      ("help", "print this message")
      ("datafiles", po::value<std::vector<std::string>>(&m_datafiles)->required()->multitoken(),
       "ROOT files containing the training dataset")
      ("treename", po::value<std::string>(&m_treename), "Name of tree in ROOT datafile")
      ("identifier", po::value<std::string>(&m_identifier)->required(), "Identifier of the outputted weightfile")
      ("variables", po::value<std::vector<std::string>>(&m_variables)->required()->multitoken(),
       "feature variables used in the training")
      ("spectators", po::value<std::vector<std::string>>(&m_spectators)->multitoken(),
       "spectator variables used in the training")
      ("target_variable", po::value<std::string>(&m_target_variable),
       "target variable used to distinguish between signal and background, isSignal is used as default.")
      ("signal_class", po::value<int>(&m_signal_class), "integer which identifies signal events")
      ("weight_variable", po::value<std::string>(&m_weight_variable), "weight variable used to weight each event")
      ("max_events", po::value<unsigned int>(&m_max_events), "maximum number of events to process, 0 means all")
      ("method", po::value<std::string>(&m_method)->required(),
       "MVA Method [FastBDT|TMVAClassification|TMVARegression|Python|FANN|]");
      return description;
    }

    void GeneralOptions::load(const boost::property_tree::ptree& pt)
    {
      m_method = pt.get<std::string>("method");
      m_identifier = pt.get<std::string>("weightfile");
      m_treename = pt.get<std::string>("treename");
      m_target_variable = pt.get<std::string>("target_variable");
      m_weight_variable = pt.get<std::string>("weight_variable");
      m_signal_class = pt.get<int>("signal_class");
      m_max_events = pt.get<unsigned int>("max_events", 0u);

      unsigned int numberOfFiles = pt.get<unsigned int>("number_data_files", 0);
      m_datafiles.resize(numberOfFiles);
      for (unsigned int i = 0; i < numberOfFiles; ++i) {
        m_datafiles[i] = pt.get<std::string>(std::string("datafile") + std::to_string(i));
      }

      unsigned int numberOfSpectators = pt.get<unsigned int>("number_spectator_variables", 0u);
      m_spectators.resize(numberOfSpectators);
      for (unsigned int i = 0; i < numberOfSpectators; ++i) {
        m_spectators[i] = pt.get<std::string>(std::string("spectator") + std::to_string(i));
      }

      unsigned int numberOfFeatures = pt.get<unsigned int>("number_feature_variables");
      m_variables.resize(numberOfFeatures);
      for (unsigned int i = 0; i < numberOfFeatures; ++i) {
        m_variables[i] = pt.get<std::string>(std::string("variable") + std::to_string(i));
      }
    }

    void GeneralOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("method", m_method);
      pt.put("weightfile", m_identifier);
      pt.put("treename", m_treename);
      pt.put("target_variable", m_target_variable);
      pt.put("weight_variable", m_weight_variable);
      pt.put("signal_class", m_signal_class);
      pt.put("max_events", m_max_events);

      pt.put("number_feature_variables", m_variables.size());
      for (unsigned int i = 0; i < m_variables.size(); ++i) {
        pt.put(std::string("variable") + std::to_string(i), m_variables[i]);
      }

      pt.put("number_spectator_variables", m_spectators.size());
      for (unsigned int i = 0; i < m_spectators.size(); ++i) {
        pt.put(std::string("spectator") + std::to_string(i), m_spectators[i]);
      }

      pt.put("number_data_files", m_datafiles.size());
      for (unsigned int i = 0; i < m_datafiles.size(); ++i) {
        pt.put(std::string("datafile") + std::to_string(i), m_datafiles[i]);
      }
    }

    po::options_description MetaOptions::getDescription()
    {
      po::options_description description("Meta options");
      description.add_options()
      ("use_multiclass", po::value<bool>(&m_use_multiclass), "whether to do multiclass training (Not implemented)")
      ("use_splot", po::value<bool>(&m_use_splot), "whether to do an splot training")
      ("splot_variable", po::value<std::string>(&m_splot_variable), "Variable used as discriminating variable in sPlot training")
      ("splot_mc_files", po::value<std::vector<std::string>>(&m_splot_mc_files)->multitoken(),
       "Monte carlo files containing the discriminant variable with the mc truth")
      ("splot_combined", po::value<bool>(&m_splot_combined), "Combine sPlot training with PDF classifier for discriminating variable")
      ("splot_boosted", po::value<bool>(&m_splot_boosted), "Use boosted sPlot training (aPlot)")
      ("use_hyperparameter", po::value<bool>(&m_use_hyperparameter), "whether to do hyperparameter search (not implementend)")
      ("hyperparameter_metric", po::value<std::string>(&m_hyperparameter_metric), "Hyperparameter metric (not implemented)")
      ("hyperparameters", po::value<std::vector<std::string>>(&m_hyperparameters)->multitoken(),
       "Hyperparameters of the chosen option which should be searched (not implemented)");
      return description;
    }

    void MetaOptions::load(const boost::property_tree::ptree& pt)
    {
      m_use_multiclass = pt.get<bool>("use_multiclass");
      m_use_hyperparameter = pt.get<bool>("use_hyperparameter");
      m_hyperparameter_metric = pt.get<std::string>("hyperparameter_metric");
      m_use_splot = pt.get<bool>("use_splot");
      m_splot_combined = pt.get<bool>("splot_combined");
      m_splot_boosted = pt.get<bool>("splot_boosted");
      m_splot_variable = pt.get<std::string>("splot_variable");

      unsigned int numberOfMCFiles = pt.get<unsigned int>("number_of_mcfiles");
      m_splot_mc_files.resize(numberOfMCFiles);
      for (unsigned int i = 0; i < numberOfMCFiles; ++i) {
        m_splot_mc_files[i] = pt.get<std::string>(std::string("splot_mc_file") + std::to_string(i));
      }

      unsigned int numberOfHyperparameters = pt.get<unsigned int>("number_of_hyperparameters");
      m_hyperparameters.resize(numberOfHyperparameters);
      for (unsigned int i = 0; i < numberOfHyperparameters; ++i) {
        m_hyperparameters[i] = pt.get<std::string>(std::string("hyperparameter") + std::to_string(i));
      }
    }

    void MetaOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("use_multiclass", m_use_multiclass);
      pt.put("use_hyperparameter", m_use_hyperparameter);
      pt.put("hyperparameter_metric", m_hyperparameter_metric);
      pt.put("use_splot", m_use_splot);
      pt.put("splot_variable", m_splot_variable);
      pt.put("splot_combined", m_splot_combined);
      pt.put("splot_boosted", m_splot_boosted);

      pt.put("number_of_mcfiles", m_splot_mc_files.size());
      for (unsigned int i = 0; i < m_splot_mc_files.size(); ++i) {
        pt.put(std::string("splot_mc_file") + std::to_string(i), m_splot_mc_files[i]);
      }

      pt.put("number_of_hyperparameters", m_hyperparameters.size());
      for (unsigned int i = 0; i < m_hyperparameters.size(); ++i) {
        pt.put(std::string("hyperparameter") + std::to_string(i), m_hyperparameters[i]);
      }
    }

  }
}
