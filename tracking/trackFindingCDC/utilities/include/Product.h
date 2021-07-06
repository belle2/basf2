/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
