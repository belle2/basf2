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
    template<typename... Args>
    static inline void evalVariadic(Args&& ...) {}

    /** Structure to serve as a placeholder for a variadic initializer list of statements to be evaluated.
     *  Similar to the evalVariadic functions this structure evaluates as variadic list of statements, but
     *  uses a initializer list instead of a function call.
     *  Loosely inspired from http://en.wikipedia.org/wiki/Variadic_templates
     */
    struct EvalVariadic {
      /// Constructor taking the variadic initalizer list.
      template<typename ...T>
      explicit EvalVariadic(T...) {}
    };


    /** Helper type to take the first type of a variadic sequence of types */
    template<class... Types_>
    struct FirstTypeImpl {
    };

    /** Specialisation where there is exactly one type given */
    template<>
    struct FirstTypeImpl<> {
    };

    /** Specialisation where there is exactly one type given */
    template<class Type_>
    struct FirstTypeImpl<Type_> {
      typedef Type_ Type;
    };

    /** Specialisation where there is more than one type given */
    template<class Type_, class... Types_>
    struct FirstTypeImpl<Type_, Types_...> {
      typedef Type_ Type;
    };

    /** Short cut meta function to take the first type of a variadic sequence */
    template<class... Types_>
    using FirstType = typename FirstTypeImpl<Types_...>::Type;

    /** Short form for std::enable_f */
    template<bool cond, class T>
    using EnableIf = typename std::enable_if<cond, T>::type;

    /** Looks up, at which index the given Type can be found in a tuple.
     *  Amounts to a type inheriting from std::integral_constant
     */
    template<class Type, class Tuple>
    struct GetIndexInTuple {};

    /// Specialisation for the case that the first type in the tuple is not the Type asked for. Recursion case.
    template<class Type, class HeadType, class ... TailTypes>
    struct GetIndexInTuple<Type, std::tuple<HeadType, TailTypes...> > :
      std::integral_constant < std::size_t, GetIndexInTuple<Type, std::tuple<TailTypes...> >::value + 1 > {};

    /// Specialisation for the case that the first type in the tuple is equal to the Type asked for. Recursion end.
    template<class Type, class... TailTypes>
    struct GetIndexInTuple <Type, std::tuple<Type, TailTypes...> > :
      std::integral_constant< std::size_t, 0> {};

  } // end namespace TrackFindingCDC
} // namespace Belle2
