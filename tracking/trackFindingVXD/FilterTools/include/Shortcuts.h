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

#include "tracking/trackFindingVXD/FilterTools/SelectionVariable.h"
#include "tracking/trackFindingVXD/FilterTools/Range.h"
#include "tracking/trackFindingVXD/FilterTools/LowerBoundedSet.h"
#include "tracking/trackFindingVXD/FilterTools/UpperBoundedSet.h"
#include "tracking/trackFindingVXD/FilterTools/SingleElementSet.h"

#include "tracking/trackFindingVXD/FilterTools/ClosedLowerBoundedSet.h"
#include "tracking/trackFindingVXD/FilterTools/ClosedUpperBoundedSet.h"
#include "tracking/trackFindingVXD/FilterTools/ClosedRange.h"

#include "tracking/trackFindingVXD/FilterTools/VoidObserver.h"
#include "tracking/trackFindingVXD/FilterTools/Filter.h"



#include <type_traits>

namespace Belle2 {

  template < class Var, class Arithmetic, typename ... types  >
  // the following typename expands to Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  // otherwise SFINAE ( Substitution Failure Is Not An Error )
  typename
  std::enable_if < std::is_base_of< SelectionVariable< typename Var::argumentType, typename Var::variableType >, Var >::value&&
  std::is_arithmetic< Arithmetic >::value,
      Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver> >::type
      operator < (const Var&, Arithmetic  upperBound)
  {
    return Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver >(UpperBoundedSet<Arithmetic> (upperBound));
  }

  template < class Var, class Arithmetic, typename ... types  >
  // the following typename expands to Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  // otherwise SFINAE ( Substitution Failure Is Not An Error )
  typename
  std::enable_if < std::is_base_of< SelectionVariable< typename Var::argumentType, typename Var::variableType >, Var >::value&&
  std::is_arithmetic< Arithmetic >::value,
      Filter< Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver> >::type
      operator <= (const Var&, Arithmetic  upperBound)
  {
    return Filter< Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver >(ClosedUpperBoundedSet<Arithmetic> (upperBound));
  }

  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator > (const Var&, Arithmetic  lowerBound)
  {
    return Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver >(LowerBoundedSet<Arithmetic> (lowerBound));
  }

  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator >= (const Var&, Arithmetic  lowerBound)
  {
    return Filter< Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver >(ClosedLowerBoundedSet<Arithmetic> (lowerBound));
  }

  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver> >::type
  operator > (Arithmetic  upperBound, const Var&)
  {
    return Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver >(UpperBoundedSet<Arithmetic> (upperBound));
  }

  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver> >::type
  operator >= (Arithmetic  upperBound, const Var&)
  {
    return Filter< Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver >(ClosedUpperBoundedSet<Arithmetic> (upperBound));
  }


  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, LowerBoundedSet<Arithmetic>, Observer>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator < (Arithmetic  lowerBound, const Var&)
  {
    return Filter< Var, LowerBoundedSet<Arithmetic>, VoidObserver >(LowerBoundedSet<Arithmetic> (lowerBound));
  }

  template < class Var, class Arithmetic  >
  // the following typename expands to Filter< Var, LowerBoundedSet<Arithmetic>, Observer>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< Arithmetic >::value,
  Filter< Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator <= (Arithmetic  lowerBound, const Var&)
  {
    return Filter< Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver >(ClosedLowerBoundedSet<Arithmetic> (lowerBound));
  }

  template < class Var, class Val  >
  // the following typename expands to Filter< Var, SingleElementSet<Val>, VoidObserver>
  // if Var derives from SelectionVariable
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value,
           Filter< Var, SingleElementSet<Val>, VoidObserver> >::type
           operator == (const Var&,  Val v)
  {
    return Filter< Var, SingleElementSet<Val>, VoidObserver >(SingleElementSet<Val> (v));
  }


  template < class Var, class Val  >
  // the following typename expands to Filter< Var, SingleElementSet<Val>, VoidObserver>
  // if Var derives from SelectionVariable
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value,
           Filter< Var, SingleElementSet<Val>, VoidObserver> >::type
           operator == (Val val, const Var& var)
  {
    return var == val;
  }




  template < class Var, class ArithmeticLower, class ArithmeticUpper, class Observer  >
  // the following typename expands to Filter< Var, Range< ArithmeticLower, ArithmeticUpper >, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< ArithmeticLower >::value
  && std::is_arithmetic< ArithmeticUpper >::value ,
  Filter< Var, Range< ArithmeticLower, ArithmeticUpper >, Observer> >::type
  operator < (Filter< Var, LowerBoundedSet< ArithmeticLower > , Observer>  filter ,  ArithmeticUpper  upperBound)
  {
    return Filter< Var, Range<ArithmeticLower, ArithmeticUpper>, Observer >
           (Range< ArithmeticLower, ArithmeticUpper> (filter.getRange().getInf() , upperBound));
  }


  template < class Var, class ArithmeticLower, class ArithmeticUpper, class Observer  >
  // the following typename expands to Filter< Var, Range< ArithmeticLower, ArithmeticUpper >, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< ArithmeticLower >::value
  && std::is_arithmetic< ArithmeticUpper >::value ,
  Filter< Var, Range< ArithmeticLower, ArithmeticUpper >, Observer> >::type
  operator > (Filter< Var, UpperBoundedSet< ArithmeticUpper > , Observer>  filter ,  ArithmeticLower  lowerBound)
  {
    return Filter< Var, Range<ArithmeticLower, ArithmeticUpper>, Observer >
           (Range< ArithmeticLower, ArithmeticUpper> (lowerBound , filter.getRange().getSup()));
  }

  template < class Var, class ArithmeticLower, class ArithmeticUpper, class Observer  >
  // the following typename expands to Filter< Var, ClosedRange< ArithmeticLower, ArithmeticUpper >, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< ArithmeticLower >::value
  && std::is_arithmetic< ArithmeticUpper >::value ,
  Filter< Var, ClosedRange< ArithmeticLower, ArithmeticUpper >, Observer> >::type
  operator <= (Filter< Var, ClosedLowerBoundedSet< ArithmeticLower > , Observer>  filter ,  ArithmeticUpper  upperBound)
  {
    return Filter< Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer >
           (ClosedRange< ArithmeticLower, ArithmeticUpper> (filter.getRange().getInf() , upperBound));
  }


  template < class Var, class ArithmeticLower, class ArithmeticUpper, class Observer  >
  // the following typename expands to Filter< Var, ClosedRange< ArithmeticLower, ArithmeticUpper >, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if < std::is_base_of< SelectionVariable<typename Var::argumentType, typename Var::variableType>, Var>::value
  && std::is_arithmetic< ArithmeticLower >::value
  && std::is_arithmetic< ArithmeticUpper >::value ,
  Filter< Var, ClosedRange< ArithmeticLower, ArithmeticUpper >, Observer> >::type
  operator >= (Filter< Var, ClosedUpperBoundedSet< ArithmeticUpper > , Observer>  filter ,  ArithmeticLower  lowerBound)
  {
    return Filter< Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer >
           (ClosedRange< ArithmeticLower, ArithmeticUpper> (lowerBound , filter.getRange().getSup()));
  }



}

