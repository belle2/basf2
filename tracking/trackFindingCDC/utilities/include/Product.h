/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <type_traits>
#include <cstddef>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Template class for compile time computation of products
    template<std::size_t... factors>
    struct Product;

    /** Template class specilisation for compile time computation of products.
     *  Recursion case. */
    template<std::size_t factor, std::size_t... factors>
    struct Product<factor, factors...> :
      std::integral_constant<std::size_t, factor* Product<factors...>::value > {};

    /** Template class specilisation for compile time computation of products.
     *  Terminating case. */
    template<>
    struct Product<> :
      std::integral_constant<std::size_t, 1> {};

  }
}
