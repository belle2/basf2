/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/Trivial.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void TrivialOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("Trivial_version");
      if (version != 1) {
        B2ERROR("Unknown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unknown weightfile version " + std::to_string(version));
      }
      m_output = pt.get<double>("Trivial_output");

      auto numberOfOutputs = pt.get<unsigned int>("Trivial_number_of_multiple_outputs", 0u);
      m_multiple_output.resize(numberOfOutputs);
      for (unsigned int i = 0; i < numberOfOutputs; ++i) {
        m_multiple_output[i] = pt.get<double>(std::string("Trivial_multiple_output") + std::to_string(i));
      }

    }

    void TrivialOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Trivial_version", 1);
      pt.put("Trivial_output", m_output);
      pt.put("Trivial_number_of_multiple_outputs", m_multiple_output.size());
      for (unsigned int i = 0; i < m_multiple_output.size(); ++i) {
        pt.put(std::string("Trivial_multiple_output") + std::to_string(i), m_multiple_output[i]);
      }
    }

    po::options_description TrivialOptions::getDescription()
    {
      po::options_description description("Trivial options");
      description.add_options()
      ("output", po::value<double>(&m_output), "Outputs this value for all predicitions in binary classification.");
      description.add_options()
      ("multiple_output", po::value<std::vector<double>>(&m_multiple_output)->multitoken(),
       "Outputs these values for their respective classes in multiclass classification.");
      return description;
    }


    TrivialTeacher::TrivialTeacher(const GeneralOptions& general_options,
                                   const TrivialOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile TrivialTeacher::train(Dataset& training_data) const
    {
      Weightfile weightfile;
      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addSignalFraction(training_data.getSignalFraction());
      return weightfile;
    }

    void TrivialExpert::load(Weightfile& weightfile)
    {
      weightfile.getOptions(m_general_options);
      weightfile.getOptions(m_specific_options);
    }

    std::vector<float> TrivialExpert::apply(Dataset& test_data) const
    {

      std::vector<float> probabilities(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        probabilities[iEvent] = m_specific_options.m_output;
      }

      return probabilities;

    }

    std::vector<std::vector<float>> TrivialExpert::applyMulticlass(Dataset& test_data) const
    {
      if (m_general_options.m_nClasses != m_specific_options.m_multiple_output.size()) {
        B2ERROR("The number of classes declared in the general options do not match the number of outputs declared in the specific options for the Trivial expert");
      }
      std::vector<std::vector<float>> probabilities(test_data.getNumberOfEvents(), std::vector<float>(m_general_options.m_nClasses));
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        for (unsigned int iClass = 0; iClass < m_general_options.m_nClasses; ++iClass) {
          probabilities[iEvent][iClass] = m_specific_options.m_multiple_output.at(iClass);
        }
      }
      return probabilities;
    }
  }
}
