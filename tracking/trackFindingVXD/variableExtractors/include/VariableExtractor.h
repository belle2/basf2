/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <string>
#include <tracking/trackFindingVXD/utilities/Named.h>
#include <unordered_map>


namespace Belle2 {
  /// class to extract individual variables
  class VariableExtractor {
  public:

  protected:
    /// add a variable to the variable set
    void addVariable(const std::string& identifier, std::vector<Named<float*>>& variables)
    {
      //todo: verify if it is faster to check explicitly or not
      auto value = m_variables.emplace(identifier, NAN).first;
      variables.emplace_back(identifier, &(value->second));
    }

    /// unordered_map to associate float value with a string name
    std::unordered_map<std::string, float> m_variables;
  };
}
