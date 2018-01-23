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
#include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>
#include <string>
#include <unordered_map>
namespace Belle2 {

  /** Return a map from the SelectionVariable name to the SelectionVariable function
   * of the Variable used in the filter that is the template argument parameter.
   * This is the basic building block we will exploit later
   **/
  template <
    class Variable,
    class Range,
    class ... Options
    >
  std::unordered_map< std::string , typename Variable::functionType >
  SelectionVariableNamesToFunctions(Belle2::Filter< Variable, Range, Options... >)
  {
    // Note: Variable::value is the pointer to the function that returns
    // the values
    return std::unordered_map< std::string , typename Variable::functionType >
    ({ {Variable::name(), Variable::value } });
  };


  template <
    class someFilter,
    class ...options >
  std::unordered_map< std::string , typename someFilter::functionType >
  SelectionVariableNamesToFunctions(Belle2::Filter < Belle2::OperatorNot,
                                    someFilter, options... >)
  {
    return SelectionVariableNamesToFunctions(someFilter());
  }

  template <
    class FilterA,
    class FilterB,
    class ...options
    >
  std::unordered_map< std::string , typename FilterA::functionType >
  SelectionVariableNamesToFunctions(Belle2::Filter<Belle2::OperatorAnd,
                                    FilterA, FilterB, options...>)
  {
    auto result = SelectionVariableNamesToFunctions(FilterA());
    auto resultB = SelectionVariableNamesToFunctions(FilterB());
    result.insert(resultB.begin(), resultB.end());
    return result;
  }


  template <
    class FilterA,
    class FilterB,
    class ...options
    >
  std::unordered_map< std::string , typename FilterA::functionType >
  SelectionVariableNamesToFunctions(Belle2::Filter< Belle2::OperatorOr,
                                    FilterA, FilterB, options...>)
  {
    auto result  = SelectionVariableNamesToFunctions(FilterA());
    auto resultB = SelectionVariableNamesToFunctions(FilterB());
    result.insert(resultB.begin(), resultB.end());
    return result;
  }


}
