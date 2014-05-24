/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/CondParser.h>

#include <boost/algorithm/string/erase.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Belle2;


CondParser::CondParser()
{
  //Supported condition operators
  m_conditionOperatorMap[">"]  = c_GT;
  m_conditionOperatorMap["<"]  = c_ST;
  m_conditionOperatorMap[">="] = c_GE;
  m_conditionOperatorMap["<="] = c_SE;
  m_conditionOperatorMap["="]  = c_EQ;
  m_conditionOperatorMap["=="] = c_EQ;
  m_conditionOperatorMap["!="] = c_NE;
}


CondParser::~CondParser()
{

}


bool CondParser::parseCondition(string expression, EConditionOperators& condOperator, int& condValue) const
{
  //Remove all spaces from the string
  boost::erase_all(expression, " ");

  //Find the operator
  unsigned int iOperator = 0;
  while ((iOperator < expression.length()) && (!isdigit(expression[iOperator]) && (expression[iOperator] != '-'))) iOperator++;
  if (iOperator == 0) return false; //if iOperator is 0, there is either no operator or the string is empty
  string opString = expression.substr(0, iOperator);

  //Try to get the operator for the found string from the map
  map<string, EConditionOperators>::const_iterator foundIter = m_conditionOperatorMap.find(opString);
  if (foundIter == m_conditionOperatorMap.end()) return false;
  condOperator = foundIter->second;

  //Try to translate the remaining text to a number
  try {
    condValue = boost::lexical_cast<int>(expression.substr(iOperator, expression.length() - 1));
  } catch (boost::bad_lexical_cast&) {
    return false;
  }
  return true;
}


bool CondParser::evalCondition(const int& value1, const int& value2, const EConditionOperators& condOperator)
{
  switch (condOperator) {
    case c_GT : return value1 >  value2;
    case c_ST : return value1 <  value2;
    case c_GE : return value1 >= value2;
    case c_SE : return value1 <= value2;
    case c_EQ : return value1 == value2;
    case c_NE : return value1 != value2;
  }
  return false;
}
