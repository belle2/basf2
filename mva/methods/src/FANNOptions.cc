/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/FANN.h>

#include <framework/logging/Logger.h>
#include <TFormula.h>

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
      m_verbose_mode = pt.get<bool>("FANN_verbose_mode");

      m_hidden_layers_architecture = pt.get<std::string>("FANN_hidden_layers_architecture");

      m_hidden_activiation_function = pt.get<std::string>("FANN_hidden_activation_function");
      m_output_activiation_function = pt.get<std::string>("FANN_output_activation_function");
      m_error_function = pt.get<std::string>("FANN_error_function");
      m_training_method = pt.get<std::string>("FANN_training_method");
      m_validation_fraction = pt.get<double>("FANN_validation_fraction");
      m_random_seeds = pt.get<unsigned int>("FANN_random_seeds");
      m_test_rate = pt.get<unsigned int>("FANN_test_rate");
      m_number_of_threads = pt.get<unsigned int>("FANN_number_of_threads");

      m_scale_features = pt.get<bool>("FANN_scale_features");
      m_scale_target = pt.get<bool>("FANN_scale_target");

    }

    void FANNOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("FANN_version", 1);
      pt.put("FANN_max_epochs", m_max_epochs);
      pt.put("FANN_verbose_mode", m_verbose_mode);
      pt.put("FANN_hidden_layers_architecture", m_hidden_layers_architecture);
      pt.put("FANN_hidden_activation_function", m_hidden_activiation_function);
      pt.put("FANN_output_activation_function", m_output_activiation_function);
      pt.put("FANN_error_function", m_error_function);
      pt.put("FANN_training_method", m_training_method);
      pt.put("FANN_validation_fraction", m_validation_fraction);
      pt.put("FANN_random_seeds", m_random_seeds);
      pt.put("FANN_test_rate", m_test_rate);
      pt.put("FANN_number_of_threads", m_number_of_threads);

      pt.put("FANN_scale_features", m_scale_features);
      pt.put("FANN_scale_target", m_scale_target);


    }

    po::options_description FANNOptions::getDescription()
    {
      po::options_description description("FANN options");
      description.add_options()
      ("max_epochs", po::value<unsigned int>(&m_max_epochs), "Number of iEpochs")
      ("verbose_mode", po::value<bool>(&m_verbose_mode), "Prints out the training status or not")
      ("hidden_layers_architecture", po::value<std::string>(&m_hidden_layers_architecture),
       "Architecture with number of neurons in each hidden layer")
      ("hidden_activiation_function", po::value<std::string>(&m_hidden_activiation_function),
       "Name of acitvation function used for hidden layers")
      ("output_activiation_function", po::value<std::string>(&m_output_activiation_function),
       "Name of acitvation function used for output layer")
      ("error_function", po::value<std::string>(&m_error_function), "Name of error function")
      ("training_method", po::value<std::string>(&m_training_method), "Method used for backpropagation")
      ("validation_fraction", po::value<double>(&m_validation_fraction), "Fraction of training sample used for validation.")
      ("random_seeds", po::value<unsigned int>(&m_random_seeds),
       "Number of times the training is repeated with a new weight random seed.")
      ("test_rate", po::value<unsigned int>(&m_test_rate), "Rate of iEpochs to check the validation error")
      ("number_of_threads", po::value<unsigned int>(&m_number_of_threads), "Number of threads for parallel training")
      ("scale_features", po::value<bool>(&m_scale_features), "Boolean indicating if features should be scaled or not")
      ("scale_target", po::value<bool>(&m_scale_target), "Boolean indicating if target should be scaled or not");
      return description;
    }

    std::vector<unsigned int> FANNOptions::getHiddenLayerNeurons(unsigned int nf) const
    {
      std::vector<unsigned int> hiddenLayers;
      std::stringstream iLayers(m_hidden_layers_architecture);
      std::string layer;
      while (std::getline(iLayers, layer, ',')) {
        for (auto& character : layer) {
          if (character == 'N') character = 'x';
        }
        auto* iLayerSize = new TFormula("iLayerSize", layer.c_str());
        hiddenLayers.push_back(iLayerSize->Eval(nf));
      }
      return hiddenLayers;
    }
  }
}