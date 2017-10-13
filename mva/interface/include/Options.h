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
#include <boost/program_options/errors.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <string>
#include <vector>
#include <functional>

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
      std::vector<std::string> m_spectators; /**< Vector of all spectators (branch names) used in the training */
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

      bool m_use_splot = false; /**< Use splot training */
      std::string m_splot_variable = "M"; /**< Discriminating variable */
      std::vector<std::string> m_splot_mc_files; /**< Monte carlo files used for the distribution of the discriminating variable */
      bool m_splot_combined = false; /**< Combine sPlot training with PDF classifier for discriminating variable */
      bool m_splot_boosted = false; /**< Use boosted sPlot training (aPlot) */

      bool m_use_sideband_substraction = false; /**< Use sideband substraction */
      std::vector<std::string> m_sideband_mc_files; /**< used to estimate the number of events in the different regions */
      std::string m_sideband_variable =
        ""; /**< Variable defining the signal region (1) background region (2) negative signal region (3) or unused (otherwise) for the sideband substraction */

      bool m_use_reweighting = false; /**< Use a pretraining of data against mc and weight the mc afterwards */
      std::string m_reweighting_identifier = ""; /**< Identifier used to save the reweighting expert */
      std::string m_reweighting_variable =
        ""; /**< Variable defining for which events the reweighting should be used (1) or not used (0). If empty the reweighting is applied to all events */
      std::vector<std::string> m_reweighting_data_files; /**< Data files for the pretraining */
      std::vector<std::string> m_reweighting_mc_files; /**< MC files for the pretraining */
    };

    template<typename T>
    std::function<void(T)> check_bounds(T min, T max, const std::string& name)
    {
      return [name, min, max](T v) -> void {
        if (v <= min || v >= max)
        {
          throw po::validation_error(po::validation_error::invalid_option_value, name,
          std::to_string(min) + " <= " + name + " <= " + std::to_string(max) + ": provided value " + std::to_string(v));
        }
      };
    }

    template<typename T>
    std::function<void(std::vector<T>)> check_bounds_vector(T min, T max, const std::string& name)
    {
      return [name, min, max](const std::vector<T>& vec) -> void {
        for (auto v : vec)
        {
          if (v <= min || v >= max) {
            throw po::validation_error(po::validation_error::invalid_option_value, name,
            std::to_string(min) + " <= " + name + " <= " + std::to_string(max) + ": provided value " + std::to_string(v));
          }
        }
      };
    }

  }
}
#endif
