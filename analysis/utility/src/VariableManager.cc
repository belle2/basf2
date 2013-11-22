#include <analysis/utility/VariableManager.h>

#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>

using namespace Belle2;
using boost::algorithm::to_lower_copy;

namespace {
  /** Do case-insensitve comparison of two variable names. */
  bool compareCaseInsensitive(const VariableManager::Var* a, const VariableManager::Var* b)
  {
    return to_lower_copy(a->name) < to_lower_copy(b->name);
  }
}


VariableManager::~VariableManager()
{
  for (auto & pair : m_variables) {
    delete pair.second;
  }
}
VariableManager& VariableManager::Instance()
{
  static VariableManager v;
  return v;
}

const VariableManager::Var* VariableManager::getVariable(const std::string& name) const
{
  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    return nullptr;
  } else {
    return mapIter->second;
  }

}

void VariableManager::registerVariable(const std::string& name, VariableManager::FunctionPtr f, const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    Var* var = new Var {name, f, description};
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }

}
