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

#include <tracking/trackFindingCDC/numerics/ESign.h>
#include <array>
#include <algorithm>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Small helper for checking the same sign of for or two floats.
     */
    class SameSignChecker {

    public:
      /// Check if two values have the same sign.
      static inline bool sameSign(double n1, double n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }

      /// Check if four values have the same sign.
      static inline bool sameSign(double n1, double n2, double n3, double n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
