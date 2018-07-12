/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck and Fernando Abudinen                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/FANN.h>

#include <framework/logging/Logger.h>
#include <TFormula.h>

#ifdef HAS_OPENMP
#include <parallel_fann.hpp>
#else
#include <fann.h>
#endif

namespace Belle2 {
  namespace MVA {

    FANNTeacher::FANNTeacher(const GeneralOptions& general_options, const FANNOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }


    Weightfile FANNTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      std::vector<unsigned int> hiddenLayers = m_specific_options.getHiddenLayerNeurons(numberOfFeatures);
      unsigned int number_of_layers = 2;
      for (unsigned int i = 0; i < hiddenLayers.size(); ++i) {
        if (hiddenLayers[i] > 0) {
          number_of_layers++;
        }
      }

      auto layers = std::unique_ptr<unsigned int[]>(new unsigned int[number_of_layers]);
      layers[0] = numberOfFeatures;
      for (unsigned int i = 0; i < hiddenLayers.size(); ++i) {
        if (hiddenLayers[i] > 0) {
          layers[i + 1] = hiddenLayers[i];
        }
      }
      layers[number_of_layers - 1] = 1;

      struct fann* ann = fann_create_standard_array(number_of_layers, layers.get());

      std::map<std::string, enum fann_activationfunc_enum> activationFunctions;
      unsigned int i = 0;
      for (auto& name : FANN_ACTIVATIONFUNC_NAMES) {
        activationFunctions[name] = fann_activationfunc_enum(i);
        i++;
      }

#ifdef HAS_OPENMP
      typedef float (*FnPtr)(struct fann * ann, struct fann_train_data * data, const unsigned int threadnumb);
      std::map<std::string, FnPtr> trainingMethods;
      trainingMethods["FANN_TRAIN_RPROP"] = parallel_fann::train_epoch_irpropm_parallel;
      trainingMethods["FANN_TRAIN_BATCH"]      = parallel_fann::train_epoch_batch_parallel;
      trainingMethods["FANN_TRAIN_QUICKPROP"] = parallel_fann::train_epoch_quickprop_parallel;
      trainingMethods["FANN_TRAIN_SARPROP"]  = parallel_fann::train_epoch_sarprop_parallel;
      trainingMethods["FANN_TRAIN_INCREMENTAL"] = nullptr;
#else
      std::map<std::string, enum fann_train_enum> trainingMethods;
      i = 0;
      for (auto& name : FANN_TRAIN_NAMES) {
        trainingMethods[name] = fann_train_enum(i);
        i++;
      }
#endif

      std::map<std::string, enum fann_errorfunc_enum> errorFunctions;
      i = 0;
      for (auto& name : FANN_ERRORFUNC_NAMES) {
        errorFunctions[name] = fann_errorfunc_enum(i);
        i++;
      }

      if (activationFunctions.find(m_specific_options.m_hidden_activiation_function) == activationFunctions.end()) {
        B2ERROR("Coulnd't find activation function named " << m_specific_options.m_hidden_activiation_function);
        throw std::runtime_error("Coulnd't find activation function named " + m_specific_options.m_hidden_activiation_function);
      }

      if (activationFunctions.find(m_specific_options.m_output_activiation_function) == activationFunctions.end()) {
        B2ERROR("Coulnd't find activation function named " << m_specific_options.m_output_activiation_function);
        throw std::runtime_error("Coulnd't find activation function named " + m_specific_options.m_output_activiation_function);
      }

      if (errorFunctions.find(m_specific_options.m_error_function) == errorFunctions.end()) {
        B2ERROR("Coulnd't find training method function named " << m_specific_options.m_error_function);
        throw std::runtime_error("Coulnd't find training method function named " + m_specific_options.m_error_function);
      }

      if (trainingMethods.find(m_specific_options.m_training_method) == trainingMethods.end()) {
        B2ERROR("Coulnd't find training method function named " << m_specific_options.m_training_method);
        throw std::runtime_error("Coulnd't find training method function named " + m_specific_options.m_training_method);
      }

      if (m_specific_options.m_max_epochs < 1) {
        B2ERROR("m_max_epochs should be larger than 0 " << m_specific_options.m_max_epochs);
        throw std::runtime_error("m_max_epochs should be larger than 0. The given value is " + std::to_string(
                                   m_specific_options.m_max_epochs));
      }

      if (m_specific_options.m_random_seeds < 1) {
        B2ERROR("m_random_seeds should be larger than 0 " << m_specific_options.m_random_seeds);
        throw std::runtime_error("m_random_seeds should be larger than 0. The given value is " + std::to_string(
                                   m_specific_options.m_random_seeds));
      }

