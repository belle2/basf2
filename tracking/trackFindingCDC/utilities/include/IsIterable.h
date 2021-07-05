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

#include <tracking/trackFindingCDC/utilities/IsDetected.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type predicate to check if Ts is iterable by means of std::begin().
    template<class Ts>
    constexpr bool isIterable()
    {
      return isDetected<GetIterator, Ts>();
    }
  }
}
