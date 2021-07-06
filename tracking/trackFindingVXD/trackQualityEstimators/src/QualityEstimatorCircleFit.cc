/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/utilities/CalcCurvatureSignum.h>

#include <framework/logging/Logger.h>

#include <TMath.h>
#include <cmath>

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
  bool clockwise = *(m_results.curvatureSign) >= 0;

  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double sumWeights = 0, divisor/*, weightNormalizer = 0*/; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter = 1.; //0.02; // this parameter is for internal tuning of the weights, 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (const SpacePoint* hit : measurements) {
    double weight = 1. / (sqrt(hit->getPositionError().X() * hit->getPositionError().X() + hit->getPositionError().Y() *
                               hit->getPositionError().Y()) * tuningParameter);
    sumWeights += weight;
    if (std::isnan(weight) or std::isinf(weight)) {
      B2ERROR("QualityEstimators::circleFit, chosen sigma is 'nan': " << weight << ", setting arbitrary error: "
              << stopper << ")"); weight = stopper;
    }
    double x = hit->getPosition().X();
    double y = hit->getPosition().Y();
    double x2 = x * x;
    double y2 = y * y;
    double r2 = x2 + y2;
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

  if (covR2R2 == 0) {
    return 0; // TODO could be problematic if it is pretty near to 0
  }

  // q1, q2: helping variables, to make the code more readable
  double q1 = covR2R2 * covXY - covXR2 * covYR2;
  double q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  // physical meaning: phi value of the point of closest approach of the fitted circle to the origin
  double pocaPhi = 0.5 * atan2(2. * q1, q2);

  double sinPhi = sin(pocaPhi);
  double cosPhi = cos(pocaPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  double curvature = 2.*kappa / (rootTerm);
  double pocaD = 2.*delta / (1. + rootTerm);

  // Checks if the random Curvature of CircleFit corresponds to CalcCurvature and adjust the results accordingly.
  if ((curvature < 0 && clockwise) || (curvature > 0 && !clockwise)) {
    // this is according to eq. 23 in the paper of Karimäki
    curvature = -curvature;
    pocaD = -pocaD;
    //TODO ..and swap correlation Terms V_rho_phi and V_rho_d (which are not implemented anyway)
  }
  double radius = 1. / curvature;
  double absRadius = fabs(radius);
  double pt = calcPt(absRadius);
  m_results.pt = pt;
  m_results.pocaD = pocaD;

  // Estimating theta of the track with the theta of the innermost hit.
  // Otherwise the track is solely a circle without theta information.
  double innermostHitTheta = 0.;
  double innermostHitRadiusSquared = 100000000;
  for (const SpacePoint* hit : measurements) {
    if (hit->getPosition().Perp2() < innermostHitRadiusSquared) {
      innermostHitRadiusSquared = hit->getPosition().Perp2();
      innermostHitTheta = hit->getPosition().Theta();
    }
  }
  // Account for precision when checking equality of innermostHitTheta with 0
  double pz = (innermostHitTheta <= 1e-6 ? 0 : pt / tan(innermostHitTheta));
  // Both p and pmag are only rough estimates based and by far not accutate!
  m_results.p = B2Vector3D(pt * cosPhi, pt * sinPhi, pz);
  m_results.pmag = sqrt(pt * pt + pz * pz);

  double chi2 = sumWeights * (1. + curvature * pocaD) * (1. + curvature * pocaD)
                * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);

  m_results.chiSquared = chi2;

  return TMath::Prob(chi2, measurements.size() - 3);
}

