/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calculation of shower positions using lists of digits and weights.     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS
#include "ecl/utility/Position.h"

// GEOMETRY
#include <ecl/geometry/ECLGeometryPar.h>

// FRAMEWORK
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

// OTHER
#include "TMath.h" //log, exp, sin

namespace Belle2 {
  namespace ECL {
    // computePositionLiLo
//    const double halfLength = 15.0 * Belle2::Unit::cm; // crystal half length
    TVector3 computePositionLiLo(std::vector<ECLCalDigit>& digits, std::vector<double>& weights, std::vector<double>& parameters)
    {

      // Total weighted sum.
      const double energySum = computeEnergySum(digits, weights);
      if (energySum <= 0.0) {
        B2ERROR("ECL computePositionLiLo() energy sum zero or negative: " << energySum);
        TVector3 liloPointError(0, 0, 0);
        return liloPointError;
      }

      // Offset from user input constants: offset = A-B * exp(-C * energysum).
      const double offset = parameters[0] - parameters[1] * TMath::Exp(-parameters[2] * energySum);

      // Geometry information for crystal positions.
      ECLGeometryPar* geom = ECLGeometryPar::Instance();

      // Log-weighted position calculation
      TVector3 liloPoint(1, 1, 1);  // anything is fine as long its not zero length
      TVector3 linearPositionVector(0, 0, 0);
      TVector3 logPositionVector(0, 0, 0);
      double logWeightSum = 0.0;
      int nTotal = 0;
      int nInLogWeightSum = 0;

      // Loop over all digits in the vector.
      for (const auto& digit : digits) {
        const double energy = digit.getEnergy();
        const int cellid    = digit.getCellId();
        const double weight = weights.at(nTotal);
        ++nTotal;

        // Crystal position and direction.
        const TVector3 position  = geom->GetCrystalPos(cellid - 1); // v = crystal center - (0, 0, 0)

        // Weights.
        const double linearWeight = energy * weight / energySum; // fraction of this digit energy to the total shower energy
        const double logWeight    = offset + TMath::Log(linearWeight);

        // Update the linear position vector.
        linearPositionVector += linearWeight * position;

        // Only digits with a positive weight are included.
        if (logWeight > 0.0) {

          // Update the log position vector.
          logPositionVector += logWeight * position;
          logWeightSum += logWeight;
          ++nInLogWeightSum;
        } // end if logWeight
      } // end digit

      // Check if at least some digits have a positive weight
      const int nMinInLogWeightSum = 1;
      if (nInLogWeightSum > nMinInLogWeightSum) logPositionVector *= 1. / logWeightSum;

      liloPoint.SetTheta(nInLogWeightSum > nMinInLogWeightSum ? logPositionVector.Theta() : linearPositionVector.Theta());
      liloPoint.SetPhi(nInLogWeightSum > nMinInLogWeightSum ? logPositionVector.Phi() : linearPositionVector.Phi());

      // The direction is filled, now get the distance to the center. It can happen, that it is outside of the crystals
      // if the shower is in the barrel/endcap overlap regions?!
      //      const int closestCrystalId = findClosestCrystal(digits, liloPoint);

      if (nInLogWeightSum > nMinInLogWeightSum) {
        liloPoint.SetMag(logPositionVector.Mag());
      } else {
        liloPoint.SetMag(linearPositionVector.Mag());
      }

      return liloPoint;
    }

    // helper: computeEnergySum
    double computeEnergySum(std::vector<ECLCalDigit>& digits, std::vector<double>& weights)
    {
      int n = 0;
      double sum = 0.0;

      for (const auto& digit : digits) {
        sum += digit.getEnergy() * weights.at(n);
        ++n;
      }

      return sum;
    }

    // helper: findClosestCrystal
    int findClosestCrystal(std::vector<ECLCalDigit>& digits, TVector3& direction)
    {
      ECLGeometryPar* geom = ECLGeometryPar::Instance();

      int best = -1;
      double min = 999.;
      for (const auto& digit : digits) {
        const int crystalid   = digit.getCellId() - 1;
        const double alpha    = direction.Angle(geom->GetCrystalVec(crystalid));
        const double R        = (geom->GetCrystalPos(crystalid)).Mag();
        const double distance = 2.0 * R * TMath::Sin(alpha / 2.0); // chord distance

        if (distance < min) {
          min = distance;
          best = crystalid;
        }
      }
      return best;
    }

    // ...

  }
}
