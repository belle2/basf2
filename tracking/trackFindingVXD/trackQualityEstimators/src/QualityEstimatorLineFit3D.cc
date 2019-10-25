/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Felix Metzner, Jonas Wagner         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorLineFit3D.h"
#include <math.h>
#include <TMath.h>

using namespace Belle2;

double QualityEstimatorLineFit3D::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  TVector3 directionVector;
  double sumWyi = 0, // sum of weights for Yi
         sumWzi = 0, // sum of weights for Zi
         sumWyiXi = 0, // sum of (y-weights times x-values)
         sumWziXi = 0, // sum of (z-weights times x-values)
         sumWyiYi = 0, // sum of (y-weights times y-values)
         sumWziZi = 0, // sum of (z-weights times z-values)
         sumWyiXiYi = 0, // sum of (y-weights times x-values times y-values)
         sumWziXiZi = 0, // sum of (z-weights times x-values times z-values)
         sumWyiXi2 = 0, // sum of (y-weights times x-values^2)
         sumWziXi2 = 0, // sum of (z-weights times x-values^2)
         detValY = 0, // determinant for norming values - y
         detValZ = 0, // determinant for norming values - z
         slopeY = 0, // = a of model
         slopeZ = 0, // = c of model
         chi2 = 0, // final chi2-value of fit
         interceptY = 0, // b of model, needed only for chi2-calculation
         interceptZ = 0; // d of model, needed only for chi2-calculation

  // NOTE: this approach is not optimal. Maybe can be optimized for less redundancy
  for (const SpacePoint* aHit : measurements) {
    double Wyi = (1. / (aHit->getPositionError().Y() * aHit->getPositionError().Y()));
    double Wzi = (1. / (aHit->getPositionError().Z() * aHit->getPositionError().Z()));

    sumWyi += Wyi;
    sumWzi += Wzi;

    sumWyiXi += Wyi * aHit->getPosition().X();
    sumWziXi += Wzi * aHit->getPosition().X();

    sumWyiYi += Wyi * aHit->getPosition().Y();
    sumWziZi += Wzi * aHit->getPosition().Z();

    sumWyiXiYi += Wyi * aHit->getPosition().X() * aHit->getPosition().Y();
    sumWziXiZi += Wzi * aHit->getPosition().X() * aHit->getPosition().Z();

    sumWyiXi2 += Wyi * aHit->getPosition().X() * aHit->getPosition().X();
    sumWziXi2 += Wzi * aHit->getPosition().X() * aHit->getPosition().X();
  }

  detValY = sumWyiXi2 * sumWyi - sumWyiXi * sumWyiXi;
  if (detValY == 0) {
    return 0;
  }
  detValY = 1. / detValY; // invert

  detValZ = sumWziXi2 * sumWzi - sumWziXi * sumWziXi;
  if (detValZ == 0) {
    return 0;
  }
  detValZ = 1. / detValZ; // invert

  slopeY = detValY * (sumWyi * sumWyiXiYi  -  sumWyiXi * sumWyiYi);
  slopeZ = detValZ * (sumWzi * sumWziXiZi  -  sumWziXi * sumWziZi);

  interceptY = detValY * (- sumWyiXi * sumWyiXiYi  +  sumWyiXi2 * sumWyiYi);
  interceptZ = detValZ * (- sumWziXi * sumWziXiZi  +  sumWziXi2 * sumWziZi);

  for (const SpacePoint* aHit : measurements) {  // chi2 of xy-fit and of xz-fit can be combined by adding their values
    chi2 += pow(((aHit->getPosition().Y() - slopeY * aHit->getPosition().X() - interceptY) / aHit->getPositionError().Y()) , 2)
            + pow(((aHit->getPosition().Z() - slopeZ * aHit->getPosition().X() - interceptZ) / aHit->getPositionError().Z()) , 2);
  }
  m_results.chiSquared = chi2;

  //m_results.p = B2Vector3<double>(1, slopeY, slopeZ);

  return TMath::Prob(chi2,  measurements.size() - 1);
}

