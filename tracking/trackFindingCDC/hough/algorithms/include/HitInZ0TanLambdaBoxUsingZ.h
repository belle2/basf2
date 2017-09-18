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

#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/WireLine.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
      * Predicate class to check for the containment of hits in a z0 tan lambda hough space part using a different algorithm than normal.
      * Instead of calculating of the hit belongs to the box, it constructs a trajectory out if the box parameters (using its taln lambda and z0)
      * and checks the distance from the hit to the trajectory. It returns e^{-distance}.
      * Note this part this code defines the performance of
      * the search in the hough plain quite significantly and there is probably room for improvement.
      */
    class HitInZ0TanLambdaBoxUsingZ {
    public:

      /// Use a Z0TanLambdaBox
      using HoughBox = Z0TanLambdaBox;

      /**
       * Return exp{-distance} with distance = distance between the trajectory constructed from the box parameters
       * and the hit in the xy-plane.
       * Note that this is not a binary decision and must be used with some sort of cutoff (because otherwise all hits belong in all boxes).
       */
      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* z0TanLambdaBox)
      {
        const CDCWire& wire = recoHit.getWire();
        const WireLine& wireLine = wire.getWireLine();

        float lowerZ0 = z0TanLambdaBox->getLowerZ0();
        float upperZ0 = z0TanLambdaBox->getUpperZ0();
        const float centerZ0 = 0.5 * (lowerZ0 + upperZ0);

        float lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        float upperTanLambda = z0TanLambdaBox->getUpperTanLambda();
        const float centerTanLambda = 0.5 * (lowerTanLambda + upperTanLambda);

        float perpS = recoHit.getArcLength2D();
        const Vector2D& recoPosition = recoHit.getRecoPos2D();

        float hitZ = centerTanLambda * perpS + centerZ0;

        Vector2D pos2D = wireLine.nominalPos2DAtZ(hitZ);

        float distanceToRecoPosition = (pos2D - recoPosition).norm();

        return exp(-distanceToRecoPosition);
      }
    };
  }
}
