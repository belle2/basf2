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

    /** Namespace scope to contain the enumeration values of the enumeration.
     *  Similar to enum class in the way it does not pollute the namespace but parsable by cint.
     *  In case cling is able to parse emum classes remove the namespace StereoType_c,
     *  exchange _c:: with ::c_ and prefix the constants with c_ like the rest of the framework.
     **/
    namespace StereoType_c {
      /// Type for the stereo property of the wire
      enum StereoType {
        /// Constant for an axial wire
        Axial = 0,

        /// Constant for an stereo wire in the V configuration
        StereoV = -1,

        /// Constant for an stereo wire in the U configuration
        StereoU = 1,

        /// Constant for an invalid stereo information
        Invalid = -127,
      };
    };

    /// Importing stereoType from the namespace containing its constants.
    typedef StereoType_c::StereoType StereoType;

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
      if (itBegin == itEnd) return StereoType_c::Invalid; // empty case
      const StereoType stereoType = (*itBegin)->getStereoType();
      for (const auto& hit : hits) {
        if (hit->getStereoType() != stereoType) {
          return StereoType_c::Invalid;
        };
      }
      return stereoType;
    }

  } // namespace TrackFindingCDC

} // namespace Belle2

