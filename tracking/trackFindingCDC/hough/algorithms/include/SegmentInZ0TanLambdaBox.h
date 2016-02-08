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
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Predicate class to check for the containment of hits in a z0 tan lambda hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class SegmentInZ0TanLambdaBox {
    public:

      /// Use a Z0TanLambdaBox
      typedef Z0TanLambdaBox HoughBox;

      /**
       *  Checks if the wire hit is contained in a z0 tan lambda hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const CDCRecoSegment3D& recoSegment,
                               const HoughBox* z0TanLambdaBox)
      {
        const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

        const float& lowerZ0 = z0TanLambdaBox->getLowerZ0();
        const float& upperZ0 = z0TanLambdaBox->getUpperZ0();

        const float& underLowerZ0 = z0TanLambdaBox->getUnderLowerZ0();
        const float& overUpperZ0 = z0TanLambdaBox->getOverUpperZ0();

        const float& lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        const float& upperTanLambda = z0TanLambdaBox->getUpperTanLambda();

        const float& underLowerTanLambda = z0TanLambdaBox->getUnderLowerTanLambda();
        const float& overUpperTanLambda = z0TanLambdaBox->getOverUpperTanLambda();

        const CDCTrajectorySZ& szTrajectory = szFitter.fitUsingStrangeFunction(recoSegment);
        const float& trajectoryZ0 = szTrajectory.getStartZ();
        const float& trajectoryTanLambda = szTrajectory.getTanLambda();

        if (SameSignChecker::isIn(trajectoryZ0, trajectoryTanLambda, lowerZ0, upperZ0, lowerTanLambda, upperTanLambda)) {
          return 1.0;
        }

        if (SameSignChecker::isIn(trajectoryZ0, trajectoryTanLambda, underLowerZ0, overUpperZ0, underLowerTanLambda, overUpperTanLambda)) {
          return 0.5;
        }


        return NAN;
      }
    };
  }
}
