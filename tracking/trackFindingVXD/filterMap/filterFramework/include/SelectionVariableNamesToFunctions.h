/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <string>
#include <unordered_map>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

namespace Belle2 {

  /** Return a map from the SelectionVariable name to the SelectionVariable function
   * of the Variable used in the filter that is the template argument parameter.
   * This is the basic building block we will exploit later
   * @tparam Variable : Filter variable type
   * @tparam Range : range type
   * @tparam Options : optional types
   * @return unordered map relating strings to the filter variable functions
   */
  template <
    class Variable,
    class Range,
    class ... Options
    >
  std::unordered_map<std::string, typename Variable::functionType>
  SelectionVariableNamesToFunctions(Belle2::Filter<Variable, Range, Options... >)
  {
    // Note: Variable::value is the pointer to the function that return the values
    return std::unordered_map< std::string, typename Variable::functionType>
    ({ {Variable::name(), Variable::value } });
  };


  /** Wrapper for filters with NOT Operator tag
   * @tparam someFilter : Filter variable type
   * @tparam options : optional types
   * @return unordered map relating strings to the filter variable functions
   */
  template <
    class someFilter,
    class ...options >
  std::unordered_map<std::string, typename someFilter::functionType>
  SelectionVariableNamesToFunctions(Belle2::Filter<Belle2::OperatorNot,
                                    someFilter, options... >)
  {
    return SelectionVariableNamesToFunctions(someFilter());
  }


  /** Wrapper for filters with AND Operator tag
   * @tparam FilterA : a Filter variable type
   * @tparam FilterB : another Filter variable type
   * @tparam options : optional types
   * @return unordered map relating strings to the filter variable functions
   */
  template <
    class FilterA,
    class FilterB,
    class ...options
    >
  std::unordered_map<std::string, typename FilterA::functionType>
  SelectionVariableNamesToFunctions(Belle2::Filter<Belle2::OperatorAnd,
                                    FilterA, FilterB, options...>)
  {
    auto result = SelectionVariableNamesToFunctions(FilterA());
    auto resultB = SelectionVariableNamesToFunctions(FilterB());
    result.insert(resultB.begin(), resultB.end());
    return result;
  }


  /** Wrapper for filters with OR Operator tag
   * @tparam FilterA : a Filter variable type
   * @tparam FilterB : another Filter variable type
   * @tparam options : optional types
   * @return unordered map relating strings to the filter variable functions
   */
  template <
    class FilterA,
    class FilterB,
    class ...options
    >
  std::unordered_map<std::string, typename FilterA::functionType>
  SelectionVariableNamesToFunctions(Belle2::Filter<Belle2::OperatorOr,
                                    FilterA, FilterB, options...>)
  {
    auto result = SelectionVariableNamesToFunctions(FilterA());
    auto resultB = SelectionVariableNamesToFunctions(FilterB());
    result.insert(resultB.begin(), resultB.end());
    return result;
  }
}
