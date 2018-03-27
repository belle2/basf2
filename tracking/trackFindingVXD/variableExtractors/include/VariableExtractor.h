/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    void addVariable(std::string identifier, std::vector<Named<float*>>& variables)
    {
      //todo: verify if it is faster to check explicitly or not
      auto value = m_variables.emplace(identifier, NAN).first;
      variables.emplace_back(identifier, &(value->second));
    }

    /// unordered_map to associate float value with a string name
    std::unordered_map<std::string, float> m_variables;
  };
}
