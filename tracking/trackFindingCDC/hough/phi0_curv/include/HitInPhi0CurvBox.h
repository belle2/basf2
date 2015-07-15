/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvBox.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <tracking/trackFindingCDC/numerics/numerics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    template<bool refined = false>
    class HitInPhi0CurvBox {

    public:
      /// Check if four values have the same sign.
      static inline bool sameSign(float n1, float n2, float n3, float n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }

      /// Check if two values have the same sign.
      static inline bool sameSign(float n1, float n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }
    public:
      /** Checks if the track hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const TrackHit* hit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        // TODO
        // Replace TrackHit with CDCWireHit or even better CDCRLWireHit !
        // Also get rid of the conformal transformation !
        const CDCWireHit* wireHit = hit->getUnderlayingCDCWireHit();
        return operator()(wireHit, phi0CurvBox);
      }

      /** Checks if the track hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(const CDCWireHit* wireHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType driftLength = wireHit->getRefDriftLength();
        const Vector2D& pos2D =  wireHit->getRefPos2D();
        const FloatType r = wireHit->getRefCylindricalR();

        bool rightIn = this->isObservationIn(r, pos2D, driftLength, phi0CurvBox);
        bool leftIn = this->isObservationIn(r, pos2D, -driftLength, phi0CurvBox);
        return (rightIn or leftIn) ? 1 : NAN;
      }

      /** Checks if the track hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(const CDCRLWireHit* rlWireHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType signedDriftLength = rlWireHit-> getSignedRefDriftLength();
        const Vector2D& pos2D =  rlWireHit->getRefPos2D();
        const FloatType r = rlWireHit->getRefCylindricalR();

        bool in = this->isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
        return in ? 1 : NAN;
      }


      inline bool isObservationIn(const Vector2D& pos2D,
                                  const FloatType signedDriftLength,
                                  const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType r = pos2D.norm();
        return isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
      }

      inline bool isObservationIn(const FloatType& r,
                                  const Vector2D& pos2D,
                                  const FloatType signedDriftLength,
                                  const Phi0CurvBox* phi0CurvBox)
      {

        const FloatType rSquared = r * r;

        FloatType orthoToPhi0 [2] = {
          pos2D.cross(phi0CurvBox->getLowerBound<0>().getAngleVec()),
          pos2D.cross(phi0CurvBox->getUpperBound<0>().getAngleVec())
        };

        FloatType rSquareTimesHalfCurve[2] = {
          rSquared* (static_cast<float>(phi0CurvBox->getLowerBound<1>()) / 2),
          rSquared* (static_cast<float>(phi0CurvBox->getUpperBound<1>()) / 2)
        };

        float dist[2][2];
        dist[0][0] = rSquareTimesHalfCurve[0] + orthoToPhi0[0] - signedDriftLength;
        dist[0][1] = rSquareTimesHalfCurve[1] + orthoToPhi0[0] - signedDriftLength;
        dist[1][0] = rSquareTimesHalfCurve[0] + orthoToPhi0[1] - signedDriftLength;
        dist[1][1] = rSquareTimesHalfCurve[1] + orthoToPhi0[1] - signedDriftLength;

        // Sinogram intersects at least on of the boundaries.
        if (not sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) return true;
        if (not refined) return false;

        // Extra check if the maximum / minimum of the sinogram is in the Box.
        // Check if the slope sign changes
        FloatType parallelToPhi0[2] = {
          pos2D.dot(phi0CurvBox->getLowerBound<0>().getAngleVec()),
          pos2D.dot(phi0CurvBox->getUpperBound<0>().getAngleVec())
        };
        if (sameSign(parallelToPhi0[0], parallelToPhi0[1])) return false;

        // This two values could be precomputed if necessary
        const float curveMax = 2 / (r - signedDriftLength);
        const float curveMin = 2 / (-r - signedDriftLength);
        return (phi0CurvBox->isIn<1>(curveMax) or phi0CurvBox->isIn<1>(curveMin));
        // TODO also extended version does not cover all cases...

      }

    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
