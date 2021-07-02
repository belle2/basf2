/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_FANN_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_FANN_HEADER

#include <mva/interface/Options.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

#include <fann.h>

namespace Belle2 {
  namespace MVA {

    /**
     * Options for the FANN MVA method
     */
    class FANNOptions : public SpecificOptions {

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

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "FANN"; }

      /**
       * Returns the internal vector parameter with the number of hidden neurons per layer
       * @param nf number of features (input nodes).
       */
      std::vector<unsigned int> getHiddenLayerNeurons(unsigned int nf) const;

      unsigned int m_max_epochs = 10000; /**< Maximum number of epochs */
      bool m_verbose_mode = true; /**< Sets to report training status or not*/
      std::string m_hidden_layers_architecture = "3*N"; /**< String containing the architecture of hidden neurons. Ex. "3,3*N,3*(N-1) */

      std::string m_hidden_activiation_function = "FANN_SIGMOID_SYMMETRIC"; /**< Activation function in hidden layer */
      std::string m_output_activiation_function = "FANN_SIGMOID_SYMMETRIC"; /**< Activation function in output layer */
      std::string m_error_function = "FANN_ERRORFUNC_LINEAR"; /**< Loss function */
      std::string m_training_method = "FANN_TRAIN_RPROP"; /**< Training method for back propagation */

      double m_validation_fraction = 0.5; /**< Fraction of training sample used for validation in order to avoid overtraining. */
      unsigned int m_random_seeds =
        3; /**< Number of times the training is repeated with a new weight random seed. The one with the best result is saved. */
      unsigned int m_test_rate =
        500; /**< Error on validation is compared with the one before. The number of epochs before is given by this parameter. */
      unsigned int m_number_of_threads = 8; /**< Number of threads for parallel training. */


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
