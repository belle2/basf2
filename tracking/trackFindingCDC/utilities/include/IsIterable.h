/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
