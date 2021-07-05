/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Predicate class to check for the containment of hits in a z0 tan lambda hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInZ0TanLambdaBox {
    public:

      /// Use a Z0TanLambdaBox
      using HoughBox = Z0TanLambdaBox;

      /**
       *  Checks if the wire hit is contained in a z0 tan lambda hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* z0TanLambdaBox)
      {
        float lowerZ0 = z0TanLambdaBox->getLowerZ0();
        float upperZ0 = z0TanLambdaBox->getUpperZ0();

        float lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        float upperTanLambda = z0TanLambdaBox->getUpperTanLambda();

        float perpS = recoHit.getArcLength2D();
        float reconstructedZ = recoHit.getRecoZ();

        float distLowerZ0LowerTanLambda = perpS * lowerTanLambda - reconstructedZ + lowerZ0;
        float distUpperZ0LowerTanLambda = perpS * lowerTanLambda - reconstructedZ + upperZ0;
        float distLowerZ0UpperTanLambda = perpS * upperTanLambda - reconstructedZ + lowerZ0;
        float distUpperZ0UpperTanLambda = perpS * upperTanLambda - reconstructedZ + upperZ0;

        const bool sameSign = SameSignChecker::sameSign(distLowerZ0LowerTanLambda, distUpperZ0LowerTanLambda,
                                                        distLowerZ0UpperTanLambda, distUpperZ0UpperTanLambda);
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
      static bool compareDistances(const HoughBox& z0TanLambdaBox, const CDCRecoHit3D& lhsRecoHit, const CDCRecoHit3D& rhsRecoHit)
      {
        const double z0Mean = (z0TanLambdaBox.getLowerZ0() + z0TanLambdaBox.getUpperZ0()) / 2.0;
        const double tanLambdaMean = (z0TanLambdaBox.getLowerTanLambda() + z0TanLambdaBox.getUpperTanLambda()) / 2.0;

        const double lhsZ = lhsRecoHit.getRecoZ();
        const double rhsZ = rhsRecoHit.getRecoZ();

        const double lhsS = lhsRecoHit.getArcLength2D();
        const double rhsS = rhsRecoHit.getArcLength2D();

        const double lhsZDistance = lhsS * tanLambdaMean + z0Mean - lhsZ;
        const double rhsZDistance = rhsS * tanLambdaMean + z0Mean - rhsZ;

        return lhsZDistance < rhsZDistance;
      }

      /// ROOT-compatible formula for z(s) = z0 + tanlambda * s
      static const char* debugLine() { return "[0] + [1] * x";}
    };
  }
}
