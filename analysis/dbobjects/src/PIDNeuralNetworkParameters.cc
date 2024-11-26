/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dbobjects/PIDNeuralNetworkParameters.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


using namespace Belle2;


int PIDNeuralNetworkParameters::pdg2OutputIndex(const int pdg, const bool throwException) const
{
  const auto itr = std::find(m_outputSpeciesPdg.begin(), m_outputSpeciesPdg.end(), pdg);
  if (itr == m_outputSpeciesPdg.end()) {
    if (throwException)
      B2FATAL("Invalid particle-species hypothesis: " << pdg
              << ". PID neural network is not trained for this particle-species hypothesis");
    return -1;
  }
  return static_cast<size_t>(itr - m_outputSpeciesPdg.begin());
}


size_t PIDNeuralNetworkParameters::getInputIndex(const std::string& name) const
{
  const auto itr = std::find(m_inputNames.begin(), m_inputNames.end(), name);
  if (itr == m_inputNames.end()) B2FATAL("Invalid input name: " << name);
  return static_cast<size_t>(itr - m_inputNames.begin());
}