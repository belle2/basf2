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

#include <tracking/trackFindingCDC/numerics/ESign.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <iterator>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type for the stereo property of the wire
    enum class EStereoType {
      /// Constant for an axial wire
      c_Axial = 0,

      /// Constant for an stereo wire in the V configuration
      c_StereoV = -1,

      /// Constant for an stereo wire in the U configuration
      c_StereoU = 1,

      /// Constant for an invalid stereo information
      c_Invalid = -999,
    };

    /** Returns the common stereo type of hits in a container.
     *  INVALID_STEREOTYPE if there is no common super layer or the container is empty.
     */
    template<class AHits>
    EStereoType getStereoType(const AHits& hits)
    {
      using Hit = ValueType<AHits>;
      auto getStereoTypeOfHit = [](const Hit & hit) {return hit->getStereoType();};
      return common(hits, getStereoTypeOfHit, EStereoType::c_Invalid);
    }

  } // namespace TrackFindingCDC

} // namespace Belle2
