/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
