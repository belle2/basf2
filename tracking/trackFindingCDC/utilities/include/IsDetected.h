/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Helper template that always amounts to void
    template<typename... Ts>
    using ToVoid = void;

    /// Implementation of the detection idiom - base case is not detected
    template<typename, template<typename...> class, typename...>
    struct DetectPropertyImpl : std::false_type {};

    /// Implementation of the detection idiom - specialisation for the detected case.
    template<template<class...> class AToPredicate, typename... Ts>
    struct DetectPropertyImpl<ToVoid<AToPredicate<Ts...>>, AToPredicate, Ts...> : std::true_type {};

    /**
     *  Detection idiom checking whether the given template type instatiates well with the given types.
     *
     *  Checks whether the ToPredicate results in a valid type.
     *  It can be used to detect whether a given type (or a list of types) supports
     *  an operation or property of interest.
     *
     *  For a typical application see IsApplicable.h and IsIterable.h
     */
    template<template<class...> class AToPredicate, typename... Ts>
    constexpr bool isDetected()
    {
      return DetectPropertyImpl<ToVoid<>, AToPredicate, Ts...>::value;
    }
  }
}
