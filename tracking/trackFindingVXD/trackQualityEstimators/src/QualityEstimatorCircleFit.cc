/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h"
#include <tracking/trackFindingVXD/utilities/CalcCurvatureSignum.h>
#include <math.h>
#include <framework/logging/Logger.h>
#include <TMath.h>

using namespace Belle2;

double QualityEstimatorCircleFit::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  if (measurements.size() < 4) {
    if (measurements.size() == 3) return 0.2; // Arbitrary value to prevent excluding measurements with 3 hits later on.
    else return 0;
  }
  // Calculates Curvature: True means clockwise, False means counterclockwise.
  // TODO this is not an optimized approach; just to get things to work.
  // CalcCurvature could be integrated into the looping over the hits which CircleFit does anyhow.
  m_results.curvatureSign = calcCurvatureSignum(measurements);
  bool clockwise = *(m_results.curvatureSign) >= 0 ? true : false;

  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor/*, weightNormalizer = 0*/; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter =
    1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (const SpacePoint* hit : measurements) {
    weight = 1. / (sqrt(hit->getPositionError().X() * hit->getPositionError().X() + hit->getPositionError().Y() *
                        hit->getPositionError().Y()) * tuningParameter);
    B2DEBUG(100, " current hitSigmaU/V/X/Y: " << hit->getPositionError().X() << "/" <<
            hit->getPositionError().Y() << ", weight: " << weight);
    sumWeights += weight;
    if (std::isnan(weight) or std::isinf(weight) == true) { B2ERROR("QualityEstimators::circleFit, chosen sigma is 'nan': " << weight << ", setting arbitrary error: " << stopper << ")"); weight = stopper; }
    x = hit->getPosition().X();
    y = hit->getPosition().Y();
    x2 = x * x;
    y2 = y * y;
    r2 = x2 + y2;
    meanX += x * weight;
    meanY += y * weight;
    meanXY += x * y * weight;
    meanX2 += x2 * weight;
    meanY2 += y2 * weight;
    meanXR2 += x * r2 * weight;
    meanYR2 += y * r2 * weight;
    meanR2 += r2 * weight;
    meanR4 += r2 * r2 * weight;
  }
  divisor = 1. / sumWeights;
  meanX *= divisor;
  meanY *= divisor;
  meanXY *= divisor;
  meanY2 *= divisor;
  meanX2 *= divisor;
  meanXR2 *= divisor;
  meanYR2 *= divisor;
  meanR2 *= divisor;
  meanR4 *= divisor;

  // covariances:
  double covXX = meanX2 - meanX * meanX;
  double covXY = meanXY - meanX * meanY;
  double covYY = meanY2 - meanY * meanY;
  double covXR2 = meanXR2 - meanX * meanR2;
  double covYR2 = meanYR2 - meanY * meanR2;
  double covR2R2 = meanR4 - meanR2 * meanR2;

  if (covR2R2 == 0) {return 0;} // TODO could be problematic if it is pretty near to 0

  // q1, q2: helping variables, to make the code more readable
  double q1 = covR2R2 * covXY - covXR2 * covYR2;
  double q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  double pocaPhi = 0.5 * atan2(2. * q1 ,
                               q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(pocaPhi);
  double cosPhi = cos(pocaPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  double curvature = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  double pocaD = 2.*delta / (1. + rootTerm);

  if ((curvature < 0 && clockwise) || (curvature > 0
                                       && !clockwise)) { // Checks if the random Curvature of CircleFit corresponds to CalcCurvature and adjust the results accordingly.
    // this is according to eq. 23 in the paper of Karimäki
    curvature = -curvature;
    pocaPhi = pocaPhi + M_PI;
    pocaD = -pocaD;
    //TODO ..and swap correlation Terms V_rho_phi and V_rho_d (which are not implemented anyway)
  }
  double radius = 1. / curvature;
  double absRadius = fabs(radius);

  m_results.pt = calcPt(absRadius);

  double chi2 = sumWeights * (1. + curvature * pocaD) * (1. + curvature * pocaD) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi *
                covXY +
                cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);

  m_results.chiSquared = chi2;

  return TMath::Prob(chi2, measurements.size() - 3);
}

