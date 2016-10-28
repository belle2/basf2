/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/Expert.h>
#include <analysis/TMVAInterface/Method.h>
#include <analysis/VariableManager/Utility.h>

#include <framework/utilities/WorkingDirectoryManager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <TSystem.h>
#include <TMVA/Tools.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace Belle2 {

  namespace TMVAInterface {

    Expert::Expert(const ExpertConfig& config, bool transformToProbability) : m_config(config),
      m_transformToProbability(transformToProbability)
    {
      B2WARNING("TMVAInterface is deprecated and will be removed, please migrate all your code to the new mva package!");
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      // Initialize TMVA and ROOT stuff
      TMVA::Tools::Instance();

      m_reader = std::unique_ptr<TMVA::Reader>(new TMVA::Reader("!Color:!Silent"));
      m_variables = config.getVariablesFromManager();
      m_spectators = config.getSpectatorsFromManager();
      m_signalFraction = config.getSignalFraction();

      if (m_signalFraction > 1.0) {
        B2ERROR("Given Signal Fraction exceeds 1.0!");
      }

      auto variable_names = m_config.getVariables();
      auto spectator_names = m_config.getSpectators();

      m_input.resize(variable_names.size() + spectator_names.size(), 0);
      for (unsigned int i = 0; i < variable_names.size(); ++i) {
        m_reader->AddVariable(makeROOTCompatible(variable_names[i]), &m_input[i]);
      }
      for (unsigned int i = 0; i < spectator_names.size(); ++i) {
        m_reader->AddSpectator(makeROOTCompatible(spectator_names[i]), &m_input[i + variable_names.size()]);
      }

      if (!m_reader->BookMVA(m_config.getMethod(), m_config.getWeightfile())) {
        B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
      }

    }

    float Expert::analyse(const Particle* particle)
    {

      for (unsigned int i = 0; i < m_variables.size(); ++i) {
        m_input[i] = m_variables[i]->function(particle);
      }

      // We do not fill the spectators, because they're not used anyway by EvaluateMVA
      // and the spectators may contain variables which need MC information, which is not available on real data
      //for (unsigned int i = 0; i < m_spectators.size(); ++i) {
      //  m_input[i + m_variables.size()] = m_spectators[i]->function(particle);
      //}

      return _analyse();

    }

    float Expert::analyse(const std::vector<float>& features)
    {

      for (unsigned int i = 0; i < features.size(); ++i) {
        m_input[i] = features[i];
      }

      return _analyse();

    }

    float Expert::_analyse()
    {

      double result = 0;
      if (m_transformToProbability)
        result = m_reader->GetProba(m_config.getMethod(), m_signalFraction);
      else
        result = m_reader->EvaluateMVA(m_config.getMethod());

      if (result == -999) {
        B2WARNING("TMVA returned -999, which indicates that something went wrong while applying the MVA method, check previous messages for more information. In consequence the returned probability is not constrained to [0,1]!");
      }

      return result;

    }



  }
}
