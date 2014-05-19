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

const VariableManager::Var* VariableManager::getVariable(const std::string& name)
{
  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    return createVariable(name);
  } else {
    return mapIter->second;
  }
}

const VariableManager::Var* VariableManager::createVariable(const std::string& name)
{

  VariableManager::FunctionPtr func;
  const static boost::regex allowedForm("^([a-zA-Z0-9_]*)\\((.*)\\)$");
  boost::match_results<std::string::const_iterator> results;

  if (boost::regex_match(name, results, allowedForm)) {
    if (results[1] == "GetExtraInfo") {
      auto extraInfoName = results[2];
      func = [extraInfoName](const Particle * particle) -> double {
        return particle->getExtraInfo(extraInfoName);
      };
    } else {
      const VariableManager::Var* var = getVariable(results[2]);
      if (results[1] == "daughterProductOf") {
        func = [var](const Particle * particle) -> double {
          double product = 1.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
            product *= var->function(particle->getDaughter(j));
          }
          return product;
        };
      } else if (results[1] == "daughterSumOf") {
        func = [var](const Particle * particle) -> double {
          double sum = 1.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
            sum += var->function(particle->getDaughter(j));
          }
          return sum;
        };
      } else if (results[1] == "daughter1") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(1)); };
      } else if (results[1] == "daughter2") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(2)); };
      } else if (results[1] == "daughter3") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(3)); };
      } else if (results[1] == "daughter4") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(4)); };
      } else if (results[1] == "daughter5") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(5)); };
      } else if (results[1] == "daughter6") {
        func = [var](const Particle * particle) -> double { return var->function(particle->getDaughter(6)); };
      } else {
        return nullptr;
      }
    }
  } else {
    return nullptr;
  }
  Var* var = new Var(name, func, name);
  m_variables[name] = var;
  m_variablesInRegistrationOrder.push_back(var);
  return getVariable(name);
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

double VariableManager::evaluate(const std::string& varName, const Particle* p)
{
  const Var* var = getVariable(varName);
  if (!var) {
    B2FATAL("VariableManager::evaluate(): variable '" << varName << "' not found!");
  }

  return var->function(p);
}
