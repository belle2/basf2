#include <analysis/utility/VariableManager.h>

#include <framework/logging/Logger.h>

#include <boost/regex.hpp>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleInfo.h>

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

void VariableManager::registerParticleInfoVariable(const std::string& name, const std::string& description, bool allowMultipleRegistrations)
{

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    auto func = [name](const Particle * particle) -> double {
      const ParticleInfo* particleInfo = DataStore::getRelated<ParticleInfo>(particle);
      if (particleInfo == nullptr or not particleInfo->isAvailable(name)) {
        B2INFO("VariableManger: Particle doesn't have " <<  name << " set in the related ParticleInfo. Return 0!")
        return 0.0;
      }
      return particleInfo->getValue(name);
    };
    Var* var = new Var {name, func, description};
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
  } else {
    if (allowMultipleRegistrations)
      return;
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }

}
