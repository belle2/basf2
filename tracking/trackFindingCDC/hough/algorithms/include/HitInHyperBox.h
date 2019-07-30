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
    * The exact parameterization is
    * z(R) = 1 / mu * (sqrt(1 - p * p) * cosh(R * mu / q + arcsinh(p / sqrt(1 - p * p))) - 1) + z0
    *
    * p is p_z / E - longitudinal fraction of energy
    * q is p_t / E - transverse fraction of energy
    * mu is gcB / E - inverse of scale; ranges from 0.0375 for E=6GeV, g=0.5 to 2.25 for E=1GeV, g=5.0
    * z0 is z at R=0
    * z and R are in units of CDC size
    *
    * If we apply a constraint z0=0,
    * The tree will not perform well as mu is highly correlated to q (and p)
    *
    * Instead we relax z0=0 and apply mu=q
    * Then for a better continuity around g=0 (sign flip),
    * we take 1/q as new q
    *
    * Resulting parameterization is
    * z(R) = q * (sqrt(1 - p * p) * cosh(R + arcsinh(p / sqrt(1 - p * p))) - 1) + z0
    * Which can still provide reasonable fits.
    *
    * Additionally, box edges are behaving more or less on same scale near origin and near edges of CDC if
    * z(R) = +-dQ + 1 / (q +- dQ) * (sqrt(1 - p * p) * cosh(R + arcsinh(p / sqrt(1 - p * p))) - 1) + z0
    */
    class HitInHyperBox {

    public:
      using HoughBox = Box<DiscreteP, DiscreteQ, DiscreteZ0>;

      Weight operator()(const CDCRecoHit3D& recoHit,
                        const HoughBox* hyperBox)
      {
        const float centerP = *(hyperBox->getCenter<0>()); //TODO getCenter(class T) is not implemented

        const float lowerQ = *(hyperBox->getLowerBound<DiscreteQ>()); //DiscreteValue is based on std::vector<T>::const_iterator
        const float upperQ = *(hyperBox->getUpperBound<DiscreteQ>());
        const float deltaQ = 0.5 * (upperQ - lowerQ);

        const float centerZ0 = *(hyperBox->getCenter<2>());

        const double perpS = recoHit.getArcLength2D();
        const double recoZ = recoHit.getRecoZ();

        const bool sameSign = SameSignChecker::sameSign(catZ(centerP, lowerQ, perpS) + centerZ0 - 100.0 * deltaQ - recoZ,
                                                        catZ(centerP, upperQ, perpS) + centerZ0 + 100.0 * deltaQ - recoZ); //100 is a reference - size of CDC in cm
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
        const float centerP = *(hyperBox.getCenter<0>());
        const float centerQ = *(hyperBox.getCenter<1>());
        const float centerZ0 = *(hyperBox.getCenter<2>());

        const double lhsZ = lhsRecoHit.getRecoZ();
        const double rhsZ = rhsRecoHit.getRecoZ();

        const double lhsS = lhsRecoHit.getArcLength2D();
        const double rhsS = rhsRecoHit.getArcLength2D();

        const double lhsZDistance = catZ(centerP, centerQ, lhsS) + centerZ0 - lhsZ;
        const double rhsZDistance = catZ(centerP, centerQ, rhsS) + centerZ0 - rhsZ;

        return lhsZDistance < rhsZDistance;
      }

    private:
      static double catZ(const double p, const double q, const double R)
      {
        //100 here is a reference  - size of CDC in cm
        return 100.0 * q * (std::sqrt(1 - p * p) * std::cosh(R / 100.0 + std::asinh(p / std::sqrt(1 - p * p))) - 1);
      }

    public:
      static float centerX(const HoughBox& hyperBox)
      {
        return *(hyperBox.getCenter<0>());
      }

      static float centerY(const HoughBox& hyperBox)
      {
        return *(hyperBox.getCenter<1>());
      }

      static float centerZ(const HoughBox& hyperBox)
      {
        return *(hyperBox.getCenter<2>());
      }

      static float deltaY(const HoughBox& hyperBox)
      {
        const float lowerQ = *(hyperBox.getLowerBound<DiscreteQ>()); //DiscreteValue is based on std::vector<T>::const_iterator
        const float upperQ = *(hyperBox.getUpperBound<DiscreteQ>());
        return 0.5 * (upperQ - lowerQ);
      }

      static const char* debugLine() { return "100.0 * [1] * (TMath::Sqrt(1 - [0] * [0]) * TMath::CosH(x / 100.0 + TMath::ASinH([0] / TMath::Sqrt(1 - [0] * [0]))) - 1) + [2]";}
    };
  }
}
