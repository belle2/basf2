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

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/GetValueType.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for the stereo property of the wire
    enum class EStereoKind {
      /// Constant for an axial wire
      c_Axial = 0,

      /// Constant for an stereo wire in the V configuration
      c_StereoV = -1,

      /// Constant for an stereo wire in the U configuration
      c_StereoU = 1,

      /// Constant for an invalid stereo information
      c_Invalid = -999,
    };

    /**
     *  This is a utility class for the free EStereoKind type.
     *  It provides the basic methods to operate on the EStereoKind numbers.
     */
    struct EStereoKindUtil {

      /// Utility classes should not be instantiated
      EStereoKindUtil() = delete;

      /**
       *  Returns the common stereo type of hits in a container.
       *  EStereoKind::c_Invalid if there is no common super layer or the container is empty.
       */
      template<class AHits>
      static EStereoKind getCommon(const AHits& hits)
      {
        using Hit = GetValueType<AHits>;
        return common(hits, getFrom<Hit>, EStereoKind::c_Invalid);
      }

      /** Returns the superlayer of an object */
      template<class T>
      static EStereoKind getFrom(const T& t)
      { return getFromImpl(t, 0); }

    private:
      /** Returns the stereo kind of an object. Favored option. */
      template<class T>
      static auto getFromImpl(const T& t, int) -> decltype(t.getStereoKind())
      { return t.getStereoKind(); }

      /** Returns the stereo kind of an object. Unfavored option. */
      template<class T>
      static auto getFromImpl(const T& t, long) -> decltype(t->getStereoKind())
      { return &*t == nullptr ? EStereoKind::c_Invalid  : t->getStereoKind(); }

    };

  }

}
