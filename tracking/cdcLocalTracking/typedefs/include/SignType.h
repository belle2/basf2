/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SIGNTYPE_H
#define SIGNTYPE_H

#include <cstdlib> //for abs
//#include <math.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Type transporting a sign
    typedef signed short SignType;
    //typedef signed char SignType; // This can be used for performance maximization but makes lousy outpu

    /// Constant for plus sign.
    const SignType PLUS = 1;

    /// Constant for minus sign.
    const SignType MINUS = -1;

    /// Constant for undefined sign.
    const SignType ZERO = 0;

    /// Constant for invalid.
    const SignType INVALID_SIGN = -32768;



    /// Return the opposite sign. Leaves INVALID_SIGN the same.
    inline SignType oppositeSign(const SignType& s)
    { return SignType(-s); }



    /// Returns true if sign is PLUS, MINUS or ZERO
    inline bool isValidSign(const SignType& s)
    { return std::abs(s) <= 1; }


  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // SIGNTYPE_H
