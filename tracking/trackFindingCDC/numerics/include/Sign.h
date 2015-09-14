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

    private:
      /// Implementing type of the sign
      typedef signed short ESign;
      //typedef signed char ESign; // This can be used to tighten memory

      /// Constant for an invalid sign
      static const ESign c_NaNSign = -32768;


    public:
      /// Default constructor
      constexpr
      Sign() : m_sign(0)
      {}

    private:
      /// Constructor from the internal representation
      constexpr
      explicit Sign(const ESign& s) :
        m_sign(s)
      {}

    public:
      /// Constructor from a float signed quantity
      constexpr
      explicit Sign(const float& s) :
        m_sign(std::isnan(s) ? c_NaNSign : (s > 0) - (s < 0))
      {}

      /// Constructor from a double signed quantity
      constexpr
      explicit Sign(const double s) :
        m_sign(std::isnan(s) ? c_NaNSign : (s > 0) - (s < 0))
      {}

    public:
      /** Calculate the common distance sign in a sweep operation over a geometric object.
       *  If the sign changes in a sweep over a geometric object or was zero in the begining
       *  the combined sign is zero. Forward the one sign if both are equal.
       *  Invalid signs are ignored and overwritten by the other sign.
       *  * ESign::c_Zero and ESign::c_Plus   -> ESign::c_Zero
       *  * ESign::c_Zero and ESign::c_Minus  -> ESign::c_Zero
       *  * ESign::c_Zero and ESign::c_Zero   -> ESign::c_Zero
       *  * ESign::c_Minus and ESign::c_Plus  -> ESign::c_Zero
       *  * ESign::c_Minus and ESign::c_Minus -> ESign::c_Minus
       *  * ESign::c_Plus and ESign::c_Plus   -> ESign::c_Plus
       *  * INVALID and ?   -> ?
       *  * and ? INVALID   -> ?
       */
      static Sign sweep(const Sign& sign0, const Sign& sign1)
      {
        if (sign0.m_sign == 0 or sign1.m_sign == 0) {
          return Sign(0.0);
        } else if (sign0.isnan()) {
          return sign1;
        } else if (sign1.isnan()) {
          return sign0;
        } else {
          return Sign(static_cast<ESign>((sign0.m_sign + sign1.m_sign) / 2));
        }
      }

    public:
      /// Change the sign inplace to its opposite.
      void reverse()
      { m_sign = static_cast<ESign>(-m_sign); }

      /// Return a copy with the reversed sign
      Sign reversed() const
      { return Sign(static_cast<ESign>(-m_sign)); }

      /// Returns true if sign is equal to c_PlusSign, c_MinusSign or c_ZeroSign.
      inline bool isfinite() const
      { return std::abs(m_sign) <= 1; }

      /// Returns true if the sign is the nan equivalent represenation.
      inline bool isnan() const
      { return m_sign == c_NaNSign; }

      /// Equality comparision
      bool operator==(const Sign& other) const
      { return (isfinite() and other.isfinite()) ? m_sign == other.m_sign : false; }

      /// Multiplication with another float
      float operator*(const float& rhs) const
      { return isnan() ? NAN : m_sign * rhs; }

      /// Multiplication with another double
      double operator*(const double rhs) const
      { return isnan() ? NAN : m_sign * rhs; }

      /// Printing operator
      friend std::ostream& operator<<(std::ostream& output, const Sign& sign)
      { return output << sign.m_sign; }

    private:
      /// Memory for the sign implementation
      ESign m_sign = 0;
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
