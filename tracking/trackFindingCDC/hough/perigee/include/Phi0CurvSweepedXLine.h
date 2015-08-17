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

#include <tracking/trackFindingCDC/hough/perigee/DiscreteCurv.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscretePhi0.h>
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>
#include <tracking/trackFindingCDC/numerics/Sign.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class Phi0CurvSweepedXLine {

    public:
      /// The hough box to which this object correspondes.
      typedef Box<DiscretePhi0 , DiscreteCurv> HoughBox;

    public:
      /// Constructor taking the curling curvature.
      Phi0CurvSweepedXLine(const float& curlCurv) : m_curlCurv(curlCurv) {;}

      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      inline SignType getDistanceSign(const HoughBox* phi0CurvBox,
                                      const Vector2D& pos2D,
                                      const double signedDriftLength,
                                      const Vector2D& /*movePerZ*/ = Vector2D(0.0, 0.0)) const
      {
        const Vector2D& lowerPhi0Vec(phi0CurvBox->getLowerBound<DiscretePhi0>());
        const Vector2D& upperPhi0Vec(phi0CurvBox->getUpperBound<DiscretePhi0>());
        const float& lowerCurv(phi0CurvBox->getLowerBound<DiscreteCurv>());
        const float& upperCurv(phi0CurvBox->getUpperBound<DiscreteCurv>());

        const FloatType parallelToPhi0[2] = { pos2D.dot(lowerPhi0Vec), pos2D.dot(upperPhi0Vec) };
        const bool isNonCurler = upperCurv <= m_curlCurv and lowerCurv >= -m_curlCurv;
        if (isNonCurler) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (parallelToPhi0[0] < 0 and parallelToPhi0[1] < 0) return INVALID_SIGN;  //c_InvalidSign;
        }

        FloatType rSquare = pos2D.normSquared();
        const FloatType rReducedSquared = rSquare - square(signedDriftLength);

        const FloatType rSquareTimesHalfCurv[2] = {
          rReducedSquared* (lowerCurv / 2),
          rReducedSquared* (upperCurv / 2)
        };

        const FloatType orthoToPhi0[2] = { pos2D.cross(lowerPhi0Vec), pos2D.cross(upperPhi0Vec) };

        // Calculate (approximate) distances of the observed position to
        // the trajectories represented by the corners of the box
        float dist[2][2];
        dist[0][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[0] - signedDriftLength;
        dist[0][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[0] - signedDriftLength;
        dist[1][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[1] - signedDriftLength;
        dist[1][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[1] - signedDriftLength;

        // Ask if sinogram is completely outside the box.

        return SameSignChecker::commonSign(SameSignChecker::commonSign(dist[0][0], dist[0][1]),
                                           SameSignChecker::commonSign(dist[1][0], dist[1][1]));
      }
    private:
      /// The curvature above which the trajectory is considered a curler.
      float m_curlCurv = NAN;
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
