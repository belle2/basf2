/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <iterator>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type function to extract the iterator type from a range of values
    template<class Ts>
    using GetIterator = typename std::remove_reference < decltype(std::begin(std::declval<Ts&>())) >::type;

  }
}
