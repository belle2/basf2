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

#include <iterator>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type function to extract the bare value type from a range of values
    template<class Ts>
    using GetValueType = typename std::remove_reference < decltype(*std::begin(std::declval<Ts&>())) >::type;

  } // end namespace TrackFindingCDC
} // end namespace Belle2
