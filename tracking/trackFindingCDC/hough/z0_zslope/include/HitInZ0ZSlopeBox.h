/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/z0_zslope/Z0ZSlopeBox.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

// For better readability we use z slope instead of inverse z slope everywhere!
namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a z0 inverse z slope hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInZ0ZSlopeBox {
    public:
      /** Checks if the wire hit is contained in a z0 inverse z slope hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const CDCRecoHit3D* recoHit,
                               const Z0ZSlopeBox* z0ZSlopeBox)
      {
        const float& lowerZ0 = z0ZSlopeBox->getLowerZ0();
        const float& upperZ0 = z0ZSlopeBox->getUpperZ0();

        const float& inverseSlopeForMinimum = 1 / recoHit->calculateZSlopeWithZ0(lowerZ0);
        const float& inverseSlopeForMaximum = 1 / recoHit->calculateZSlopeWithZ0(upperZ0);

        const float& lowerInverseZSlope = z0ZSlopeBox->getLowerZSlope();
        const float& upperInverseZSlope = z0ZSlopeBox->getUpperZSlope();

        float dist[2][2];
        dist[0][0] = lowerInverseZSlope - inverseSlopeForMinimum;
        dist[0][1] = lowerInverseZSlope - inverseSlopeForMaximum;
        dist[1][0] = upperInverseZSlope - inverseSlopeForMinimum;
        dist[1][1] = upperInverseZSlope - inverseSlopeForMaximum;

        if (not SameSignChecker::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
          return 1.0;
        } else {
          return NAN;
        }
      }
    };
  }
}
