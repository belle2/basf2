/*******************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                          *
 * Copyright(C) 2013 - Belle II Collaboration                                  *
 *                                                                             *
 * Author: The Belle II Collaboration                                          *
 * Contributors: Eugenio Paoloni                                               *
 *                                                                             *
 * This software is provided "as is" without any warranty.                     *
 ******************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/sectorMap/filterFramework/Filter.h>
#include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>

#include <string>
#include <map>


namespace Belle2 {
  /**
   * This function pass through all the variables in a filter,
   * evaluates theirs value and collect them in \param collectedData.
   *
   * Simple case: a filter based on a single SelectionVariable Var
   * and a range.
   */
  template <
    class Var,
    class Argument1, typename ... Arguments, class ReturnType,
    class Range, typename ... Optionals  >

  // the following typename expands to void
  // if Variable derives from SelectionVariable< Argument, ReturnType >
  // and if all the types in Arguments
  // otherwise SFINAE ( Substitution Failure Is Not An Error )
  typename
  std::enable_if < // if Var derives form SelectionVariable< ... >
  std::is_base_of <
  SelectionVariable< typename Var::argumentType,
                     typename Var::variableType >, Var >::value
                     &&
                     std::is_same< ReturnType, typename Var::variableType >::value
                     &&
                     std::is_same< Argument1, typename Var::argumentType >::value
                     &&
                     all_same< Argument1, Arguments... >::value
                     ,
                     void
                     >
                     CollectAllFilterVariables(Belle2::Filter< Var, Range, Optionals...>,
                                               std::map<std::string, ReturnType >& collectedData,
                                               const Argument1& outer,
                                               const Arguments& ... inners)
  {
    Var variable;
    collectedData.insert(std::pair< std::string, ReturnType >
                         (variable.name(),
                          variable.value(outer, inners ...)));
  }

  template< class ... args>
  void CollectAllFilterVariables(args ...) {};

  /**
   * This function pass through all the variables in a filter,
   * evaluates theirs value and collect it.
   * If the filter is derived using a unary boolean
   * then dig into it...
   */
  template< class booleanUnaryOperator,
            class Argument1, typename ... Arguments, class Return,
            typename ... types1, class observer >
  void
  CollectAllFilterVariables(Filter< booleanUnaryOperator,
                            Filter< types1...>, observer>,
                            std::map<std::string, Return >& collectedData,
                            const Argument1& outer,
                            const Arguments& ... inners)
  {
    CollectAllFilterVariables(Filter< types1...>() , collectedData,
                              outer, inners ...);
  }

  /**
   * This function pass through all the variables in a filter,
   * evaluates theirs value and collect it.
   * If the filter is derived using a binary boolean operator
   * then dig into both operands...
   */
  template <
    class booleanBinaryOperator,
    class Argument1, typename ... Arguments,
    class Return,
    typename ... types1,
    typename ... types2,
    class observer >
  void
  CollectAllFilterVariables(Filter< booleanBinaryOperator,
                            Filter< types1...>,
                            Filter< types2...>,
                            observer>,
                            std::map<std::string, Return >& collectedData,
                            const Argument1& outer,
                            const Arguments& ... inners)
  {
    CollectAllFilterVariables(Filter< types1...>() , collectedData,
                              outer, inners ...);
    CollectAllFilterVariables(Filter< types2...>() , collectedData,
                              outer, inners ...);
  }

}
