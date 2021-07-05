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
    namespace NRightLeft {

      /// Enumeration to represent the distinct possibilities of the right left passage
      enum ERightLeft : signed short {

        /// Constant for right passage
        c_Right = 1,

        /// Constant for an not yet determined right left passage
        c_Unknown = 0,

        /// Constant for left passage
        c_Left = -1,

        /// Constant for an invalid passage information
        c_Invalid = SHRT_MIN,
      };

      /// Return the reversed right left indicator. Leaves ERightLeft::c_Invalid the same.
      inline ERightLeft reversed(ERightLeft eRightLeft)
      { return ERightLeft(-eRightLeft); }

      /// Check whether the given enum instance is one of the valid values.
      inline bool isValid(ERightLeft eRightLeft)
      { return std::abs(eRightLeft) <= 1; }
    }

    /// Importing only the enum but not the constants
    using ERightLeft = NRightLeft::ERightLeft;

  }
}
