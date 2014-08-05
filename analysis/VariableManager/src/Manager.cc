#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace Belle2;

Variable::Manager::~Manager()
{
  for (auto & pair : m_variables) {
    delete pair.second;
  }
}
Variable::Manager& Variable::Manager::Instance()
{
  static Variable::Manager v;
  return v;
}

const Variable::Manager::Var* Variable::Manager::getVariable(const std::string& name)
{
  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    if (createVariable(name)) {
      mapIter = m_variables.find(name);
      if (mapIter == m_variables.end()) {
        return nullptr;
      }
    }
  }
  return mapIter->second;
}

void Variable::Manager::setVariableGroup(const std::string& groupName)
{
  m_currentGroup = groupName;
}

bool Variable::Manager::createVariable(const std::string& name)
{

  boost::match_results<std::string::const_iterator> results;

  // Check if name is a simple number
  if (boost::regex_match(name, results, boost::regex("^([0-9]+\\.?[0-9]*)$"))) {
    float float_number = boost::lexical_cast<float>(results[1]);
    auto func = [float_number](const Particle*) -> double {
      return float_number;
    };
    Var* var = new Var(name, func, std::string("Returns number ") + name);
    if (var == nullptr)
      return false;
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
    return true;
  }

  // Check if name is a function call
  if (boost::regex_match(name, results, boost::regex("^([a-zA-Z0-9_]*)\\((.*)\\)$"))) {

    std::string functionName = results[1];
    boost::algorithm::trim(functionName);
    std::vector<std::string> functionArguments = splitOnDelimiterAndConserveParenthesis(results[2], ',', '(', ')');
    for (auto & str : functionArguments) {
      boost::algorithm::trim(str);
    }

    // Search function name in parameter variables
    auto parameterIter = m_parameter_variables.find(functionName);
    if (parameterIter != m_parameter_variables.end()) {

      std::vector<double> arguments;
      for (auto & arg : functionArguments) {
        double number = 0;
        try {
          number = boost::lexical_cast<double>(arg);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("Argument for parameter function cannot be converted to float: " << arg);
        }
        arguments.push_back(number);
      }
      auto pfunc = parameterIter->second->function;
      auto func = [pfunc, arguments](const Particle * particle) -> double { return pfunc(particle, arguments); };
      Var* var = new Var(name, func, parameterIter->second->description, parameterIter->second->group);
      if (var == nullptr)
        return false;
      m_variables[name] = var;
      m_variablesInRegistrationOrder.push_back(var);
      return true;

    }

    // Search function name in meta variables
    auto metaIter = m_meta_variables.find(functionName);
    if (metaIter != m_meta_variables.end()) {
      auto func = metaIter->second->function(functionArguments);
      Var* var = new Var(name, func, metaIter->second->description, metaIter->second->group);
      if (var == nullptr)
        return false;
      m_variables[name] = var;
      m_variablesInRegistrationOrder.push_back(var);
      return true;
    }
  }

  B2WARNING("Encountered bad variable name " << name << " i don't know what to do with it")
  return false;
}


void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::FunctionPtr f, const std::string& description)
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
    Var* var = new Var(name, f, description, m_currentGroup);
    B2DEBUG(100, "Registered Variable " << name)
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::ParameterFunctionPtr f, const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  const static boost::regex allowedNameRegex("^[a-zA-Z0-9_]*$");

  auto mapIter = m_parameter_variables.find(name);
  if (mapIter == m_parameter_variables.end()) {
    ParameterVar* var = new ParameterVar(name, f, description, m_currentGroup);
    std::string rawName = name.substr(0, name.find('('));
    if (!boost::regex_match(rawName, allowedNameRegex)) {
      B2FATAL("Variable '" << rawName << "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
    }
    B2DEBUG(100, "Registered parameter Variable " << rawName)
    m_parameter_variables[rawName] = var;
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}

void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::MetaFunctionPtr f, const std::string& description)
{
  if (!f) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  const static boost::regex allowedNameRegex("^[a-zA-Z0-9_]*$");

  auto mapIter = m_meta_variables.find(name);
  if (mapIter == m_meta_variables.end()) {
    MetaVar* var = new MetaVar(name, f, description, m_currentGroup);
    std::string rawName = name.substr(0, name.find('('));
    if (!boost::regex_match(rawName, allowedNameRegex)) {
      B2FATAL("Variable '" << rawName << "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
    }
    B2DEBUG(100, "Registered meta Variable " << rawName)
    m_meta_variables[rawName] = var;
  } else {
    B2FATAL("A variable named '" << name << "' was already registered! Note that all variables need a unique name!");
  }
}


std::vector<std::string> Variable::Manager::getNames() const
{
  std::vector<std::string> names;
  for (const Var * var : m_variablesInRegistrationOrder) {
    names.push_back(var->name);
  }
  return names;
}

void Variable::Manager::printList() const
{
  std::string group;
  for (const Var * var : getVariables()) {
    if (var->group != group) {
      //group changed, print header
      std::cout << "\n" << var->group << ":\n";
      group = var->group;
    }
    std::cout << std::setw(14) << var->name << "  " << var->description << std::endl;
  }

  for (auto & pair : m_parameter_variables) {
    auto* var = pair.second;
    if (var->group != group) {
      //group changed, print header
      std::cout << "\n" << var->group << ":\n";
      group = var->group;
    }
    std::cout << std::setw(14) << var->name << "  " << var->description << std::endl;
  }

  for (auto & pair : m_meta_variables) {
    auto* var = pair.second;
    if (var->group != group) {
      //group changed, print header
      std::cout << "\n" << var->group << ":\n";
      group = var->group;
    }
    std::cout << std::setw(14) << var->name << "  " << var->description << std::endl;
  }
}

double Variable::Manager::evaluate(const std::string& varName, const Particle* p)
{
  const Var* var = getVariable(varName);
  if (!var) {
    B2FATAL("Variable::Manager::evaluate(): variable '" << varName << "' not found!");
    return 0.0; //never reached, suppresses cppcheck warning
  }

  return var->function(p);
}
