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
    namespace NIncDec {

      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum EIncDec : signed short {

        /// Constant for a variable that increases
        c_Increasing = 1,

        /// Constant for a variable that is constant
        c_Constant = 0,

        /// Constant for a variable that decreases
        c_Decreasing = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,
      };

      /// Return the reversed increasing decreasing indicator. Leaves EIncDec::c_Invalid the same.
      inline EIncDec reversed(EIncDec eIncDec)
      { return EIncDec(-eIncDec); }

      /// Check whether the given enum instance is one of the valid values.
      inline bool isValid(EIncDec eIncDec)
      { return std::abs(eIncDec) <= 1; }
    }

    /// Importing only the enum but not the constants
    using EIncDec = NIncDec::EIncDec;

  }
}
