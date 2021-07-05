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

#include <cstdlib> // for abs
#include <climits>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace hiding the constants of the enum
    namespace NRotation {

      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum ERotation : signed short {

        /// Constant for counter clockwise oriented circle
        c_CounterClockwise = 1,

        /// For the orientation of a line
        c_Unknown = 0,

        /// Constant for clockwise oriented circle
        c_Clockwise = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,
      };

      /// Return the reversed rotation. Leaves ERotation::c_Invalid the same.
      inline ERotation reversed(ERotation eRotation)
      { return ERotation(-eRotation); }

      /// Check whether the given enum instance is one of the valid values.
      inline bool isValid(ERotation eRotation)
      { return std::abs(eRotation) <= 1; }
    }

    /// Importing only the enum but not the constants
    using ERotation = NRotation::ERotation;
  }
}
