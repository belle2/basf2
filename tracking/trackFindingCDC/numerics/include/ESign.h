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

#include <cstdlib> //for abs

namespace Belle2 {

  namespace TrackFindingCDC {

    namespace SignT {

      enum Enum : signed short {
        /// Constant for plus sign.
        c_Plus = 1,

        /// Constant for minus sign.
        c_Minus = -1,

        /// Constant for undefined sign.
        c_Zero = 0,

        /// Constant for invalid.
        c_Invalid = -32768
      };
    }

    typedef SignT::Enum ESign;

    /// Return the opposite sign. Leaves ESign::c_Invalid the same.
    inline ESign oppositeSign(ESign s)
    { return static_cast<ESign>(-s); }

    /// Returns true if sign is ESign::c_Plus, ESign::c_Minus or ESign::c_Zero
    inline bool isValidSign(ESign s)
    { return std::abs(s) <= 1; }


  } // namespace TrackFindingCDC

} // namespace Belle2