      if (m_specific_options.m_test_rate < 1) {
        B2ERROR("m_test_rate should be larger than 0 " << m_specific_options.m_test_rate);
        throw std::runtime_error("m_test_rate should be larger than 0. The given value is " + std::to_string(
                                   m_specific_options.m_test_rate));
      }

      if (m_specific_options.m_number_of_threads < 1) {
        B2ERROR("m_number_of_threads should be larger than 0. The given value is " << m_specific_options.m_number_of_threads);
        throw std::runtime_error("m_number_of_threads should be larger than 0. The given value is " +
                                 std::to_string(m_specific_options.m_number_of_threads));
      }

      // set network parameters
      fann_set_activation_function_hidden(ann, activationFunctions[m_specific_options.m_hidden_activiation_function]);
      fann_set_activation_function_output(ann, activationFunctions[m_specific_options.m_output_activiation_function]);
      fann_set_train_error_function(ann, errorFunctions[m_specific_options.m_error_function]);


      double nTestingAndValidationEvents = numberOfEvents * m_specific_options.m_validation_fraction;
      unsigned int nTestingEvents = int(nTestingAndValidationEvents * 0.5); // Number of events in the test sample.
      unsigned int nValidationEvents = int(nTestingAndValidationEvents * 0.5);
      unsigned int nTrainingEvents = numberOfEvents - nValidationEvents - nTestingEvents;

      if (nTestingAndValidationEvents < 1) {
        B2ERROR("m_validation_fraction should be a number between 0 and 1 (0 < x < 1). The given value is " <<
                m_specific_options.m_validation_fraction <<
                ". The total number of events is " << numberOfEvents << ". numberOfEvents * m_validation_fraction has to be larger than one");
        throw std::runtime_error("m_validation_fraction should be a number between 0 and 1 (0 < x < 1). numberOfEvents * m_validation_fraction has to be larger than one");
      }

      if (nTrainingEvents < 1) {
        B2ERROR("m_validation_fraction should be a number between 0 and 1 (0 < x < 1). The given value is " <<
                m_specific_options.m_validation_fraction <<
                ". The total number of events is " << numberOfEvents << ". numberOfEvents * (1 - m_validation_fraction) has to be larger than one");
        throw std::runtime_error("m_validation_fraction should be a number between 0 and 1 (0 < x < 1). numberOfEvents * (1 - m_validation_fraction) has to be larger than one");
      }

      // training set
      struct fann_train_data* train_data =
        fann_create_train(nTrainingEvents, numberOfFeatures, 1);
      for (unsigned iEvent = 0; iEvent < nTrainingEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        for (unsigned iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          train_data->input[iEvent][iFeature] = training_data.m_input[iFeature];
        }
        train_data->output[iEvent][0] = training_data.m_target;
      }
      // validation set
      struct fann_train_data* valid_data =
        fann_create_train(nValidationEvents, numberOfFeatures, 1);
      for (unsigned iEvent = nTrainingEvents; iEvent < nTrainingEvents + nValidationEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        for (unsigned iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          valid_data->input[iEvent - nTrainingEvents][iFeature] = training_data.m_input[iFeature];
        }
        valid_data->output[iEvent - nTrainingEvents][0] = training_data.m_target;
      }

      // testing set
      struct fann_train_data* test_data =
        fann_create_train(nTestingEvents, numberOfFeatures, 1);
      for (unsigned iEvent = nTrainingEvents + nValidationEvents; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        for (unsigned iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          test_data->input[iEvent - nTrainingEvents - nValidationEvents][iFeature] = training_data.m_input[iFeature];
        }
        test_data->output[iEvent - nTrainingEvents - nValidationEvents][0] = training_data.m_target;
      }

