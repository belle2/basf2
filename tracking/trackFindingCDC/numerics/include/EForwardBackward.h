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
    namespace NForwardBackward {

      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum EForwardBackward : signed short {

        /// Constant for a situation where something is more forward
        c_Forward = 1,

        /// Constant for an not yet determined forward or backward arrangement
        c_Unknown = 0,

        /// Constant for a situation where something is more backward
        c_Backward = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,
      };

      /// Return the reversed forward backward indicator. Leaves EForwardBackward::c_Invalid the same.
      inline EForwardBackward reversed(EForwardBackward eForwardBackward)
      { return EForwardBackward(-eForwardBackward); }

      /// Check whether the given enum instance is one of the valid values.
      inline bool isValid(EForwardBackward eForwardBackward)
      { return std::abs(eForwardBackward) <= 1; }
    }

    /// Importing only the enum but not the constants
    using EForwardBackward = NForwardBackward::EForwardBackward;

  }
}
