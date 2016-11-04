/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tuple>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Function serves as a placeholder for a variadic list of statements to be evaluated.
     *  The variadic syntax of C++ allows for variadic calls to a function which first map each element in an expression
     *  However, to evaluate statements for their side effects only one still has to wrap the statements by a dummy
     *  function call. This function provides this dummy structure, since it only takes a variadic argument list,
     *  but has no instructions on its own.
     *  Loosely inspired from http://en.wikipedia.org/wiki/Variadic_templates
     */
    template <class... Ts>
    static inline void evalVariadic(Ts&& ... expressions __attribute__((unused)))
    {
    }

    /** Structure to serve as a placeholder for a variadic initializer list of statements to be evaluated.
     *  Similar to the evalVariadic functions this structure evaluates as variadic list of statements, but
     *  uses a initializer list instead of a function call.
     *  Loosely inspired from http://en.wikipedia.org/wiki/Variadic_templates
     */
    struct EvalVariadic {
      /// Constructor taking the variadic initalizer list.
      template <class... Ts>
      explicit EvalVariadic(Ts&& ... expressions __attribute__((unused)))
      {
      }
    };


    /** Helper type to take the first type of a variadic sequence of types */
    template<class... ATypes>
    struct FirstTypeImpl {
    };

    /** Specialisation where there is exactly one type given */
    template<>
    struct FirstTypeImpl<> {
    };

    /** Specialisation where there is exactly one type given */
    template<class AType>
    struct FirstTypeImpl<AType> {
      /// Result first item of the variadic sequence.
      using Type = AType;
    };

    /** Specialisation where there is more than one type given */
    template<class AType, class... ATypes>
    struct FirstTypeImpl<AType, ATypes...> {
      /// Result first item of the variadic sequence.
      using Type = AType;
    };

    /** Short cut meta function to take the first type of a variadic sequence */
    template<class... ATypes>
    using FirstType = typename FirstTypeImpl<ATypes...>::Type;

    /** Looks up, at which index the given Type can be found in a tuple.
     *  Amounts to a type inheriting from std::integral_constant
     */
    template<class AType, class ATuple>
    struct GetIndexInTuple {};

    /// Specialisation to terminate the recursion in case it was not found.
    template<class AType>
    struct GetIndexInTuple<AType, std::tuple<> > :
      std::integral_constant<std::size_t, 0> {};

    /// Specialisation for the case that the first type in the tuple is not the AType asked for. Recursion case.
    template<class AType, class AHeadType, class... ATailTypes>
    struct GetIndexInTuple<AType, std::tuple<AHeadType, ATailTypes...> > :
      std::integral_constant < std::size_t, GetIndexInTuple<AType, std::tuple<ATailTypes...> >::value + 1 > {};

    /// Specialisation for the case that the first type in the tuple is equal to the AType asked for. Recursion end.
    template<class AType, class... ATailTypes>
    struct GetIndexInTuple <AType, std::tuple<AType, ATailTypes...> > :
      std::integral_constant< std::size_t, 0> {};

    /// Looks in a tuple if the asked type can be found.
    template<class T, class ATuple>
    using TypeInTuple =
      std::integral_constant < bool, (GetIndexInTuple<T, ATuple>::value < std::tuple_size<ATuple>::value) >;

  }
}
