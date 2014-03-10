/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/Expert.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <TMVA/Reader.h>
#include <TMVA/Tools.h>
#include <TString.h>
#include <TSystem.h>

#include <fstream>

namespace Belle2 {

  namespace TMVAInterface {

    Expert::Expert(std::string weightfile)
    {

      // Initialize TMVA and ROOT stuff
      TMVA::Tools::Instance();
      m_reader = new TMVA::Reader("!Color:!Silent");

      std::ifstream istream(weightfile);
      if (not istream.good()) {
        B2ERROR("Couldn't open weightfile" << weightfile)
      }
      m_method = new Method(istream);

      const auto& variables = m_method->getVariables();
      m_input.resize(variables.size(), 0);
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_reader->AddVariable(variables[i]->name, &m_input[i]);
      }

      if (!m_reader->BookMVA(m_method->getName(), weightfile)) {
        B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
      }

    }

    float Expert::analyse(const Particle* particle)
    {
      const auto& variables = m_method->getVariables();
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_input[i] = variables[i]->function(particle);
      }

      // Return Signal Probability of given Particle
      // reader->GetEvaluateMVA(m_method, signal_efficiency);
      // reader->GetProba(m_method, signal_fraction);
      return m_reader->GetProba(m_method->getName());

    }

    Expert::~Expert()
    {
      delete m_method;
      delete m_reader;
    }

  }
}

