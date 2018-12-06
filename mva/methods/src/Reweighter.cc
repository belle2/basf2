/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/Reweighter.h>
#include <mva/interface/Interface.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void ReweighterOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("Reweighter_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }

      m_weightfile = pt.get<std::string>(std::string("Reweighter_weightfile"));
      m_variable = pt.get<std::string>(std::string("Reweighter_variable"));

    }

    void ReweighterOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Reweighter_version", 1);
      pt.put(std::string("Reweighter_weightfile"), m_weightfile);
      pt.put(std::string("Reweighter_variable"), m_variable);
    }

    po::options_description ReweighterOptions::getDescription()
    {
      po::options_description description("Reweighter options");
      description.add_options()
      ("reweighter_weightfile", po::value<std::string>(&m_weightfile),
       "Weightfile of the expert used to reweight")
      ("reweighter_variable", po::value<std::string>(&m_variable),
       "Variable which decides if the reweighter is applied or not");
      return description;
    }


    ReweighterTeacher::ReweighterTeacher(const GeneralOptions& general_options,
                                         const ReweighterOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile ReweighterTeacher::train(Dataset& training_data) const
    {

      Weightfile weightfile;

      GeneralOptions general_options;
      auto expert_weightfile = Weightfile::load(m_specific_options.m_weightfile);
      expert_weightfile.getOptions(general_options);

      // Override our variables with the one from the expert
      GeneralOptions mod_general_options = m_general_options;
      mod_general_options.m_variables = general_options.m_variables;
      mod_general_options.m_spectators = general_options.m_spectators;
      mod_general_options.m_target_variable = general_options.m_target_variable;
      mod_general_options.m_weight_variable = general_options.m_weight_variable;

      // Add reweighting variable if it is not already present somewhere
      if (m_specific_options.m_variable != "") {
        if (std::find(mod_general_options.m_variables.begin(), mod_general_options.m_variables.end(),
                      m_specific_options.m_variable) == mod_general_options.m_variables.end() and
            std::find(mod_general_options.m_spectators.begin(), mod_general_options.m_spectators.end(),
                      m_specific_options.m_variable) == mod_general_options.m_spectators.end() and
            mod_general_options.m_target_variable != m_specific_options.m_variable and
            mod_general_options.m_weight_variable != m_specific_options.m_variable) {
          mod_general_options.m_spectators.push_back(m_specific_options.m_variable);
        }
      }

      AbstractInterface::initSupportedInterfaces();
      auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
      if (supported_interfaces.find(general_options.m_method) == supported_interfaces.end()) {
        B2ERROR("Couldn't find method named " + general_options.m_method);
        throw std::runtime_error("Couldn't find method named " + general_options.m_method);
      }
      auto expert = supported_interfaces[general_options.m_method]->getExpert();
      expert->load(expert_weightfile);

      auto prediction = expert->apply(training_data);

      double data_fraction = expert_weightfile.getSignalFraction();
      double data_over_mc_fraction = data_fraction / (1 - data_fraction);

      double sum_reweights = 0;
      unsigned long int count_reweights = 0;

      auto isSignal = training_data.getSignals();

      if (m_specific_options.m_variable != "") {
        auto variable = training_data.getSpectator(training_data.getSpectatorIndex(m_specific_options.m_variable));
        for (unsigned int iEvent = 0; iEvent < training_data.getNumberOfEvents(); ++iEvent) {
          // We calculate the norm only on MC events (that is background), because
          // this is were we apply the weights in the end
          if (isSignal[iEvent]) {
            continue;
          }

          if (variable[iEvent] == 1.0) {
            if (prediction[iEvent] > 0.995)
              prediction[iEvent] = 0.995;
            if (prediction[iEvent] < 0.005)
              prediction[iEvent] = 0.005;

            prediction[iEvent] = (prediction[iEvent]) / (1 - prediction[iEvent]);
            sum_reweights += prediction[iEvent];
            count_reweights++;
          }
        }
      } else {
        for (unsigned int iEvent = 0; iEvent < training_data.getNumberOfEvents(); ++iEvent) {
          // We calculate the norm only on MC events (that is background), because
          // this is were we apply the weights in the end
          if (isSignal[iEvent]) {
            continue;
          }

          if (prediction[iEvent] > 0.995)
            prediction[iEvent] = 0.995;
          if (prediction[iEvent] < 0.005)
            prediction[iEvent] = 0.005;

          prediction[iEvent] = (prediction[iEvent]) / (1 - prediction[iEvent]);
          sum_reweights += prediction[iEvent];
          count_reweights++;
        }
      }

      double norm = sum_reweights / count_reweights / data_over_mc_fraction;

      weightfile.addOptions(mod_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("Reweighter_Weightfile", m_specific_options.m_weightfile);
      weightfile.addSignalFraction(data_fraction);
      weightfile.addElement("Reweighter_norm", norm);

      return weightfile;

    }

    void ReweighterExpert::load(Weightfile& weightfile)
    {

      weightfile.getOptions(m_specific_options);

      std::string sub_weightfile_name = weightfile.generateFileName(".xml");
      weightfile.getFile("Reweighter_Weightfile", sub_weightfile_name);
      auto sub_weightfile = Weightfile::load(sub_weightfile_name);
      sub_weightfile.getOptions(m_expert_options);

      AbstractInterface::initSupportedInterfaces();
      auto supported_interfaces = AbstractInterface::getSupportedInterfaces();
      if (supported_interfaces.find(m_expert_options.m_method) == supported_interfaces.end()) {
        B2ERROR("Couldn't find method named " + m_expert_options.m_method);
        throw std::runtime_error("Couldn't find method named " + m_expert_options.m_method);
      }
      m_expert = supported_interfaces[m_expert_options.m_method]->getExpert();
      m_expert->load(sub_weightfile);

      m_norm = weightfile.getElement<float>("Reweighter_norm");
    }

    std::vector<float> ReweighterExpert::apply(Dataset& test_data) const
    {
      auto prediction = m_expert->apply(test_data);

      if (m_specific_options.m_variable != "") {
        auto variable = test_data.getSpectator(test_data.getSpectatorIndex(m_specific_options.m_variable));

        for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
          if (variable[iEvent] != 1.0) {
            prediction[iEvent] = 1.0;
          } else {
            if (prediction[iEvent] > 0.995)
              prediction[iEvent] = 0.995;
            if (prediction[iEvent] < 0.005)
              prediction[iEvent] = 0.005;

            prediction[iEvent] = (prediction[iEvent]) / (1 - prediction[iEvent]);
            prediction[iEvent] /= m_norm;
          }
        }
      } else {
        for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
          if (prediction[iEvent] > 0.995)
            prediction[iEvent] = 0.995;
          if (prediction[iEvent] < 0.005)
            prediction[iEvent] = 0.005;

          prediction[iEvent] = (prediction[iEvent]) / (1 - prediction[iEvent]);
          prediction[iEvent] /= m_norm;
        }
      }

      return prediction;

    }
  }
}
