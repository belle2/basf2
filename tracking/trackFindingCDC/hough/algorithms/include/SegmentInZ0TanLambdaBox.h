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
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

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
      using HoughBox = Z0TanLambdaBox;

      /**
       *  Checks if the wire hit is contained in a z0 tan lambda hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      Weight operator()(const std::pair<CDCSegment3D, CDCTrajectorySZ>& segmentWithTrajectorySZ,
                        const HoughBox* z0TanLambdaBox)
      {
        float lowerZ0 = z0TanLambdaBox->getLowerZ0();
        float upperZ0 = z0TanLambdaBox->getUpperZ0();

        float lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        float upperTanLambda = z0TanLambdaBox->getUpperTanLambda();

        const CDCTrajectorySZ& szTrajectory = segmentWithTrajectorySZ.second;
        const CDCSegment3D& segment = segmentWithTrajectorySZ.first;

        float trajectoryZ0 = szTrajectory.getZ0();
        float trajectoryTanLambda = szTrajectory.getTanLambda();

        if (std::isnan(trajectoryZ0) or std::isnan(trajectoryTanLambda)) {
          return NAN;
        }

        if (SameSignChecker::isIn(trajectoryZ0, trajectoryTanLambda, lowerZ0, upperZ0, lowerTanLambda, upperTanLambda)) {
          return 0.6 * segment.size();
        }

        return NAN;
      }
    };
  }
}
