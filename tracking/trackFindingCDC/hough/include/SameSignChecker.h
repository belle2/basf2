/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/typedefs/SignType.h>
#include <array>
#include <algorithm>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Small helper for checking the same sign of for or two floats.
     */
    class SameSignChecker {

    public:
      /** Check if four values have a common sign.
       *  Ignores nan values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      static inline SignType commonSign(const float& n1, const float& n2,
                                        const float& n3, const float& n4)
      {
        return (std::isnan(n1) and std::isnan(n2) and std::isnan(n3) and std::isnan(n4)) ?
               INVALID_SIGN :
               (not(n1 <= 0) and not(n2 <= 0) and not(n3 <= 0) and not(n4 <= 0)) -
               (not(n1 >= 0) and not(n2 >= 0) and not(n3 >= 0) and not(n4 >= 0));
      }

      /** Check if two values have a common sign.
       *  Ignores nan values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      static inline SignType commonSign(const float& n1,
                                        const float& n2)
      {
        return std::isnan(n1) and std::isnan(n2) ?
               INVALID_SIGN :
               ((not(n1 <= 0) and not(n2 <= 0)) - (not(n1 >= 0) and not(n2 >= 0)));
      }

      /** Check if four values have a common sign.
       *  Ignores nan values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      template<std::size_t n>
      static inline SignType commonSign(const std::array<float, n>& a)
      {
        return (std::all_of(a.begin(), a.end(), std::isnan) ?
                INVALID_SIGN :
        std::all_of(a.begin(), a.end(), [](const float & a) { return not(a <= 0); }) -
        std::all_of(a.begin(), a.end(), [](const float & a) { return not(a >= 0); }));
      }

      /** Check if four values have a common sign.
       *  Ignores nan values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      static inline SignType commonSign(const std::array<float, 4>& a)
      {
        return (std::isnan(a[0]) and std::isnan(a[1]) and std::isnan(a[2]) and std::isnan(a[3])) ?
               INVALID_SIGN :
               (not(a[0] <= 0) and not(a[1] <= 0) and not(a[2] <= 0) and not(a[3] <= 0)) -
               (not(a[0] >= 0) and not(a[1] >= 0) and not(a[2] >= 0) and not(a[3] >= 0));
      }

      /** Check if two values have a common sign.
       *  Ignores nan values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      static inline SignType commonSign(const std::array<float, 2>& a)
      {
        return std::isnan(a[0]) and std::isnan(a[1]) ?
               INVALID_SIGN :
               ((not(a[0] <= 0) and not(a[1] <= 0)) - (not(a[0] >= 0) and not(a[1] >= 0)));
      }

      /** Check if two values have a common sign.
       *  Ignores INVALID_SIGN values.
       *  Returns PLUS if all signs are positive.
       *  Returns MINUS if all signs are negative.
       *  Returns ZERO for mixed signs.
       */
      static inline SignType commonSign(const SignType& n1, const SignType& n2)
      {
        return (n1 == INVALID_SIGN) and (n2 == INVALID_SIGN) ?
               INVALID_SIGN :
               ((not(n1 <= 0) and not(n2 <= 0)) - (not(n1 >= 0) and not(n2 >= 0)));
      }


      /// Check if four values have the same sign.
      static inline bool sameSign(double n1, double n2, double n3, double n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }

      /// Check if two values have the same sign.
      static inline bool sameSign(double n1, double n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
