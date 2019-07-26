/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>
#include <framework/logging/Logger.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** An algorithm to check if a hit is contained in a hyperbolic cosine hough box
    *
    * The parameterization is
    * z(R) = mu * (sqrt(1 - p * p) * cosh(R / mu / q + arcsinh(p / sqrt(1 - p * p))) - 1)
    *
    * p is p_z / E - longitudinal fraction of energy
    * q is p_t / E - transverse fraction of energy
    * mu is E / gcB - scale
    * z and R are in units of CDC size
    *
    * The catenary passes through the origin
    */
    class HitInHyperBox {

    public:
      using HoughBox = Box<DiscreteP, DiscreteQ, DiscreteMu>;

      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* hyperBox)
      {
        const float lowerP = *(hyperBox->getLowerBound<DiscreteP>()); //DiscreteValue is based on std::vector<T>::const_iterator
        const float upperP = *(hyperBox->getUpperBound<DiscreteP>());

        const float lowerQ = *(hyperBox->getLowerBound<DiscreteQ>());
        const float upperQ = *(hyperBox->getUpperBound<DiscreteQ>());

        const float lowerMu = *(hyperBox->getLowerBound<DiscreteMu>());
        const float upperMu = *(hyperBox->getUpperBound<DiscreteMu>());

        const double perpS = recoHit.getArcLength2D();
        const double recoZ = recoHit.getRecoZ();

        const bool sameSign = SameSignChecker::sameSign(catZ(lowerP, lowerQ, lowerMu, perpS) - recoZ,
                                                        catZ(lowerP, lowerQ, upperMu, perpS) - recoZ,
                                                        catZ(lowerP, upperQ, lowerMu, perpS) - recoZ,
                                                        catZ(lowerP, upperQ, upperMu, perpS) - recoZ,
                                                        catZ(upperP, lowerQ, lowerMu, perpS) - recoZ,
                                                        catZ(upperP, lowerQ, upperMu, perpS) - recoZ,
                                                        catZ(upperP, upperQ, lowerMu, perpS) - recoZ,
                                                        catZ(upperP, upperQ, upperMu, perpS) - recoZ);
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
      static bool compareDistances(const HoughBox& hyperBox, const CDCRecoHit3D& lhsRecoHit, const CDCRecoHit3D& rhsRecoHit)
      {
        const float centerP = *(hyperBox.getCenter<0>()); //TODO getCenter(class T) is not implemented
        const float centerQ = *(hyperBox.getCenter<1>());
        const float centerMu = *(hyperBox.getCenter<2>());

        const double lhsZ = lhsRecoHit.getRecoZ();
        const double rhsZ = rhsRecoHit.getRecoZ();

        const double lhsS = lhsRecoHit.getArcLength2D();
        const double rhsS = rhsRecoHit.getArcLength2D();

        const double lhsZDistance = catZ(centerP, centerQ, centerMu, lhsS) - lhsZ;
        const double rhsZDistance = catZ(centerP, centerQ, centerMu, rhsS) - rhsZ;

        return lhsZDistance < rhsZDistance;
      }

    private:
      static double catZ(const double p, const double q, const double mu, const double R)
      {
        //100 here is a reference  - size of CDC in cm
        return 100.0 * mu * (std::sqrt(1 - p * p) * std::cosh(R / 100.0 / mu / q + std::asinh(p / std::sqrt(1 - p * p))) - 1);
      }
    };
  }
}
