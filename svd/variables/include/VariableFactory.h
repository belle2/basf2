#pragma once

#include <svd/variables/Variable.h>

namespace Belle2::SVD::Variables {
  class VariableFactory {
    public::
    static std::vector<ComputableVariable> create(const std::vector<std::string>& variableNames);
  };

  std::vector<ComputableVariable> VariableFactory::create(const std::vector<std::string>& variableNames)
  {
    std::vector<ComputableVariable> variables;
    for (const auto& name : variableNames) {
      variables.push_back(ComputableVariable{name});
    }
    return variables;
  }
}
