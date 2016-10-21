/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_OPTIONS_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_OPTIONS_HEADER

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <string>
#include <vector>

namespace po = boost::program_options;

namespace Belle2 {
  namespace MVA {


    /**
     * Abstract base class of all Options given to the MVA interface
     */
    class Options {
    public:
      /**
       * Load mechanism (used by Weightfile) to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) = 0;

      /**
       * Save mechanism (used by Weightfile) to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const = 0;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() = 0;

      /**
       * Virtual default destructor
       */
      virtual ~Options() = default;
    };

    /**
     * General options which are shared by all MVA trainings
     */
    class GeneralOptions : public Options {

    public:
      /**
       * Load mechanism (used by Weightfile) to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism (used by Weightfile) to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      std::string m_method; /**< Name of the MVA method to use */
      std::string m_identifier; /**< Identifier containing the finished training */
      std::vector<std::string> m_datafiles; /**< Name of the datafiles containing the training data */
      std::string m_treename = "ntuple"; /**< Name of the TTree inside the datafile containing the training data */
      std::vector<std::string> m_variables; /**< Vector of all variables (branch names) used in the training */
      int m_signal_class = 1; /**< Signal class which is used as signal in a classification problem */
      std::string m_target_variable = "isSignal"; /**< Target variable (branch name) defining the target */
      std::string m_weight_variable = "__weight__"; /**< Weight variable (branch name) defining the weights */
      unsigned int m_max_events = 0; /**< Maximum number of events to process, 0 means all */
    };

    /**
     * Specific Options, all mehtod Options have to inherit from this class
     */
    class SpecificOptions : public Options {

    public:
      /**
       * Returns method name, used in the interface to register the method
       */
      virtual std::string getMethod() const = 0;

    };

    /**
     * Meta Options which modify the underlying training by doing sPlot, Multiclass and HyperparameterSearch
     */
    class MetaOptions : public Options {

    public:
      /**
       * Load mechanism (used by Weightfile) to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism (used by Weightfile) to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      bool m_use_multiclass = false; /**< Use multi-class training */
      bool m_use_hyperparameter = false; /**< Use hyperparameter training */
      std::string m_hyperparameter_metric = "AUC"; /**< Hyperparameter Metric */
      std::vector<std::string> m_hyperparameters; /**< List of hyper-parameters to change */

      bool m_use_splot = false; /**< Use splot training */
      std::string m_splot_variable = "M"; /**< Discriminating variable */
      std::vector<std::string> m_splot_mc_files; /**< Monte carlo files used for the distribution of the discriminating variable */
      bool m_splot_combined = false; /**< Combine sPlot training with PDF classifier for discriminating variable */
      bool m_splot_boosted = false; /**< Use boosted sPlot training (aPlot) */
    };


  }
}
#endif
