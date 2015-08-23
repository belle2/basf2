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

#include <tracking/trackFindingCDC/numerics/SignType.h>
#include <iterator>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type for the stereo property of the wire
    typedef SignType StereoType;

    /// Constant for an axial wire
    const StereoType AXIAL = 0;

    /// Constant for an stereo wire in the U configuration
    const StereoType STEREO_U = 1;

    /// Constant for an stereo wire in the V configuration
    const StereoType STEREO_V = -1;

    /// Constant for an invalid stereo information
    const StereoType INVALID_STEREOTYPE = -127;

    /** Returns the common stereo type of hits in a container.
     *  INVALID_STEREOTYPE if there is no common super layer or the container is empty.
     */
    template<class Hits>
    StereoType getStereoType(const Hits& hits)
    {
      using std::begin;
      using std::end;
      auto itBegin = begin(hits);
      auto itEnd = end(hits);
      if (itBegin == itEnd) return INVALID_STEREOTYPE; // empty case
      const StereoType stereoType = (*itBegin)->getStereoType();
      for (const auto& hit : hits) {
        if (hit->getStereoType() != stereoType) {
          return INVALID_STEREOTYPE;
        };
      }
      return stereoType;
    }

  } // namespace TrackFindingCDC

} // namespace Belle2

