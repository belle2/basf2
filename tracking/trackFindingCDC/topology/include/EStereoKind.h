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

#include <tracking/trackFindingCDC/utilities/FunctorTag.h>

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
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Returns the stereo kind of an object.
      template<class T, class SFINAE = decltype(&T::getStereoKind)>
      EStereoKind operator()(const T& t) const
      {
        return t.getStereoKind();
      }
    };
  }
}
