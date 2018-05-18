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
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/boxes/Z1Z2Box.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Predicate class to check for the containment of hits in a z1 z2 hough space part. FIXME rename parameters
     * z(s) = 0 + (p + 4q)*s - q/25 * s^2
     * p - z coordinate of track in outer layers of CDC (in units of 100cm //CDC radius is 113)
     * q - divergence of track from straight line in the middle of CDC (in units of 100cm)
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInZ1Z2Box {
    public:

      /// Use a Z1Z2Box
      using HoughBox = Z1Z2Box;

      /**
       *  Checks if the wire hit is contained in a z1 z2 hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* z1z2Box)
      {
        float lowerZ1 = z1z2Box->getLowerZ1();
        float upperZ1 = z1z2Box->getUpperZ1();

        float lowerZ2 = z1z2Box->getLowerZ2();
        float upperZ2 = z1z2Box->getUpperZ2();

        float perpS = recoHit.getArcLength2D();
        float reconstructedZ = recoHit.getRecoZ();

        float distLowerZ1LowerZ2 = (lowerZ1 + 4 * lowerZ2) * perpS - lowerZ2 / 25 * perpS * perpS - reconstructedZ;
        float distUpperZ1LowerZ2 = (upperZ1 + 4 * lowerZ2) * perpS - lowerZ2 / 25 * perpS * perpS - reconstructedZ;
        float distLowerZ1UpperZ2 = (lowerZ1 + 4 * upperZ2) * perpS - upperZ2 / 25 * perpS * perpS - reconstructedZ;
        float distUpperZ1UpperZ2 = (upperZ1 + 4 * upperZ2) * perpS - upperZ2 / 25 * perpS * perpS - reconstructedZ;

        const bool sameSign = SameSignChecker::sameSign(distLowerZ1LowerZ2, distUpperZ1LowerZ2,
                                                        distLowerZ1UpperZ2, distUpperZ1UpperZ2);
        if (not sameSign) {
          return 1.0;
        } else {
          return NAN;
        }
      }

      static const char* debugLine() { return "([0] + 4*[1])*x - [1] / 25 * x * x";}
    };
  }
}
