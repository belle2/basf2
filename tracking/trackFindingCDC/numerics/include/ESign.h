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

#include <cstdlib> // for abs
#include <cmath> // for isnan
#include <climits> // for SHRT_MIN

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the constants of the ESign enumeration.
    namespace NSign {

      /// Enumeration for the distinct sign values of floating point variables
      enum ESign : signed short {
        /// Constant for plus sign.
        c_Plus = 1,

        /// Constant for minus sign.
        c_Minus = -1,

        /// Constant for undefined sign.
        c_Zero = 0,

        /// Constant for invalid sign, e.g. the sign of NAN.
        c_Invalid = SHRT_MIN,
      };

      /// Return the opposite sign. Leaves ESign::c_Invalid the same.
      inline ESign oppositeSign(ESign s)
      { return static_cast<ESign>(-s); }

      /// Returns true if sign is ESign::c_Plus, ESign::c_Minus or ESign::c_Zero
      inline bool isValidSign(ESign s)
      { return std::abs(s) <= 1; }
    }

    /// Importing only the enum but not the constants from the nested namespace.
    typedef NSign::ESign ESign;

    /// Returns the sign of a floating point number.
    /** Essentially return the signbit of the float.
     *  This means 0.0 has sign ESign::c_Plus while -0.0 has sign ESign::c_Minus
     *  NAN is treat specially and returns an ESign::c_Invalid
     */
    inline ESign sign(double x)
    { return std::isnan(x) ? ESign::c_Invalid : (std::signbit(x) ? ESign::c_Minus : ESign::c_Plus); }

    /// Returns the sign of an integer number
    inline int sign(int x)
    { return (x > 0) - (x < 0); }

  } // namespace TrackFindingCDC

} // namespace Belle2
