/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calculation of shower positions using lists of digits and weights.     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS
#include "ecl/utility/Position.h"

// GEOMETRY
#include <ecl/geometry/ECLGeometryPar.h>

// FRAMEWORK
#include <framework/logging/Logger.h>

// OTHER
#include "TMath.h"

namespace Belle2 {
  namespace ECL {
    // computePositionLiLo
    B2Vector3D computePositionLiLo(const std::vector<ECLCalDigit>& digits, std::vector<double>& weights,
                                   const std::vector<double>& parameters)
    {

      // Total weighted sum.
      const double energySum = computeEnergySum(digits, weights);
      if (energySum <= 0.0) {
        B2WARNING("ECL computePositionLiLo() energy sum zero or negative: " << energySum << ", return (0, 0, 0)");
        B2Vector3D liloPointError(0, 0, 0);
        return liloPointError;
      }

      // Offset from user input constants: offset = A-B * exp(-C * energysum).
      const double offset = parameters[0] - parameters[1] * TMath::Exp(-parameters[2] * energySum);

      // Geometry information for crystal positions.
      ECLGeometryPar* geom = ECLGeometryPar::Instance();

      // Log-weighted position calculation
      B2Vector3D liloPoint(1, 1, 1);  // anything is fine as long its not zero length
      B2Vector3D linearPositionVector(0, 0, 0);
      B2Vector3D logPositionVector(0, 0, 0);
      double logWeightSum = 0.0;
      int nTotal = 0;
      int nInLogWeightSum = 0;
      double firstPhi = -999.;
      double firstTheta = -999.;
      bool foundSecondPhi = false;
      bool foundSecondTheta = false;

      // Loop over all digits in the vector.
      for (const auto& digit : digits) {
        const double energy = digit.getEnergy();
        const int cellid    = digit.getCellId();
        const double weight = weights.at(nTotal);
        const B2Vector3D position  = geom->GetCrystalPos(cellid - 1); // v = crystal center - (0, 0, 0)
        const double theta         = position.Theta();
        const double phi           = position.Phi();
        ++nTotal;

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

          // We have to avoid that only crystals with the same theta or phi values are used (this will bias the position towards the crystal center
          if (nInLogWeightSum == 1) {
            firstTheta = theta;
            firstPhi = phi;
          } else {
            if (!foundSecondPhi and fabs(phi - firstPhi) > 1e-4) { // we cant use phi id in the endcaps
              foundSecondPhi = true;
            }
            if (!foundSecondTheta
                and fabs(theta - firstTheta) > 1e-4) { // we could probably use thetaid since this works in the endcaps as well
              foundSecondTheta = true;
            }
          }
        } // end if logWeight
      } // end digit

      // Check if at least one digit has a positive weight in the logweightsum.
      if (nInLogWeightSum > 0) logPositionVector *= 1. / logWeightSum;

      // The direction is filled, now get the distance to the center. It can happen, that it is outside of the crystals
      // if the shower is in the barrel/endcap overlap regions?!
      liloPoint.SetTheta(foundSecondTheta ? logPositionVector.Theta() : linearPositionVector.Theta());
      liloPoint.SetPhi(foundSecondPhi ? logPositionVector.Phi() : linearPositionVector.Phi());
      liloPoint.SetMag(nInLogWeightSum > 0 ? logPositionVector.Mag() : linearPositionVector.Mag());

      return liloPoint;
    }

    // helper: computeEnergySum
    double computeEnergySum(const std::vector<ECLCalDigit>& digits, std::vector<double>& weights)
    {
      int n = 0;
      double sum = 0.0;

      for (const auto& digit : digits) {
        sum += digit.getEnergy() * weights.at(n);
        ++n;
      }

      return sum;
    }

//    // helper: findClosestCrystal
//    int findClosestCrystal(std::vector<ECLCalDigit>& digits, TVector3& direction)
//    {
//      ECLGeometryPar* geom = ECLGeometryPar::Instance();
//
//      int best = -1;
//      double min = 999.;
//      for (const auto& digit : digits) {
//        const int crystalid   = digit.getCellId() - 1;
//        const double alpha    = direction.Angle(geom->GetCrystalVec(crystalid));
//        const double R        = (geom->GetCrystalPos(crystalid)).Mag();
//        const double distance = 2.0 * R * TMath::Sin(alpha / 2.0); // chord distance
//
//        if (distance < min) {
//          min = distance;
//          best = crystalid;
//        }
//      }
//      return best;
//    }

    // ...

  }
}
