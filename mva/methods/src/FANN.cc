/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/FANN.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void FANNOptions::load(const boost::property_tree::ptree& pt)
    {

      int version = pt.get<int>("FANN_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_max_epochs = pt.get<unsigned int>("FANN_max_epochs");
      m_epochs_between_reports = pt.get<unsigned int>("FANN_epochs_between_reports");
      m_desired_error = pt.get<double>("FANN_desired_error");

      unsigned int numberOfLayers = pt.get<unsigned int>("FANN_number_of_layers");
      m_hidden_layer_neurons.resize(numberOfLayers);
      for (unsigned int i = 0; i < numberOfLayers; ++i) {
        m_hidden_layer_neurons[i] = pt.get<unsigned int>(std::string("FANN_hidden_layer") + std::to_string(i));
      }

      m_hidden_activiation_function = pt.get<std::string>("FANN_hidden_activation_function");
      m_output_activiation_function = pt.get<std::string>("FANN_output_activation_function");
      m_error_function = pt.get<std::string>("FANN_error_function");
      m_training_method = pt.get<std::string>("FANN_training_method");
      m_scale_features = pt.get<bool>("FANN_scale_features");
      m_scale_target = pt.get<bool>("FANN_scale_target");

    }

    void FANNOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("FANN_version", 1);
      pt.put("FANN_max_epochs", m_max_epochs);
      pt.put("FANN_epochs_between_reports", m_epochs_between_reports);
      pt.put("FANN_desired_error", m_desired_error);
      pt.put("FANN_hidden_activation_function", m_hidden_activiation_function);
      pt.put("FANN_output_activation_function", m_output_activiation_function);
      pt.put("FANN_error_function", m_error_function);
      pt.put("FANN_training_method", m_training_method);
      pt.put("FANN_scale_features", m_scale_features);
      pt.put("FANN_scale_target", m_scale_target);

      pt.put("FANN_number_of_layers", m_hidden_layer_neurons.size());
      for (unsigned int i = 0; i < m_hidden_layer_neurons.size(); ++i) {
        pt.put(std::string("FANN_hidden_layer") + std::to_string(i), m_hidden_layer_neurons[i]);
      }

    }

    po::options_description FANNOptions::getDescription()
    {
      po::options_description description("FANN options");
      description.add_options()
      ("hidden_layer_neurons", po::value<std::vector<unsigned int>>(&m_hidden_layer_neurons)->multitoken(),
       "Number of neurons in each hidden layer")
      ("max_epochs", po::value<unsigned int>(&m_max_epochs), "Number of epochs")
      ("epochs_between_reports", po::value<unsigned int>(&m_epochs_between_reports), "Number of epochs between reports")
      ("desired_error", po::value<double>(&m_desired_error), "Desired error")
      ("hidden_activiation_function", po::value<std::string>(&m_hidden_activiation_function),
       "Name of acitvation function used for hidden layers")
      ("output_activiation_function", po::value<std::string>(&m_output_activiation_function),
       "Name of acitvation function used for output layer")
      ("error_function", po::value<std::string>(&m_error_function), "Name of error function")
      ("training_method", po::value<std::string>(&m_training_method), "Method used for backpropagation")
      ("scale_features", po::value<bool>(&m_scale_features), "Boolean indicating if features should be scaled or not")
      ("scale_target", po::value<bool>(&m_scale_target), "Boolean indicating if target should be scaled or not");
      return description;
    }

    FANNTeacher::FANNTeacher(const GeneralOptions& general_options, const FANNOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile FANNTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      unsigned int number_of_layers = m_specific_options.m_hidden_layer_neurons.size() + 2;

      if (m_specific_options.m_hidden_layer_neurons.size() == 0) {
        number_of_layers += 1;
      }

      auto layers = std::unique_ptr<unsigned int[]>(new unsigned int[number_of_layers]);
      layers[0] = numberOfFeatures;
      if (m_specific_options.m_hidden_layer_neurons.size() == 0) {
        layers[0] = numberOfFeatures + 1;
      } else {
        for (unsigned int i = 0; i < m_specific_options.m_hidden_layer_neurons.size(); ++i) {
          layers[i + 1] = m_specific_options.m_hidden_layer_neurons[i];
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

      std::map<std::string, enum fann_train_enum> trainingMethods;
      i = 0;
      for (auto& name : FANN_TRAIN_NAMES) {
        trainingMethods[name] = fann_train_enum(i);
        i++;
      }

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

      fann_set_activation_function_hidden(ann, activationFunctions[m_specific_options.m_hidden_activiation_function]);
      fann_set_activation_function_output(ann, activationFunctions[m_specific_options.m_output_activiation_function]);
      fann_set_train_error_function(ann, errorFunctions[m_specific_options.m_error_function]);
      fann_set_training_algorithm(ann, trainingMethods[m_specific_options.m_training_method]);

      fann_train_data* data = fann_create_train(numberOfEvents, numberOfFeatures, 1);
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
      }

      fann_randomize_weights(ann, -0.1, 0.1);
      fann_train_on_data(ann, data, m_specific_options.m_max_epochs, m_specific_options.m_epochs_between_reports,
                         m_specific_options.m_desired_error);

      fann_destroy_train(data);

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.getFileName();

      fann_save(ann, custom_weightfile.c_str());
      fann_destroy(ann);

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("FANN_Weightfile", custom_weightfile);
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

      std::string custom_weightfile = weightfile.getFileName();
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
        fann_scale_input(m_ann, input.data());
        probabilities[iEvent] = fann_run(m_ann, input.data())[0];
      }

      return probabilities;
    }

  }
}
