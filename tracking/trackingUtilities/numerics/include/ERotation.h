/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
