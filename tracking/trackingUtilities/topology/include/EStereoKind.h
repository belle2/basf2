/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
