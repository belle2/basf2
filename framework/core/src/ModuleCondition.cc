#include <framework/core/ModuleCondition.h>
#include <framework/core/Path.h>
#include <framework/utilities/Conversion.h>

#include <boost/algorithm/string/erase.hpp>

#include <boost/python/class.hpp>
#include <boost/python/docstring_options.hpp>

#include <map>

using namespace Belle2;
using namespace boost::python;

ModuleCondition::ModuleCondition(std::string expression, PathPtr conditionPath, EAfterConditionPath afterConditionPath):
  m_conditionPath(conditionPath),
  m_afterConditionPath(afterConditionPath)
{
  /** Maps the operator string to a EConditionOperators value. */
  std::map<std::string, EConditionOperators> m_conditionOperatorMap;
  m_conditionOperatorMap[">"]  = c_GT;
  m_conditionOperatorMap["<"]  = c_ST;
  m_conditionOperatorMap[">="] = c_GE;
  m_conditionOperatorMap["<="] = c_SE;
  m_conditionOperatorMap["="]  = c_EQ;
  m_conditionOperatorMap["=="] = c_EQ;
  m_conditionOperatorMap["!="] = c_NE;

  //Remove all spaces from the string
  boost::erase_all(expression, " ");

  //Find the operator
  unsigned int iOperator = 0;
  while ((iOperator < expression.length()) && (!isdigit(expression[iOperator]) && (expression[iOperator] != '-'))) iOperator++;
  if (iOperator == 0) //if iOperator is 0, there is either no operator or the string is empty
    throw std::runtime_error("Invalid condition: could not parse condition: '" + expression + "'!");
  std::string opString = expression.substr(0, iOperator);

  //Try to get the operator for the found string from the map
  auto foundIter = m_conditionOperatorMap.find(opString);
  if (foundIter == m_conditionOperatorMap.end())
    throw std::runtime_error("Invalid condition: could not parse condition: '" + expression + "'!");

  m_conditionOperator = foundIter->second;
  //Try to translate the remaining text to a number
  m_conditionValue = convertString<int>(expression.substr(iOperator, expression.length() - 1));
}

ModuleCondition::ModuleCondition(const ModuleCondition& other)
{
  if (other.m_conditionPath) {
    std::shared_ptr<Path> p = std::static_pointer_cast<Path>(other.m_conditionPath->clone());
    m_conditionPath = p;
  }
  m_conditionOperator = other.m_conditionOperator;
  m_conditionValue = other.m_conditionValue;
  m_afterConditionPath = other.m_afterConditionPath;
}

bool ModuleCondition::evaluate(int value) const
{
  switch (m_conditionOperator) {
    case c_GT : return value >  m_conditionValue;
    case c_ST : return value <  m_conditionValue;
    case c_GE : return value >= m_conditionValue;
    case c_SE : return value <= m_conditionValue;
    case c_EQ : return value == m_conditionValue;
    case c_NE : return value != m_conditionValue;
  }
  return false;
}

std::string ModuleCondition::getString() const
{
  std::string output = "(? ";
  switch (m_conditionOperator) {
    case c_GT: output += ">"; break;
    case c_ST: output += "<"; break;
    case c_GE: output += ">="; break;
    case c_SE: output += "<="; break;
    case c_NE: output += "!="; break;
    case c_EQ: output += "=="; break;
    default: output += "???";
  }
  output += std::to_string(m_conditionValue);
  output += m_conditionPath->getPathString();
  output += " )";
  return output;
}

namespace {
  /// Same function as in the class itself, but return by value
  std::shared_ptr<Path> _getPathPython(ModuleCondition* m) {return m->getPath(); };
}

void ModuleCondition::exposePythonAPI()
{
  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  //Python class definition
  class_<ModuleCondition, boost::noncopyable>("ModuleCondition", no_init)
  .def("__str__", &ModuleCondition::getString)
  .def("get_value", &ModuleCondition::getConditionValue)
  .def("get_operator", &ModuleCondition::getConditionOperator)
  .def("get_after_path", &ModuleCondition::getAfterConditionPath)
  .def("get_path", &_getPathPython)
  ;
}
