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
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_output = pt.get<double>("Trivial_output");
    }

    void TrivialOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Trivial_version", 1);
      pt.put("Trivial_output", m_output);
    }

    po::options_description TrivialOptions::getDescription()
    {
      po::options_description description("Trivial options");
      description.add_options()
      ("output", po::value<double>(&m_output), "Outputs allows this given floating point number");
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

  }
}
