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
#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *
     */
    class HitInZ0TanLambdaBoxUsingZ {
    public:

      /// Use a Z0TanLambdaBox
      typedef Z0TanLambdaBox HoughBox;

      /**
       *
       */
      inline Weight operator()(const CDCRecoHit3D& recoHit,
                               const HoughBox* z0TanLambdaBox)
      {
        const CDCWire& wire = recoHit.getWire();
        const WireLine& wireLine = wire.getWireLine();

        const float& lowerZ0 = z0TanLambdaBox->getLowerZ0();
        const float& upperZ0 = z0TanLambdaBox->getUpperZ0();
        const float centerZ0 = 0.5 * (lowerZ0 + upperZ0);

        const float& lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        const float& upperTanLambda = z0TanLambdaBox->getUpperTanLambda();
        const float centerTanLambda = 0.5 * (lowerTanLambda + upperTanLambda);

        const CDCTrajectorySZ boxTrajectory(centerTanLambda, centerZ0);

        const float& perpS = recoHit.getArcLength2D();
        const Vector2D& recoPosition = recoHit.getRecoPos2D();

        const float& hitZ = boxTrajectory.mapSToZ(perpS);

        const Vector3D& pos3D = wireLine.pos3DAtZ(hitZ);

        /*if(not wire.isInCellZBounds(lowerPos3D) and not wire.isInCellZBounds(upperPos3D)) {
          return NAN;
        }*/

        const float& distanceToRecoPosition = (pos3D.xy() - recoPosition).norm();

        /*const float& distLowerZ0LowerTanLambda = perpS * lowerTanLambda - reconstructedZ + lowerZ0;
        const float& distUpperZ0LowerTanLambda = perpS * lowerTanLambda - reconstructedZ + upperZ0;
        const float& distLowerZ0UpperTanLambda = perpS * upperTanLambda - reconstructedZ + lowerZ0;
        const float& distUpperZ0UpperTanLambda = perpS * upperTanLambda - reconstructedZ + upperZ0;

        const bool sameSign = SameSignChecker::sameSign(distLowerZ0LowerTanLambda, distUpperZ0LowerTanLambda,
                                                        distLowerZ0UpperTanLambda, distUpperZ0UpperTanLambda);
        if (not sameSign) {
          return 1.0;
        } else {
          return NAN;
        }*/

        //if(distanceToRecoPosition > 10) return NAN;

        return exp(-distanceToRecoPosition);
      }
    };
  }
}
