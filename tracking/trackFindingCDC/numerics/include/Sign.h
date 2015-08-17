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
      /// Implementing type of the sign
      typedef float SignType;

    public:
      /// Default constructor
      constexpr
      Sign() : m_sign(0)
      {;}

      /// Constructor from a float signed quantity
      constexpr
      Sign(const float& s) :
        m_sign(std::isnan(s) ? s : (s > 0) - (s < 0))
      {;}

      /// Constructor from a double signed quantity
      constexpr
      Sign(const double& s) :
        m_sign(std::isnan(s) ? s : (s > 0) - (s < 0))
      {;}

    public:
      /** Helper function that yield the second argument if the first is NAN. */
      static float fillnan(const float testValue, const float& defaultValue)
      { return std::isnan(testValue) ? defaultValue : testValue; }

      /** Calculate the common distance sign in a sweep operation over a geometric object.
       *  If the sign changes in a sweep over a geometric object or was zero in the begining
       *  the combined sign is zero. Forward the one sign if both are equal.
       *  Invalid signs are ignored and overwritten by the other sign.
       *  * ZERO and PLUS   -> ZERO
       *  * ZERO and MINUS  -> ZERO
       *  * ZERO and ZERO   -> ZERO
       *  * MINUS and PLUS  -> ZERO
       *  * MINUS and MINUS -> MINUS
       *  * PLUS and PLUS   -> PLUS
       *  * INVALID and ?   -> ?
       *  * and ? INVALID   -> ?
       */
      static Sign sweep(const Sign& sign0, const Sign& sign1)
      {
        if (sign0.m_sign == 0 or sign1.m_sign == 0) {
          return Sign(0.0);
        } else if (not sign0.isValid()) {
          return sign1;
        } else if (not sign1.isValid()) {
          return sign0;
        } else {
          return (sign0.m_sign + sign1.m_sign) / 2;
        }
      }
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

      /// Returns true if sign is equal to c_PlusSign, c_MinusSign or c_ZeroSign.
      inline bool isValid() const
      { return not std::isnan(m_sign); }

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
      SignType m_sign = 0;
    };

    /// Instance for a negative sign
    constexpr const Sign c_MinusSign(-1.0);

    /// Instance for zero sign
    constexpr const Sign c_ZeroSign(0.0);

    /// Instance for a positive sign
    constexpr const Sign c_PlusSign(1.0);

    /// Instance for a invalid sign
    constexpr const Sign c_InvalidSign(NAN);

  } // namespace TrackFindingCDC

} // namespace Belle2
