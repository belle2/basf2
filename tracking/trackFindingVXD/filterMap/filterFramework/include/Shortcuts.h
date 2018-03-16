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

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Range.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/LowerBoundedSet.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/UpperBoundedSet.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/SingleElementSet.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/ClosedLowerBoundedSet.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/ClosedUpperBoundedSet.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/ClosedRange.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/VoidObserver.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

#include <type_traits>


namespace Belle2 {
  /** Creates a Filters with an upper bound < on the provided variable
   * Var < lowerBound
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @tparam types : other types
   * @param upperBound : upper bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic, typename ... types>
  // the following typename expands to Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  // otherwise SFINAE ( Substitution Failure Is Not An Error )
  typename
  std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value&&
  std::is_arithmetic<Arithmetic>::value,
      Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver >>::type
      operator < (const Var&, Arithmetic upperBound)
  {
    return Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>(UpperBoundedSet<Arithmetic> (upperBound));
  }


  /** Creates a Filters with a closed upper bound <= on the provided variable
   * Var <= lowerBound
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @tparam types : other types
   * @param upperBound : upper bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic, typename ... types>
  // the following typename expands to Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  // otherwise SFINAE ( Substitution Failure Is Not An Error )
  typename
  std::enable_if <
  std::is_base_of< SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value&&
  std::is_arithmetic<Arithmetic>::value,
      Filter<Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver> >::type
      operator <= (const Var&, Arithmetic upperBound)
  {
    return Filter<Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver>(ClosedUpperBoundedSet<Arithmetic> (upperBound));
  }


  /** Creates a Filters with an lower bound > on the provided variable
   * Var > lowerBound
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param lowerBound : lower bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator > (const Var&, Arithmetic lowerBound)
  {
    return Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver >(LowerBoundedSet<Arithmetic> (lowerBound));
  }


  /** Creates a Filters with a closed lower bound >= on the provided variable
   * Var >= lowerBound
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param lowerBound : lower bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator >= (const Var&, Arithmetic  lowerBound)
  {
    return Filter<Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver >(ClosedLowerBoundedSet<Arithmetic> (lowerBound));
  }


  /** Creates a Filters with an upper bound < on the provided variable
   * upperBound > Var
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param upperBound : upper bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver> >::type
  operator > (Arithmetic upperBound, const Var&)
  {
    return Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>(UpperBoundedSet<Arithmetic> (upperBound));
  }


  /** Creates a Filters with a closed upper bound <= on the provided variable
   * upperBound >= Var
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param upperBound : upper bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, UpperBoundedSet<Arithmetic>, VoidObserver>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver> >::type
  operator >= (Arithmetic upperBound, const Var&)
  {
    return Filter<Var, ClosedUpperBoundedSet<Arithmetic>, VoidObserver>(ClosedUpperBoundedSet<Arithmetic> (upperBound));
  }


  /** Creates a Filters with an lower bound > on the provided variable
   * lowerBound < Var
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param lowerBound : lower bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, LowerBoundedSet<Arithmetic>, Observer>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver> >::type
  operator < (Arithmetic lowerBound, const Var&)
  {
    return Filter<Var, LowerBoundedSet<Arithmetic>, VoidObserver>(LowerBoundedSet<Arithmetic> (lowerBound));
  }


  /** Creates a Filters with a closed lower bound >= on the provided variable
   * lowerBound <= Var
   * @tparam Var : variable type to use for the filter
   * @tparam Arithmetic : type of the return value of the variable function
   * @param lowerBound : lower bound value to be checked by the filter
   * @return the created filter
   */
  template <class Var, class Arithmetic>
  // the following typename expands to Filter<Var, LowerBoundedSet<Arithmetic>, Observer>
  // if Var derives from SelectionVariable and Arithmetic is an integer or a floating point type
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<Arithmetic>::value,
  Filter<Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver >>::type
  operator <= (Arithmetic lowerBound, const Var&)
  {
    return Filter<Var, ClosedLowerBoundedSet<Arithmetic>, VoidObserver>(ClosedLowerBoundedSet<Arithmetic> (lowerBound));
  }


  /** Creates a Filters to compare a variable against a given value
   * Var == Val;
   * @tparam Var : variable type to use for the filter
   * @tparam Val : value type to compare variable against
   * @param v : value to compare variable against
   * @return the created filter
   */
  template <class Var, class Val>
  // the following typename expands to Filter<Var, SingleElementSet<Val>, VoidObserver>
  // if Var derives from SelectionVariable
  typename std::enable_if
  <std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs,  typename Var::variableType>, Var>::value,
  Filter<Var, SingleElementSet<Val>, VoidObserver> >::type
  operator == (const Var&, Val v)
  {
    return Filter<Var, SingleElementSet<Val>, VoidObserver>(SingleElementSet<Val> (v));
  }


