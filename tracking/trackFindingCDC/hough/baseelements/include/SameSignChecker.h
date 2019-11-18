/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Small helper for checking the same sign of two, four or eight floats.
     */
    class SameSignChecker {

    public:
      /// Check if two values have the same sign.
      static bool sameSign(double n1, double n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }

      /// Check if four values have the same sign.
      static bool sameSign(double n1, double n2, double n3, double n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }

      /// Check if eight values have the same sign.
      static bool sameSign(double n1, double n2, double n3, double n4,
                           double n5, double n6, double n7, double n8)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0 and
                 n5 > 0 and n6 > 0 and n7 > 0 and n8 > 0)
                or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0 and
                 n5 < 0 and n6 < 0 and n7 < 0 and n8 < 0));
      }

      /// Check if two values are in the rectangle spanned by the other four values
      static bool isIn(double x, double y, double x1, double x2, double y1, double y2)
      {
        return sameSign(x - x1, x2 - x, y - y1, y2 - y);
      }
    };

  }
}
