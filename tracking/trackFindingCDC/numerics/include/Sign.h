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

#include <ostream>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type to represent the sign of a quantity
    class Sign {

    public:
      /// Instance for a negative sign
      static const Sign c_Minus;

      /// Instance for a unsigned
      static const Sign c_Zero;

      /// Instance for a positive sign
      static const Sign c_Plus;

      /// Instance for a invalid sign
      static const Sign c_Invalid;

    public:
      /// Implementing type of the sign
      typedef float SignType;

    public:
      /// Constructor from a float signed quantity
      Sign(const float& s) :
        m_sign(std::isnan(s) ? NAN : (s > 0) - (s < 0))
      {;}

      /// Constructor from a double signed quantity
      Sign(const double& s) :
        m_sign(std::isnan(s) ? NAN : (s > 0) - (s < 0))
      {;}

    public:
      /** Helper function that yield the second argument if the first is NAN. */
      static float fillnan(const float testValue, const float& defaultValue)
      { return std::isnan(testValue) ? defaultValue : testValue; }

      /** Calculate the common sign between to signed expressions.
       *  The common sign of
       *  * ZERO and PLUS   -> ZERO
       *  * ZERO and MINUS  -> ZERO
       *  * ZERO and ZERO   -> ZERO
       *  * MINUS and PLUS  -> ZERO
       *  * MINUS and MINUS -> MINUS
       *  * PLUS and PLUS   -> PLUS
       *  * INVALID and ?   -> ?
       */
      static Sign dominant(const Sign& sign0, const Sign& sign1)
      { return Sign(std::round((fillnan(sign0.m_sign, sign1.m_sign) + fillnan(sign1.m_sign, sign0.m_sign)) / 4.0));}
      /// Investigates if two signs are identical.
      static bool same(const Sign& sign0, const Sign& sign1)
      { return sign0 == sign1; }

    public:
      /// Change the sign inplace to its opposite.
      void reverse()
      { m_sign = SignType(-m_sign); }

      /// Return a copy with the reversed sign
      Sign reversed()
      { return Sign(SignType(-m_sign)); }

      /// Returns true if sign is c_Plus, c_Minus or c_Zero
      inline bool isValid()
      { return std::abs(m_sign) <= 1; }

      /// Equality comparision
      bool operator==(const Sign& other) const
      { return m_sign == other.m_sign; }

      /// Multiplication with another float
      float operator*(const float& rhs) const
      { return m_sign * rhs; }

      /// Multiplication with another double
      double operator*(const double& rhs) const
      { return m_sign * rhs; }

      /// Printing operator
      friend std::ostream& operator<<(std::ostream& output, const Sign& sign)
      { return output << sign.m_sign; }

    private:
      /// Memory for the sign
      SignType m_sign;
    };

    const Sign Sign::c_Minus(-1.0);
    const Sign Sign::c_Zero(0.0);
    const Sign Sign::c_Plus(1.0);
    const Sign Sign::c_Invalid(NAN);

  } // namespace TrackFindingCDC

} // namespace Belle2
