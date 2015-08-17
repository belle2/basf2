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
                                      const Vector2D& globalPos2D,
                                      const double signedDriftLength,
                                      const Vector2D& /*movePerZ*/ = Vector2D(0.0, 0.0)) const
      {
        const Vector2D localPos2D = globalPos2D - m_localOrigin;

        const Vector2D& lowerPhi0Vec(phi0CurvBox->getLowerBound<DiscretePhi0>());
        const Vector2D& upperPhi0Vec(phi0CurvBox->getUpperBound<DiscretePhi0>());
        const float& lowerCurv(phi0CurvBox->getLowerBound<DiscreteCurv>());
        const float& upperCurv(phi0CurvBox->getUpperBound<DiscreteCurv>());

        const FloatType parallelToPhi0[2] = { localPos2D.dot(lowerPhi0Vec), localPos2D.dot(upperPhi0Vec) };
        const bool isNonCurler = upperCurv <= m_curlCurv and lowerCurv >= -m_curlCurv;
        if (isNonCurler) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (parallelToPhi0[0] < 0 and parallelToPhi0[1] < 0) return INVALID_SIGN;  //c_InvalidSign;
        }

        //FloatType localRSquare = square(localR);
        FloatType localRSquare = localPos2D.normSquared();
        const FloatType rReducedSquared = localRSquare - square(signedDriftLength);

        const FloatType rSquareTimesHalfCurv[2] = {
          rReducedSquared* (lowerCurv / 2),
          rReducedSquared* (upperCurv / 2)
        };

        const FloatType orthoToPhi0[2] = { localPos2D.cross(lowerPhi0Vec), localPos2D.cross(upperPhi0Vec) };

        // Calculate (approximate) distances of the observed position to
        // the trajectories represented by the corners of the box
        double dist[2][2];
        dist[0][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[0] - signedDriftLength;
        dist[0][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[0] - signedDriftLength;
        dist[1][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[1] - signedDriftLength;
        dist[1][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[1] - signedDriftLength;

        // Ask if sinogram is completely outside the box.
        const SignType passingSide = SameSignChecker::commonSign(dist[0][0], dist[0][1],
                                                                 dist[1][0], dist[1][1]);

        return passingSide;

        // Currently disable and not ported to retain the sign.
        // if (passingSide == ZERO) return ZERO;
        // if (not m_refined) return passingSide;
        // if (not m_refined) return false;
        // // Continue to check if the extrema of the sinogram are in the box,
        // // while only touching only one boundary of the box
        // // (but not crossing two what the check above actually means).
        // // Currently only checking the positive curvature branch correctly
        // FloatType extremR = r - signedDriftLength;
        // bool extremRInCurvRange = not SameSignChecker::sameSign(extremR * lowerCurv - 1, extremR * upperCurv - 1);
        // if (not extremRInCurvRange) return false;

        // Vector2D extremPhi0Vec = pos2D.orthogonal(CCW); // Not normalised but does not matter.
        // bool extremPhi0VecInPhi0Range = extremPhi0Vec.isBetween(lowerPhi0Vec, upperPhi0Vec);
        // return extremRInCurvRange and extremPhi0VecInPhi0Range;
      }

    public:
      /// Getter for the local origin relative to which the parameters of the hough space are understood
      const Vector2D& getLocalOrigin() const
      { return m_localOrigin; }

      /// Setter for the local origin relative to which the parameters of the hough space are understood
      void setLocalOrigin(const Vector2D& localOrigin)
      { m_localOrigin = localOrigin; }

    private:
      /// The curvature above which the trajectory is considered a curler.
      float m_curlCurv = NAN;

      /// A displaced origin to search for off origin circles
      Vector2D m_localOrigin = Vector2D(0.0, 0.0);

    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
