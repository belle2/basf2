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

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

#include <cmath>

namespace Belle2 {
  namespace trackFindingCDC {

    /** An algorithm to check if a hit is contained in a hyperbolic cosine hough box
    *
    * The parameterization is
    * z(R) = 1/rho * sqrt(1 + mu**2 * rho**2) * (cosh(rho(R - alpha) - cosh(rho*alpha))
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
      using HoughBox = Box<DiscreteAlpha, DiscreteMu, DiscreteRho>;

      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* hyperBox)
      {
        const double lowerAlpha = hyperBox->getLowerBound<DiscreteAlpha>();
        const double upperAlpha = hyperBox->getUpperBound<DiscreteAlpha>();

        const double lowerMu = hyperBox->getLowerBound<DiscreteMu>();
        const double upperMu = hyperBox->getUpperBound<DiscreteMu>();

        const double lowerRho = hyperBox->getLowerBound<DiscreteRho>();
        const double upperRho = hyperBox->getUpperBound<DiscreteRho>();

        const double perpS = recoHit.getArcLength2D();
        const double recoZ = recoHit.getRecoZ();

        const bool sameSign = SameSignChecker::sameSign(deltaZ(lowerAlpha, lowerMu, lowerRho, perpS, recoZ),
                                                        deltaZ(lowerAlpha, lowerMu, upperRho, perpS, recoZ),
                                                        deltaZ(lowerAlpha, upperMu, lowerRho, perpS, recoZ),
                                                        deltaZ(lowerAlpha, upperMu, upperRho, perpS, recoZ),
                                                        deltaZ(upperAlpha, lowerMu, lowerRho, perpS, recoZ),
                                                        deltaZ(upperAlpha, lowerMu, upperRho, perpS, recoZ),
                                                        deltaZ(upperAlpha, upperMu, lowerRho, perpS, recoZ),
                                                        deltaZ(upperAlpha, upperMu, upperRho, perpS, recoZ));
        if (not sameSign) {
          return 1.0;
        } else {
          return NAN;
        }
      }

    private:
      double deltaZ(alpha, mu, rho, R, Z)
      {
        //100 here is a reference  - size of CDC in cm
        return 100.0 * (1.0 / rho) * std::sqrt(1 + mu * mu * rho * rho) *
               (std::cosh(rho * (R / 100.0 - alpha)) -
                std::cosh(alpha * rho)
               ) - Z;
      }
    }
  }
