/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>
#include <framework/utilities/GeneralCut.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <regex>
#include <set>

using namespace Belle2;

Variable::Manager::~Manager() = default;
Variable::Manager& Variable::Manager::Instance()
{
  static Variable::Manager v;
  return v;
}

const Variable::Manager::Var* Variable::Manager::getVariable(const std::string& functionName,
    const std::vector<std::string>& functionArguments)
{
  // Combine to full name for alias resolving
  std::string fullname = functionName + "(" + boost::algorithm::join(functionArguments, ", ") + ")";

  // resolve aliases. Aliases might point to other aliases so we need to keep a
  // set of what we have seen so far to avoid running into infinite loops
  std::set<std::string> aliasesSeen;
  for (auto aliasIter = m_alias.find(fullname); aliasIter != m_alias.end(); aliasIter = m_alias.find(fullname)) {
    const auto [it, added] = aliasesSeen.insert(fullname);
    if (!added) {
      B2FATAL("Encountered a loop in the alias definitions between the aliases "
              << boost::algorithm::join(aliasesSeen, ", "));
    }
    fullname = aliasIter->second;
  }
  auto mapIter = m_variables.find(fullname);
  if (mapIter == m_variables.end()) {
    if (!createVariable(fullname, functionName, functionArguments)) return nullptr;
    mapIter = m_variables.find(fullname);
    if (mapIter == m_variables.end()) return nullptr;
  }
  return mapIter->second.get();
}

const Variable::Manager::Var* Variable::Manager::getVariable(std::string name)
{
  // resolve aliases. Aliases might point to other aliases so we need to keep a
  // set of what we have seen so far to avoid running into infinite loops
  std::set<std::string> aliasesSeen;
  for (auto aliasIter = m_alias.find(name); aliasIter != m_alias.end(); aliasIter = m_alias.find(name)) {
    const auto [it, added] = aliasesSeen.insert(name);
    if (!added) {
      B2FATAL("Encountered a loop in the alias definitions between the aliases "
              << boost::algorithm::join(aliasesSeen, ", "));
    }
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
    if (variable == m_alias[alias]) { return true; }
    B2WARNING("An alias with the name '" << alias << "' exists and is set to '" << m_alias[alias] << "', setting it to '" << variable <<
              "'. Be aware: only the last alias defined before processing the events will be used!");
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


void Variable::Manager::printAliases()
{
  long unsigned int longest_alias_size = 0;
  for (const auto& a : m_alias) {
    if (a.first.length() > longest_alias_size) {
      longest_alias_size = a.first.length();
    }
  }
  B2INFO("=====================================");
  B2INFO("The following aliases are registered:");
  for (const auto& a : m_alias) {
    B2INFO(std::string(a.first, 0, longest_alias_size) << std::string(longest_alias_size - a.first.length(),
           ' ') << " --> " << a.second);
  }
  B2INFO("=====================================");
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
        number = Belle2::convertString<double>(arg);
        arguments.push_back(number);
      }
      auto pfunc = parameterIter->second->function;
      auto func = [pfunc, arguments](const Particle * particle) -> std::variant<double, int, bool> { return pfunc(particle, arguments); };
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

  B2FATAL("Encountered bad variable name '" << name << "'. Maybe you misspelled it?");
  return false;
}

bool Variable::Manager::createVariable(const std::string fullname, const std::string& functionName,
                                       const std::vector<std::string>& functionArguments)
{
  // Search function name in parameter variables
  auto parameterIter = m_parameter_variables.find(functionName);
  if (parameterIter != m_parameter_variables.end()) {

    std::vector<double> arguments;
    for (auto& arg : functionArguments) {
      double number = 0;
      number = Belle2::convertString<double>(arg);
      arguments.push_back(number);
    }
    auto pfunc = parameterIter->second->function;
    auto func = [pfunc, arguments](const Particle * particle) -> std::variant<double, int, bool> { return pfunc(particle, arguments); };
    m_variables[fullname] = std::make_shared<Var>(fullname, func, parameterIter->second->description, parameterIter->second->group);
    return true;

  }

  // Search function fullname in meta variables
  auto metaIter = m_meta_variables.find(functionName);
  if (metaIter != m_meta_variables.end()) {
    auto func = metaIter->second->function(functionArguments);
    m_variables[fullname] = std::make_shared<Var>(fullname, func, metaIter->second->description, metaIter->second->group);
    return true;
  }

  B2FATAL("Encountered bad variable name '" << fullname << "'. Maybe you misspelled it?");
  return false;
}


void Variable::Manager::registerVariable(const std::string& name, const Variable::Manager::FunctionPtr& f,
                                         const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  assertValidName(name);

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    auto var = std::make_shared<Var>(name, f, description, m_currentGroup);
    B2DEBUG(19, "Registered Variable " << name);
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, const Variable::Manager::ParameterFunctionPtr& f,
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
    B2DEBUG(19, "Registered parameter Variable " << rawName);
    m_parameter_variables[rawName] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, const Variable::Manager::MetaFunctionPtr& f,
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
    B2DEBUG(19, "Registered meta Variable " << rawName);
    m_meta_variables[rawName] = var;
    m_variablesInRegistrationOrder.push_back(var.get());
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::deprecateVariable(const std::string& name, bool make_fatal, const std::string& version,
                                          const std::string& description)
{
  auto varIter = m_deprecated.find(name);
  if (varIter == m_deprecated.end())
    m_deprecated.insert(std::make_pair(name, std::make_pair(make_fatal, description)));
  else
    B2FATAL("There seem to be two calls to deprecate the variable: Please remove one.");

  auto mapIter = m_variables.find(name);
  if (mapIter != m_variables.end()) {
    if (make_fatal) {
      mapIter->second.get()->extendDescriptionString("\n\n.. warning:: ");
    } else {
      mapIter->second.get()->extendDescriptionString("\n\n.. note:: ");
    }
    mapIter->second.get()->extendDescriptionString(".. deprecated:: " + version + "\n " + description);
  }

}

void Variable::Manager::checkDeprecatedVariable(const std::string& name)
{
  auto varIter = m_deprecated.find(name);

  if (varIter == m_deprecated.end())
    return;
  else {
    bool make_fatal = varIter->second.first;
    std::string message = varIter->second.second;
    if (make_fatal)
      B2FATAL("Variable " << name << " is deprecated. " << message);
    else
      B2WARNING("Variable " << name << " is deprecated. " << message);
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

std::vector<std::string> Variable::Manager::getAliasNames() const
{
  std::vector<std::string> names;
  for (auto al : m_alias) names.push_back(al.first);
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
