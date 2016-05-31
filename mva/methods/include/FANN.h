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
#ifndef INCLUDE_GUARD_BELLE2_MVA_FANN_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_FANN_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

#include <fann.h>
#include "floatfann.h"

namespace Belle2 {
  namespace MVA {

    /**
     * Options for the FANN MVA method
     */
    class FANNOptions : public Options {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override;

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override;

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override;

      unsigned int m_max_epochs = 1000; /**< Maximum number of epochs */
      unsigned int m_epochs_between_reports = 100; /**< Epochs between each report of training status */
      std::vector<unsigned int> m_hidden_layer_neurons; /**< Number of hidden neurons per layer */
      double m_desired_error = 0.100; /**< Desired error */
      std::string m_hidden_activiation_function = "FANN_SIGMOID_SYMMETRIC"; /**< Activation function in hidden layer */
      std::string m_output_activiation_function = "FANN_SIGMOID_SYMMETRIC"; /**< Activation function in output layer */
      std::string m_error_function = "FANN_ERRORFUNC_LINEAR"; /**< Loss function */
      std::string m_training_method = "FANN_TRAIN_SARPROP"; /**< Training method for back propagation */
      bool m_scale_features = true; /**< Scale features before training */
      bool m_scale_target = true; /**< Scale target before training */

    };

    /**
     * Teacher for the FANN MVA method
     */
    class FANNTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       * @param general_options defining all shared options
       * @param specific_options defininf all method specific options
       */
      FANNTeacher(const GeneralOptions& general_options, const FANNOptions& specific_options);

      /**
       * Train a mva method using the given dataset returning a Weightfile
       * @param training_data used to train the method
       */
      virtual Weightfile train(Dataset& training_data) const override;

    private:
      FANNOptions m_specific_options; /**< Method specific options */
    };


    /**
     * Expert for the FANN MVA method
     */
    class FANNExpert : public MVA::Expert {

    public:

      /**
       * Destructor of FANN Expert
       */
      virtual ~FANNExpert();

      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this expert onto a dataset
       * @param test_data dataset
       */
      virtual std::vector<float> apply(Dataset& test_data) const override;

    private:
      FANNOptions m_specific_options; /**< Method specific options */
      struct fann* m_ann = nullptr; /**< Pointer to FANN expert */
    };

  }
}
#endif
