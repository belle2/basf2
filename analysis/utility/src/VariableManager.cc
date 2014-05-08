#include <analysis/utility/VariableManager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

#include <boost/regex.hpp>

#include <iostream>
#include <iomanip>

using namespace Belle2;


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

  const static boost::regex allowedNameRegex("^[a-zA-Z0-9_]*$");
  if (!boost::regex_match(name, allowedNameRegex)) {
    B2FATAL("Variable '" << name << "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
  }

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    Var* var = new Var(name, f, description);
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void VariableManager::registerParticleExtraInfoVariable(const std::string& name, const std::string& description)
{

  const static boost::regex allowedNameRegex("^[a-zA-Z0-9_]*$");
  if (!boost::regex_match(name, allowedNameRegex)) {
    B2FATAL("Variable '" << name << "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
  }

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    auto func = [name](const Particle * particle) -> double {
      return particle->getExtraInfo(name);
    };
    Var* var = new Var(name, func, description);
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
  }

}
std::vector<std::string> VariableManager::getNames() const
{
  std::vector<std::string> names;
  for (const Var * var : m_variablesInRegistrationOrder) {
    names.push_back(var->name);
  }
  return names;
}

void VariableManager::printList() const
{
  for (const Var * var : getVariables()) {
    std::cout << std::setw(14) << var->name << "  " << var->description << std::endl;
  }
}

double VariableManager::evaluate(const std::string& varName, const Particle* p) const
{
  const Var* var = getVariable(varName);
  if (!var) {
    B2FATAL("VariableManager::evaluate(): variable '" << varName << "' not found!");
  }

  return var->function(p);
}
