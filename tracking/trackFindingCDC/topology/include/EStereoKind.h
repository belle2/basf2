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

#include <tracking/trackFindingCDC/utilities/Functional.h>

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

    /// Generic functor to get the stereo kind from an object.
    struct GetEStereoKind {

      /// Constant returned on an invalid get operation
      static const EStereoKind c_Invalid = EStereoKind::c_Invalid;

      /// Returns the stereo kind of an object.
      template<class T, class SFINAE = decltype(&T::getStereoKind)>
      EStereoKind operator()(const T& t) const
      {
        return t.getStereoKind();
      }
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
       *  EStereoKind::c_Invalid if there is no common stereo kind or the container is empty.
       */
      template<class AHits>
      static EStereoKind getCommon(const AHits& hits)
      {
        return Common<MayIndirectTo<GetEStereoKind>>()(hits);
      }

      /// Returns the superlayer of an object
      template<class T>
      static EStereoKind getFrom(const T& t)
      {
        return MayIndirectTo<GetEStereoKind>()(t);
      }
    };
  }
}
