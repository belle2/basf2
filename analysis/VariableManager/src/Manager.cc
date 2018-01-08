#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <string>
#include <regex>

using namespace Belle2;

Variable::Manager::~Manager()
{
}
Variable::Manager& Variable::Manager::Instance()
{
  static Variable::Manager v;
  return v;
}

const Variable::Manager::Var* Variable::Manager::getVariable(std::string name)
{
  auto aliasIter = m_alias.find(name);
  if (aliasIter != m_alias.end()) {
    name = aliasIter->second;
  }
  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    if (!createVariable(name)) return nullptr;
    mapIter = m_variables.find(name);
    if (mapIter == m_variables.end()) return nullptr;
  }
  return mapIter->second.get();
}

std::vector<const Variable::Manager::Var*> Variable::Manager::getVariables(const std::vector<std::string>& variables)
{

  std::vector<const Variable::Manager::Var*> variable_pointers;
  for (auto& variable : variables) {
    const Var* x = getVariable(variable);
    if (x == nullptr) {
      B2WARNING("Couldn't find variable " << variable << " via the Variable::Manager. Check the name!");
    }
    variable_pointers.push_back(x);
  }
  return variable_pointers;

}


bool Variable::Manager::addAlias(const std::string& alias, const std::string& variable)
{

  assertValidName(alias);

  if (m_alias.find(alias) != m_alias.end()) {
    B2WARNING("Another alias with the name'" << alias << "' is already set! I overwrite it!");
    m_alias[alias] = variable;
    return true;
  }

  if (m_variables.find(alias) != m_variables.end()) {
    B2ERROR("Variable with the name '" << alias << "' exists already, cannot add it as an alias!");
    return false;
  }

  m_alias.insert(std::make_pair(alias, variable));
  return true;
}

bool Variable::Manager::addCollection(const std::string& collection, const std::vector<std::string>& variables)
{

  assertValidName(collection);

  if (m_collection.find(collection) != m_collection.end()) {
    B2WARNING("Another collection with the name'" << collection << "' is already set! I overwrite it!");
    m_collection[collection] = variables;
    return true;
  }

  if (m_variables.find(collection) != m_variables.end()) {
    B2ERROR("Variable with the name '" << collection << "' exists already, won't add it as an collection!");
    return false;
  }

  m_collection.insert(std::make_pair(collection, variables));
  return true;
}


std::vector<std::string> Variable::Manager::getCollection(const std::string& collection)
{

  return m_collection[collection];

}

std::vector<std::string> Variable::Manager::resolveCollections(const std::vector<std::string>& variables)
{

  std::vector<std::string> temp;

  for (const auto& var : variables) {
    auto it = m_collection.find(var);
    if (it != m_collection.end()) {
      temp.insert(temp.end(), it->second.begin(), it->second.end());
    } else {
      temp.push_back(var);
    }
  }
  return temp;

}


void Variable::Manager::assertValidName(const std::string& name)
{
  const static std::regex allowedNameRegex("^[a-zA-Z0-9_]*$");

  if (!std::regex_match(name, allowedNameRegex)) {
    B2FATAL("Variable '" << name <<
            "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
  }
}


void Variable::Manager::setVariableGroup(const std::string& groupName)
{
  m_currentGroup = groupName;
}

bool Variable::Manager::createVariable(const std::string& name)
{
  std::match_results<std::string::const_iterator> results;

  // Check if name is a simple number
  if (std::regex_match(name, results, std::regex("^([0-9]+\\.?[0-9]*)$"))) {
    float float_number = std::stof(results[1]);
    auto func = [float_number](const Particle*) -> double {
      return float_number;
    };
    m_variables[name] = std::make_shared<Var>(name, func, std::string("Returns number ") + name);
    return true;
  }

  // Check if name is a function call
  if (std::regex_match(name, results, std::regex("^([a-zA-Z0-9_]*)\\((.*)\\)$"))) {

    std::string functionName = results[1];
    boost::algorithm::trim(functionName);
    std::vector<std::string> functionArguments = splitOnDelimiterAndConserveParenthesis(results[2], ',', '(', ')');
    for (auto& str : functionArguments) {
      boost::algorithm::trim(str);
    }

    // Search function name in parameter variables
    auto parameterIter = m_parameter_variables.find(functionName);
    if (parameterIter != m_parameter_variables.end()) {

      std::vector<double> arguments;
      for (auto& arg : functionArguments) {
        double number = 0;
        number = Belle2::convertString<float>(arg);
        arguments.push_back(number);
      }
      auto pfunc = parameterIter->second->function;
      auto func = [pfunc, arguments](const Particle * particle) -> double { return pfunc(particle, arguments); };
      m_variables[name] = std::make_shared<Var>(name, func, parameterIter->second->description, parameterIter->second->group);
      return true;

    }

    // Search function name in meta variables
    auto metaIter = m_meta_variables.find(functionName);
    if (metaIter != m_meta_variables.end()) {
      auto func = metaIter->second->function(functionArguments);
      m_variables[name] = std::make_shared<Var>(name, func, metaIter->second->description, metaIter->second->group);
      return true;
    }
  }

  B2WARNING("Encountered bad variable name '" << name << "'. Maybe you misspelled it?");
  return false;
}


void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::FunctionPtr f, const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  assertValidName(name);

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    auto var = std::make_shared<Var>(name, f, description, m_currentGroup);
    B2DEBUG(100, "Registered Variable " << name);
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::ParameterFunctionPtr f,
                                         const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  auto mapIter = m_parameter_variables.find(name);
  if (mapIter == m_parameter_variables.end()) {
    auto var = std::make_shared<ParameterVar>(name, f, description, m_currentGroup);
    std::string rawName = name.substr(0, name.find('('));
    assertValidName(rawName);
    B2DEBUG(100, "Registered parameter Variable " << rawName);
    m_parameter_variables[rawName] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::MetaFunctionPtr f,
                                         const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  auto mapIter = m_meta_variables.find(name);
  if (mapIter == m_meta_variables.end()) {
    auto var = std::make_shared<MetaVar>(name, f, description, m_currentGroup);
    std::string rawName = name.substr(0, name.find('('));
    assertValidName(rawName);
    B2DEBUG(100, "Registered meta Variable " << rawName);
    m_meta_variables[rawName] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}


std::vector<std::string> Variable::Manager::getNames() const
{
  std::vector<std::string> names;
  for (const VarBase* var : m_variablesInRegistrationOrder) {
    names.push_back(var->name);
  }
  return names;
}

double Variable::Manager::evaluate(const std::string& varName, const Particle* p)
{
  const Var* var = getVariable(varName);
  if (!var) {
    throw std::runtime_error("Variable::Manager::evaluate(): variable '" + varName + "' not found!");
    return 0.0; //never reached, suppresses cppcheck warning
  }

  return var->function(p);
}
