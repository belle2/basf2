/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class CurvSweepedXLine {

    public:
      /// The hough box to which this object correspondes.
      typedef Box<DiscreteCurv> HoughBox;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      inline SignType getDistanceSign(const HoughBox& houghBox,
                                      const float& x,
                                      const float& y,
                                      const float& signedDriftLength,
                                      const float& /*dxdz*/ = 0,
                                      const float& /*dydz*/ = 0) const
      {
        const float& lowerCurv(houghBox.getLowerBound<DiscreteCurv>());
        const float& upperCurv(houghBox.getUpperBound<DiscreteCurv>());

        const float driftLengthSquared = square(signedDriftLength);
        const float correctedY = -(y + signedDriftLength);

        const float rReducedSquaredHalf = x * x + y * y - driftLengthSquared;

        // Calculate (approximate) distances of the observed position to
        // the trajectories represented by the corners of the box
        const float lowerDist = lowerCurv / 2 * rReducedSquaredHalf + correctedY;
        const float upperDist = upperCurv / 2 * rReducedSquaredHalf + correctedY;

        // Ask if sinogram is completely outside the box.
        return SameSignChecker::commonSign(lowerDist, upperDist);
      }

    private:
      /// The curvature above which the trajectory is considered a curler.
      float m_curlCurv = NAN;
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
