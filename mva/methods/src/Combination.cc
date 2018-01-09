/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/Combination.h>
#include <mva/interface/Interface.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void CombinationOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("Combination_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }

      unsigned int numberOfWeightfiles = pt.get<unsigned int>("Combination_number_of_weightfiles");
      m_weightfiles.resize(numberOfWeightfiles);
      for (unsigned int i = 0; i < numberOfWeightfiles; ++i) {
        m_weightfiles[i] = pt.get<std::string>(std::string("Combination_weightfile") + std::to_string(i));
      }

    }

    void CombinationOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Combination_version", 1);
      pt.put("Combination_number_of_weightfiles", m_weightfiles.size());
      for (unsigned int i = 0; i < m_weightfiles.size(); ++i) {
        pt.put(std::string("Combination_weightfile") + std::to_string(i), m_weightfiles[i]);
      }
    }

    po::options_description CombinationOptions::getDescription()
    {
      po::options_description description("PDF options");
      description.add_options()
      ("weightfiles", po::value<std::vector<std::string>>(&m_weightfiles)->multitoken(),
       "Weightfiles of other experts we want to combine together");
      return description;
    }


    CombinationTeacher::CombinationTeacher(const GeneralOptions& general_options,
                                           const CombinationOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile CombinationTeacher::train(Dataset& training_data) const
    {

      Weightfile weightfile;
      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);

      for (unsigned int i = 0; i < m_specific_options.m_weightfiles.size(); ++i) {
        weightfile.addFile("Combination_Weightfile" + std::to_string(i), m_specific_options.m_weightfiles[i]);
      }

      weightfile.addSignalFraction(training_data.getSignalFraction());

      return weightfile;

    }

    void CombinationExpert::load(Weightfile& weightfile)
    {

      weightfile.getOptions(m_specific_options);

      m_experts.clear();
      m_expert_variables.clear();

      for (unsigned int i = 0; i < m_specific_options.m_weightfiles.size(); ++i) {
        std::string sub_weightfile_name = weightfile.generateFileName(".xml");
        weightfile.getFile("Combination_Weightfile" +  std::to_string(i), sub_weightfile_name);
        auto sub_weightfile = Weightfile::load(sub_weightfile_name);
        GeneralOptions general_options;
        sub_weightfile.getOptions(general_options);

        AbstractInterface::initSupportedInterfaces();
        auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
        if (supported_interfaces.find(general_options.m_method) == supported_interfaces.end()) {
          B2ERROR("Couldn't find method named " + general_options.m_method);
          throw std::runtime_error("Couldn't find method named " + general_options.m_method);
        }
        auto expert = supported_interfaces[general_options.m_method]->getExpert();
        expert->load(sub_weightfile);
        m_experts.emplace_back(std::move(expert));
        m_expert_variables.push_back(general_options.m_variables);
      }
    }

    std::vector<float> CombinationExpert::apply(Dataset& test_data) const
    {
      std::vector<float> probabilities(test_data.getNumberOfEvents(), 0);
      std::vector<std::vector<float>> expert_probabilities;

      for (unsigned int iExpert = 0; iExpert < m_experts.size(); ++iExpert) {
        GeneralOptions sub_general_options = m_general_options;
        sub_general_options.m_variables = m_expert_variables[iExpert];
        SubDataset sub_dataset(sub_general_options, {}, test_data);
        expert_probabilities.push_back(m_experts[iExpert]->apply(sub_dataset));
      }

      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        double a = 1.0;
        double b = 1.0;
        for (unsigned int iExpert = 0; iExpert < m_experts.size(); ++iExpert) {
          a *= expert_probabilities[iExpert][iEvent];
          b *= (1.0 - expert_probabilities[iExpert][iEvent]);
        }
        probabilities[iEvent] = a / (a + b);
      }
      return probabilities;
    }

  }
}