      struct fann_train_data* data = fann_create_train(numberOfEvents, numberOfFeatures, 1);
      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          data->input[iEvent][iFeature] = training_data.m_input[iFeature];
        }
        data->output[iEvent][0] = training_data.m_target;
      }

      if (m_specific_options.m_scale_features) {
        fann_set_input_scaling_params(ann, data, -1.0, 1.0);
      }

      if (m_specific_options.m_scale_target) {
        fann_set_output_scaling_params(ann, data, -1.0, 1.0);
      }

      if (m_specific_options.m_scale_features or m_specific_options.m_scale_target) {
        fann_scale_train(ann, data);
        fann_scale_train(ann, train_data);
        fann_scale_train(ann, valid_data);
        fann_scale_train(ann, test_data);
      }

      struct fann* bestANN = nullptr;
      double bestRMS = 999.;
      std::vector<double> bestTrainLog = {};
      std::vector<double> bestValidLog = {};

      // repeat training several times with different random start weights
      for (unsigned int iRun = 0; iRun < m_specific_options.m_random_seeds; ++iRun) {
        double bestValid = 999.;
        std::vector<double> trainLog = {};
        std::vector<double> validLog = {};
        trainLog.assign(m_specific_options.m_max_epochs, 0.);
        validLog.assign(m_specific_options.m_max_epochs, 0.);
        int breakEpoch = 0;
        struct fann* iRunANN = nullptr;
        fann_randomize_weights(ann, -0.1, 0.1);
        for (unsigned int iEpoch = 1; iEpoch <= m_specific_options.m_max_epochs; ++iEpoch) {
          double mse;
#ifdef HAS_OPENMP
          if (m_specific_options.m_training_method != "FANN_TRAIN_INCREMENTAL") {
            mse = trainingMethods[m_specific_options.m_training_method](ann, train_data, m_specific_options.m_number_of_threads);
          } else {mse = parallel_fann::train_epoch_incremental_mod(ann, train_data);}
#else
          fann_set_training_algorithm(ann, trainingMethods[m_specific_options.m_training_method]);
          mse = fann_train_epoch(ann, train_data);
#endif
          trainLog[iEpoch - 1] = mse;
          // evaluate validation set
          fann_reset_MSE(ann);

#ifdef HAS_OPENMP
          double valid_mse = parallel_fann::test_data_parallel(ann, valid_data, m_specific_options.m_number_of_threads);
#else
          double valid_mse = fann_test_data(ann, valid_data);
#endif

          validLog[iEpoch - 1] = valid_mse;
          // keep weights for lowest validation error
          if (valid_mse < bestValid) {
            bestValid = valid_mse;
            iRunANN = fann_copy(ann);
          }
          // break when validation error increases
          if (iEpoch > m_specific_options.m_test_rate && valid_mse > validLog[iEpoch - m_specific_options.m_test_rate]) {
            if (m_specific_options.m_verbose_mode) {
              B2INFO("Training stopped in iEpoch " << iEpoch);
              B2INFO("Train error: " << mse << ", valid error: " << valid_mse <<
                     ", best valid: " << bestValid);
            }
            breakEpoch = iEpoch;
            break;
          }
          // print current status
          if (iEpoch == 1 || (iEpoch < 100 && iEpoch % 10 == 0) || iEpoch % 100 == 0) {
            if (m_specific_options.m_verbose_mode) B2INFO("Epoch " << iEpoch << ": Train error = " << mse <<
                                                            ", valid error = " << valid_mse << ", best valid = " << bestValid);
          }
        }

        // test trained network

#ifdef HAS_OPENMP
        double test_mse = parallel_fann::test_data_parallel(iRunANN, test_data, m_specific_options.m_number_of_threads);
#else
        double test_mse = fann_test_data(iRunANN, test_data);
#endif

        double RMS = sqrt(test_mse);

        if (RMS < bestRMS) {
          bestRMS = RMS;
          bestANN = fann_copy(iRunANN);
          fann_destroy(iRunANN);
          bestTrainLog.assign(trainLog.begin(), trainLog.begin() + breakEpoch);
          bestValidLog.assign(validLog.begin(), validLog.begin() + breakEpoch);
        }
        if (m_specific_options.m_verbose_mode) B2INFO("RMS on test samples: " << RMS << " (best: " << bestRMS << ")");
      }

      fann_destroy_train(data);
      fann_destroy_train(train_data);
      fann_destroy_train(valid_data);
      fann_destroy_train(test_data);
      fann_destroy(ann);

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.generateFileName();

      fann_save(bestANN, custom_weightfile.c_str());
      fann_destroy(bestANN);

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("FANN_Weightfile", custom_weightfile);
      weightfile.addVector("FANN_bestTrainLog", bestTrainLog);
      weightfile.addVector("FANN_bestValidLog", bestValidLog);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      return weightfile;

    }

    FANNExpert::~FANNExpert()
    {
      if (m_ann) {
        fann_destroy(m_ann);
      }
    }

    void FANNExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.generateFileName();
      weightfile.getFile("FANN_Weightfile", custom_weightfile);

      if (m_ann) {
        fann_destroy(m_ann);
      }
      m_ann = fann_create_from_file(custom_weightfile.c_str());

      weightfile.getOptions(m_specific_options);
    }

    std::vector<float> FANNExpert::apply(Dataset& test_data) const
    {

      std::vector<fann_type> input(test_data.getNumberOfFeatures());
      std::vector<float> probabilities(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        for (unsigned int iFeature = 0; iFeature < test_data.getNumberOfFeatures(); ++iFeature) {
          input[iFeature] = test_data.m_input[iFeature];
        }
        if (m_specific_options.m_scale_features) fann_scale_input(m_ann, input.data());
        probabilities[iEvent] = fann_run(m_ann, input.data())[0];
      }
      if (m_specific_options.m_scale_target) fann_descale_output(m_ann, probabilities.data());
      return probabilities;
    }

  }
}
