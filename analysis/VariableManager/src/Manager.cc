#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventExtraInfo.h>

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
    return createVariable(name);
  } else {
    return mapIter->second;
  }
}
void Variable::Manager::setVariableGroup(const std::string& groupName)
{
  m_currentGroup = groupName;
}

const Variable::Manager::Var* Variable::Manager::createVariable(const std::string& name)
{

  Variable::Manager::FunctionPtr func;

  // Check if name is a simple number
  const static boost::regex allowedNumberForm("^([0-9]+\\.?[0-9]*)$");
  boost::match_results<std::string::const_iterator> number;
  if (boost::regex_match(name, number, allowedNumberForm)) {
    float float_number;
    std::stringstream sstream(number[1]) ;
    sstream >> float_number;
    func = [float_number](const Particle*) -> double {
      return float_number;
    };
  } else {
    const static boost::regex allowedForm("^([a-zA-Z0-9_]*)\\((.*)\\)$");
    boost::match_results<std::string::const_iterator> results;

    if (boost::regex_match(name, results, allowedForm)) {

      std::string functionName = results[1];
      boost::algorithm::trim(functionName);
      std::vector<std::string> functionArguments = splitOnDelimiterAndConserveParenthesis(results[2], ',', '(', ')');
      for (auto & str : functionArguments) {
        boost::algorithm::trim(str);
      }

      if (functionName == "getExtraInfo") {
        if (functionArguments.size() == 1) {
          auto extraInfoName = functionArguments[0];
          func = [extraInfoName](const Particle * particle) -> double {
            if (particle == nullptr) {
              StoreObjPtr<EventExtraInfo> eventExtraInfo;
              return eventExtraInfo->getExtraInfo(extraInfoName);
            }
            return particle->getExtraInfo(extraInfoName);
          };
        } else {
          B2FATAL("Wrong number of arguments for meta function getExtraInfo");
        }
      }

      else if (functionName == "daughterProductOf") {
        if (functionArguments.size() == 1) {
          const Variable::Manager::Var* var = getVariable(functionArguments[0]);
          func = [var](const Particle * particle) -> double {
            double product = 1.0;
            for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
              product *= var->function(particle->getDaughter(j));
            }
            return product;
          };
        } else {
          B2FATAL("Wrong number of arguments for meta function daughterProductOf");
        }

      } else if (functionName == "daughterSumOf") {
        if (functionArguments.size() == 1) {
          const Variable::Manager::Var* var = getVariable(functionArguments[0]);
          func = [var](const Particle * particle) -> double {
            double sum = 0.0;
            for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
              sum += var->function(particle->getDaughter(j));
            }
            return sum;
          };
        } else {
          B2FATAL("Wrong number of arguments for meta function daughterSumOf");
        }

      } else if (functionName == "abs") {
        if (functionArguments.size() == 1) {
          const Variable::Manager::Var* var = getVariable(functionArguments[0]);
          func = [var](const Particle * particle) -> double { return std::abs(var->function(particle)); };
        } else {
          B2FATAL("Wrong number of arguments for meta function abs");
        }

      } else if (functionName == "daughter") {
        if (functionArguments.size() == 2) {
          int daughterNumber = 0;
          try {
            daughterNumber = boost::lexical_cast<int>(functionArguments[0]);
          } catch (boost::bad_lexical_cast&) {
            B2FATAL("First argument of daughter meta function must be integer!");
          }
          const Variable::Manager::Var* var = getVariable(functionArguments[1]);
          func = [var, daughterNumber](const Particle * particle) -> double { return var->function(particle->getDaughter(daughterNumber)); };
        } else {
          B2FATAL("Wrong number of arguments for meta function daughter");
        }

      } else if (m_variables.find(functionName) != m_variables.end()) {
        const Variable::Manager::Var* var = getVariable(functionName);
        if (var->pfunction == nullptr) {
          B2FATAL("Functions doesn't takes arguments!");
        }
        std::vector<double> arguments;
        for (auto & arg : functionArguments) {
          double number = 0;
          try {
            number = boost::lexical_cast<double>(arg);
          } catch (boost::bad_lexical_cast&) {
            B2FATAL("Any additional argument of a function must be float!");
          }
          arguments.push_back(number);
        }
        func = [var, arguments](const Particle * particle) -> double { return var->pfunction(particle, arguments); };
      } else {
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }

  std::string escapedName = name;
  std::replace(escapedName.begin(), escapedName.end(), '(', '_');
  std::replace(escapedName.begin(), escapedName.end(), ')', '_');

  Var* var = new Var(name, func, nullptr, name);
  m_variables[name] = var;
  m_variablesInRegistrationOrder.push_back(var);
  return getVariable(name);
}

void Variable::Manager::registerVariable(const std::string& name, Variable::Manager::FunctionPtr f, Variable::Manager::FunctionPtrWithParameters pf, const std::string& description)
{
  if (!f and !pf) {
    B2FATAL("No function provided for variable '" << name << "'.");
  }

  const static boost::regex allowedNameRegex("^[a-zA-Z0-9_]*$");

  if (!boost::regex_match(name, allowedNameRegex)) {
    B2FATAL("Variable '" << name << "' contains forbidden characters! Only alphanumeric characters plus underscores (_) are allowed for variable names.");
  }

  auto mapIter = m_variables.find(name);
  if (mapIter == m_variables.end()) {
    Var* var = new Var(name, f, pf, description, m_currentGroup);
    m_variables[name] = var;
    m_variablesInRegistrationOrder.push_back(var);
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
