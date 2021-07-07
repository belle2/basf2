/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/IsDetected.h>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Typical application of the detection idiom
     *
     *  Tests whether a Functor can be called with the given argument types.
     */
    template <class AFunctor, typename... Ts>
    constexpr bool isApplicable()
    {
      return isDetected<std::result_of_t, AFunctor(Ts...)>();
    }
  }
}
