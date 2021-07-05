/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <algorithm>
#include <cstdlib> // for abs
#include <cmath> // for isnan
#include <climits> // for SHRT_MIN

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the constants of the ESign enumeration.
    namespace ESignUtil {

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
      inline ESign opposite(ESign s)
      { return static_cast<ESign>(-s); }

      /// Returns true if sign is ESign::c_Plus, ESign::c_Minus or ESign::c_Zero
      inline bool isValid(ESign s)
      { return std::abs(s) <= 1; }

      /**
       *  Check if two values have a common sign.
       *  Ignores ESign::c_Invalid values.
       *  Returns ESign::c_Plus if all signs are positive.
       *  Returns ESign::c_Minus if all signs are negative.
       *  Returns ESign::c_Zero for mixed signs.
       *  Returns ESign::c_Invalid if all input are c_Invalid
       */
      static inline ESign common(ESign n1, ESign n2)
      {
        return ((not isValid(n1) and not isValid(n2)) ?
                ESign::c_Invalid :
                static_cast<ESign>((not(n1 <= 0) and not(n2 <= 0)) - (not(n1 >= 0) and not(n2 >= 0))));
      }

      /**
       *  Check if two values have a common sign.
       *  Ignores nan values.
       *  Returns ESign::c_Plus if all signs are positive.
       *  Returns ESign::c_Minus if all signs are negative.
       *  Returns ESign::c_Zero for mixed signs.
       *  Returns ESign::c_Invalid if all input are NaN.
       */
      static inline ESign common(float n1, float n2)
      {
        return ((std::isnan(n1) and std::isnan(n2)) ?
                ESign::c_Invalid :
                static_cast<ESign>((not(n1 <= 0) and not(n2 <= 0)) - (not(n1 >= 0) and not(n2 >= 0))));
      }

      /**
       *  Check if four values have a common sign.
       *  Ignores nan values.
       *  Returns ESign::c_Plus if all signs are positive.
       *  Returns ESign::c_Minus if all signs are negative.
       *  Returns ESign::c_Zero for mixed signs.
       *  Returns ESign::c_Invalid if all input are NaN.
       */
      static inline ESign common(float n1, float n2, float n3, float n4)
      {
        return ((std::isnan(n1) and std::isnan(n2) and std::isnan(n3) and std::isnan(n4)) ?
                ESign::c_Invalid :
                static_cast<ESign>((not(n1 <= 0) and not(n2 <= 0) and not(n3 <= 0) and not(n4 <= 0)) -
                                   (not(n1 >= 0) and not(n2 >= 0) and not(n3 >= 0) and not(n4 >= 0))));
      }

      /**
       *  Check if four values have a common sign.
       *  Ignores nan values.
       *  Returns ESign::c_Plus if all signs are positive.
       *  Returns ESign::c_Minus if all signs are negative.
       *  Returns ESign::c_Zero for mixed signs.
       *  Returns ESign::c_Invalid if all input are NaN.
       */
      template<class FloatRange>
      static inline ESign common(const FloatRange& as)
      {
        bool allNaN = std::all_of(as.begin(), as.end(), [](float a) { return std::isnan(a); });
        if (allNaN) {
          return ESign::c_Invalid;
        } else {
          return static_cast<ESign>(std::all_of(as.begin(), as.end(), [](float a) { return not(a <= 0); }) -
          std::all_of(as.begin(), as.end(), [](float a) { return not(a >= 0); }));
        }
      }

    };

    /// Importing only the enum but not the constants from the nested namespace.
    using ESign = ESignUtil::ESign;

    /**
     *  Returns the sign of a floating point number.
     *  Essentially return the signbit of the float.
     *  This means 0.0 has sign ESign::c_Plus while -0.0 has sign ESign::c_Minus
     *  NAN is treat specially and returns an ESign::c_Invalid
     */
    inline ESign sign(double x)
    {
      return std::isnan(x) ? ESign::c_Invalid : (std::signbit(x) ? ESign::c_Minus : ESign::c_Plus);
    }

    /// Returns the sign of an integer number
    inline ESign sign(int x)
    {
      return static_cast<ESign>((x > 0) - (x < 0));
    }

    /// Returns the sign of an integer number
    template<class Enum, Enum invalid = Enum::c_Invalid>
    inline ESign sign(Enum x)
    {
      if (x == Enum::c_Invalid) return ESign::c_Invalid;
      return sign(static_cast<int>(x));
    }
  }

}
