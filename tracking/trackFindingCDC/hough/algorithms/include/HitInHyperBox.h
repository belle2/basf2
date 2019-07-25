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

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** An algorithm to check if a hit is contained in a hyperbolic cosine hough box
    *
    * The parameterization is
    * z(R) = 1/rho * sqrt(1 + mu**2 * rho**2) * (cosh(rho(R - alpha)) - cosh(rho*alpha))
    *
    * rho is g*c*B / p_t
    * mu is m / g*c*B
    * alpha is R position of catenary vertex
    *
    * The catenary passes through the origin
    *
    * TODO maybe there is a better parameterization
    */
    class HitInHyperBox {

    public:
      using HoughBox = Box<DiscreteAlpha, DiscreteRho, DiscreteMu>;

      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* hyperBox)
      {
        const float lowerAlpha = *(hyperBox->getLowerBound<DiscreteAlpha>()); //DiscreteValue is based on std::vector<T>::const_iterator
        const float upperAlpha = *(hyperBox->getUpperBound<DiscreteAlpha>());

        const float lowerMu = *(hyperBox->getLowerBound<DiscreteMu>());
        const float upperMu = *(hyperBox->getUpperBound<DiscreteMu>());

        const float lowerRho = *(hyperBox->getLowerBound<DiscreteRho>());
        const float upperRho = *(hyperBox->getUpperBound<DiscreteRho>());

        const double perpS = recoHit.getArcLength2D();
        const double recoZ = recoHit.getRecoZ();

        const bool sameSign = SameSignChecker::sameSign(catZ(lowerAlpha, lowerMu, lowerRho, perpS) - recoZ,
                                                        catZ(lowerAlpha, lowerMu, upperRho, perpS) - recoZ,
                                                        catZ(lowerAlpha, upperMu, lowerRho, perpS) - recoZ,
                                                        catZ(lowerAlpha, upperMu, upperRho, perpS) - recoZ,
                                                        catZ(upperAlpha, lowerMu, lowerRho, perpS) - recoZ,
                                                        catZ(upperAlpha, lowerMu, upperRho, perpS) - recoZ,
                                                        catZ(upperAlpha, upperMu, lowerRho, perpS) - recoZ,
                                                        catZ(upperAlpha, upperMu, upperRho, perpS) - recoZ);
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
        const float centerAlpha = *(hyperBox.getCenter<0>()); //TODO getCenter(class T) is not implemented
        const float centerRho = *(hyperBox.getCenter<1>());
        const float centerMu = *(hyperBox.getCenter<2>());

        const double lhsZ = lhsRecoHit.getRecoZ();
        const double rhsZ = rhsRecoHit.getRecoZ();

        const double lhsS = lhsRecoHit.getArcLength2D();
        const double rhsS = rhsRecoHit.getArcLength2D();

        const double lhsZDistance = catZ(centerAlpha, centerMu, centerRho, lhsS) - lhsZ;
        const double rhsZDistance = catZ(centerAlpha, centerMu, centerRho, rhsS) - rhsZ;

        return lhsZDistance < rhsZDistance;
      }

    private:
      static double catZ(const double alpha, const double mu, const double rho, const double R)
      {
        //100 here is a reference  - size of CDC in cm
        return 100.0 / rho * std::sqrt(1.0 + mu * mu * rho * rho) *
               (std::cosh((R / 100.0 - alpha) * rho) -
                std::cosh(alpha * rho));
      }
    };
  }
}