  /** Creates a Filters to compare a variable against a given value
   * Val == Var;
   * @tparam Var : variable type to be used for the filter
   * @tparam Val : value type to compare variable against
   * @param val : value to compare variable against
   * @param var : variable
   * @return the created filter
   */
  template <class Var, class Val>
  // the following typename expands to Filter<Var, SingleElementSet<Val>, VoidObserver>
  // if Var derives from SelectionVariable
  typename std::enable_if
  <std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value,
  Filter<Var, SingleElementSet<Val>, VoidObserver> >::type
  operator == (Val val, const Var& var)
  {
    return var == val;
  }


  /** Adding upper bound to filter with lower bound to create a filter with an allowed range
   * between lower and upper bound.
   * (Var with lowerBound) < upperBound
   * @tparam Var : variable type to be used for the filter
   * @tparam ArithmeticLower : type of lower bound
   * @tparam ArithmeticUpper : type of upper bound
   * @tparam Observer : observer type
   * @param filter : filter with lower bound to which the upper bound shall be added
   * @param upperBound : value of upper bound
   * @return filter with range allowing values between lower and upper bound
   */
  template <class Var, class ArithmeticLower, class ArithmeticUpper, class Observer>
  // the following typename expands to Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<ArithmeticLower>::value
  && std::is_arithmetic<ArithmeticUpper>::value ,
  Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer >>::type
  operator < (const Filter<Var, LowerBoundedSet<ArithmeticLower>, Observer>& filter, ArithmeticUpper upperBound)
  {
    return Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer>
           (Range<ArithmeticLower, ArithmeticUpper> (filter.getRange().getInf(), upperBound));
  }


  /** Adding lower bound to filter with upper bound to create a filter with an allowed range
   * between lower and upper bound.
   * (Var with upperBound) > lowerBound
   * @tparam Var : variable type to be used for the filter
   * @tparam ArithmeticLower : type of lower bound
   * @tparam ArithmeticUpper : type of upper bound
   * @tparam Observer : observer type
   * @param filter : filter with lower bound to which the upper bound shall be added
   * @param lowerBound : value of lower bound
   * @return filter with range allowing values between lower and upper bound
   */
  template <class Var, class ArithmeticLower, class ArithmeticUpper, class Observer>
  // the following typename expands to Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<ArithmeticLower>::value
  && std::is_arithmetic<ArithmeticUpper>::value ,
  Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer >>::type
  operator > (const Filter<Var, UpperBoundedSet<ArithmeticUpper>, Observer>& filter, ArithmeticLower lowerBound)
  {
    return Filter<Var, Range<ArithmeticLower, ArithmeticUpper>, Observer>
           (Range<ArithmeticLower, ArithmeticUpper> (lowerBound , filter.getRange().getSup()));
  }


  /** Adding closed upper bound to filter with closed lower bound to create a filter with an allowed closed range
   * between lower and upper bound.
   * (Var with closed lowerBound) <= upperBound
   * @tparam Var : variable type to be used for the filter
   * @tparam ArithmeticLower : type of lower bound
   * @tparam ArithmeticUpper : type of upper bound
   * @tparam Observer : observer type
   * @param filter : filter with lower bound to which the upper bound shall be added
   * @param upperBound : value of upper bound
   * @return filter with closed range allowing values between lower and upper bound
   */
  template <class Var, class ArithmeticLower, class ArithmeticUpper, class Observer>
  // the following typename expands to Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<ArithmeticLower>::value
  && std::is_arithmetic<ArithmeticUpper>::value ,
  Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer >>::type
  operator <= (const Filter<Var, ClosedLowerBoundedSet<ArithmeticLower>, Observer>& filter, ArithmeticUpper upperBound)
  {
    return Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer>
           (ClosedRange<ArithmeticLower, ArithmeticUpper> (filter.getRange().getInf(), upperBound));
  }


  /** Adding closed lower bound to filter with closed upper bound to create a filter with an allowed closed range
   * between lower and upper bound.
   * (Var with closed upperBound) >= lowerBound
   * @tparam Var : variable type to be used for the filter
   * @tparam ArithmeticLower : type of lower bound
   * @tparam ArithmeticUpper : type of upper bound
   * @tparam Observer : observer type
   * @param filter : filter with lower bound to which the upper bound shall be added
   * @param lowerBound : value of lower bound
   * @return filter with closed range allowing values between lower and upper bound
   */
  template <class Var, class ArithmeticLower, class ArithmeticUpper, class Observer>
  // the following typename expands to Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer>
  // if Var derives from SelectionVariable and Arithmetics are integer or floating point types
  typename std::enable_if <
  std::is_base_of<SelectionVariable<typename Var::argumentType, Var::c_Nargs, typename Var::variableType>, Var>::value
  && std::is_arithmetic<ArithmeticLower>::value
  && std::is_arithmetic<ArithmeticUpper>::value ,
  Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer >>::type
  operator >= (const Filter<Var, ClosedUpperBoundedSet<ArithmeticUpper> , Observer>& filter, ArithmeticLower lowerBound)
  {
    return Filter<Var, ClosedRange<ArithmeticLower, ArithmeticUpper>, Observer>
           (ClosedRange<ArithmeticLower, ArithmeticUpper> (lowerBound , filter.getRange().getSup()));
  }
}
