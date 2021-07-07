/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/boxes/PQBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Predicate class to check for the containment of hits in a p q hough space part. FIXME physical names of parameters?
     * z(s) = 0 + (p + 4q)*s - q/25 * s^2
     * p - z coordinate of track in outer layers of CDC (in units of 100cm //CDC radius is 113)
     * q - divergence of track from straight line in the middle of CDC (in units of 100cm)
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInQuadraticBox {
    public:

      /// Use a PQBox
      using HoughBox = PQBox;

      /**
       *  Checks if the wire hit is contained in a p q hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* pqBox)
      {
        float lowerP = pqBox->getLowerP();
        float upperP = pqBox->getUpperP();

        float lowerQ = pqBox->getLowerQ();
        float upperQ = pqBox->getUpperQ();

        float perpS = recoHit.getArcLength2D();
        float reconstructedZ = recoHit.getRecoZ();

        float distLowerPLowerQ = (lowerP + 4 * lowerQ) * perpS - lowerQ / 25 * perpS * perpS - reconstructedZ;
        float distUpperPLowerQ = (upperP + 4 * lowerQ) * perpS - lowerQ / 25 * perpS * perpS - reconstructedZ;
        float distLowerPUpperQ = (lowerP + 4 * upperQ) * perpS - upperQ / 25 * perpS * perpS - reconstructedZ;
        float distUpperPUpperQ = (upperP + 4 * upperQ) * perpS - upperQ / 25 * perpS * perpS - reconstructedZ;

        const bool sameSign = SameSignChecker::sameSign(distLowerPLowerQ, distUpperPLowerQ,
                                                        distLowerPUpperQ, distUpperPUpperQ);
        if (not sameSign) {
          return 1.0;
        } else {
          return NAN;
        }
      }

      /**
       * Compares distances from two hits to the track represented by the given box.
       * The comparison is done based on reconstructed Z coordinates of hits and track Z position.
       */
      static bool compareDistances(const HoughBox& pqBox, const CDCRecoHit3D& lhsRecoHit, const CDCRecoHit3D& rhsRecoHit)
      {
        const double pMean = (pqBox.getLowerP() + pqBox.getUpperP()) / 2.0;
        const double qMean = (pqBox.getLowerQ() + pqBox.getUpperQ()) / 2.0;

        const double lhsZ = lhsRecoHit.getRecoZ();
        const double rhsZ = rhsRecoHit.getRecoZ();

        const double lhsS = lhsRecoHit.getArcLength2D();
        const double rhsS = rhsRecoHit.getArcLength2D();

        const double lhsZDistance = (pMean + 4 * qMean) * lhsS - qMean / 25 * lhsS * lhsS - lhsZ;
        const double rhsZDistance = (pMean + 4 * qMean) * rhsS - qMean / 25 * rhsS * rhsS - rhsZ;

        return lhsZDistance < rhsZDistance;
      }

      /// ROOT-compatible formula for z(s) = (p + 4q)*s - q/25 * s^2
      static const char* debugLine() { return "([0] + 4*[1])*x - [1] / 25 * x * x";}
    };
  }
}
