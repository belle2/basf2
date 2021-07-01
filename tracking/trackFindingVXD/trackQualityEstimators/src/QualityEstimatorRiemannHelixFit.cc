/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h"
#include <tracking/trackFindingVXD/utilities/CalcCurvatureSignum.h>
#include <math.h>
#include <Eigen/Dense>
#include <framework/logging/Logger.h>
#include <TMath.h>

using namespace Belle2;
// Set precision to be used for Eigen Library
typedef double Precision;

double QualityEstimatorRiemannHelixFit::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  const int nHits = measurements.size();
  if (nHits < 3) return 0;

  // Circle Fit

  Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic> W = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>::Zero(nHits,
      nHits);
  Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic> Wz = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>::Zero(nHits,
      nHits);
  Eigen::Matrix<Precision, Eigen::Dynamic, 3> X = Eigen::Matrix<Precision, Eigen::Dynamic, 3>::Zero(nHits, 3);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> Z = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Zero(nHits, 1);
  Precision traceOfW = 0.;

  short index = 0;
  for (SpacePoint const* hit : measurements) {
    auto position = hit->getPosition();
    auto positionError = hit->getPositionError();
    double x = position.X();
    double y = position.Y();
    double z = position.Z();
    double sigmaX = positionError.X();
    double sigmaY = positionError.Y();
    double sigmaZ = positionError.Z();

    double r2 = x * x + y * y;
    double inverseVarianceXY = 1. / sqrt(sigmaX * sigmaX + sigmaY * sigmaY);

    // The following weight matrix W can be improved for cases with multiple scattering
    // by adding a correction term which will make the matrix non-diagonal.
    // However, this requires prior knowledge of the curvature of the track and thus a
    // second iteration (see page 368 of above mentioned source).
    W(index, index) = inverseVarianceXY;
    traceOfW += inverseVarianceXY;

    X(index, 0) = x;
    X(index, 1) = y;
    X(index, 2) = r2;

    // Values for z line fit for extrended Riemann fit
    Wz(index, index) = 1. / sigmaZ;
    Z(index, 0) = z;

    index++;
  }

  Eigen::Matrix<Precision, 1, 3> xBar = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1).transpose() * W * X / traceOfW;
  Eigen::Matrix<Precision, 3, 3> Vx = X.transpose() * W * X - xBar.transpose() * xBar * traceOfW;

  // Find eigenvector to smallest eigenvalue
  Eigen::EigenSolver<Eigen::Matrix<Precision, 3, 3>> eigencollection(Vx);
  Eigen::Matrix<Precision, 3, 1> eigenvalues = eigencollection.eigenvalues().real();
  Eigen::Matrix<std::complex<Precision>, 3, 3> eigenvectors = eigencollection.eigenvectors();
  Eigen::Matrix<Precision, 3, 1>::Index minRow, minCol;
  eigenvalues.minCoeff(&minRow, &minCol);

  Eigen::Matrix<Precision, 3, 1> n = eigenvectors.col(minRow).real();

  // Calculate results with this eigenvector
  Precision c = - xBar * n;
  Precision x0 = - 0.5 * n(0) / n(2);
  Precision y0 = - 0.5 * n(1) / n(2);
  Precision rho2 = (1 - n(2) * (n(2) + 4 * c)) / (4 * n(2) * n(2));
  Precision rho = sqrt(rho2);

  // calculation of chi2 for circle fit using Karimaeki circle fit
  Precision divisor = 1. / traceOfW;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> unitvec = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1);
  Precision meanX = unitvec.transpose() * W * X.col(0);
  meanX *= divisor;
  Precision meanY = unitvec.transpose() * W * X.col(1);
  meanY *= divisor;
  Precision meanXY = unitvec.transpose() * W * (X.col(0).cwiseProduct(X.col(1)));
  meanXY *= divisor;
  Precision meanX2 = unitvec.transpose() * W * (X.col(0).cwiseProduct(X.col(0)));
  meanX2 *= divisor;
  Precision meanY2 = unitvec.transpose() * W * (X.col(1).cwiseProduct(X.col(1)));
  meanY2 *= divisor;
  Precision meanXR2 = unitvec.transpose() * W * (X.col(0).cwiseProduct(X.col(2)));
  meanXR2 *= divisor;
  Precision meanYR2 = unitvec.transpose() * W * (X.col(1).cwiseProduct(X.col(2)));
  meanYR2 *= divisor;
  Precision meanR2 = unitvec.transpose() * W * X.col(2);
  meanR2 *= divisor;
  Precision meanR4 = unitvec.transpose() * W * (X.col(2).cwiseProduct(X.col(2)));
  meanR4 *= divisor;

  // covariances:
  Precision covXX = meanX2 - meanX * meanX;
  Precision covXY = meanXY - meanX * meanY;
  Precision covYY = meanY2 - meanY * meanY;
  Precision covXR2 = meanXR2 - meanX * meanR2;
  Precision covYR2 = meanYR2 - meanY * meanR2;
  Precision covR2R2 = meanR4 - meanR2 * meanR2;

  // q1, q2: helping variables, to make the code more readable
  Precision q1 = covR2R2 * covXY - covXR2 * covYR2;
  Precision q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  Precision pocaPhi = 0.5 * atan2(2. * q1, q2);

  Precision sinPhi = sin(pocaPhi);
  Precision cosPhi = cos(pocaPhi);
  Precision kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  Precision delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  Precision rootTerm = sqrt(1. - 4.*delta * kappa);
  Precision curvature = 2.*kappa / (rootTerm);
  Precision pocaD = 2.*delta / (1. + rootTerm);
  short curvatureSign = calcCurvatureSignum(measurements);

  if ((curvature < 0 && curvatureSign >= 0) || (curvature > 0 && curvatureSign < 0)) {
    curvature = -curvature;
    // pocaPhi = pocaPhi + M_PI; //
    pocaD = -pocaD;
  }

  Precision chi2 = traceOfW * (1. + pocaD / rho) * (1. + curvature * pocaD) *
                   (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);

  // Arc length calculation
  Precision x_first = X.col(0)(0) - x0;
  Precision y_first = X.col(1)(0) - y0;
  Precision r_mag_first = sqrt(x_first * x_first + y_first * y_first);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> x0s = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * x0;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> y0s = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * y0;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> r_mags = ((X.col(0) - x0s).cwiseProduct(X.col(0) - x0s) + (X.col(1) - y0s).cwiseProduct(
                                                          X.col(1) - y0s)).cwiseSqrt();

  Eigen::Matrix<Precision, Eigen::Dynamic, 1> arc_angles = (x_first * (X.col(0) - x0s) + y_first * (X.col(1) - y0s)).cwiseQuotient(
                                                             r_mags) / r_mag_first;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> arc_lengths = rho * arc_angles.array().acos().matrix();

  Eigen::Matrix<Precision, Eigen::Dynamic, 2> A = Eigen::Matrix<Precision, Eigen::Dynamic, 2>::Ones(nHits, 2);
  arc_lengths(0) = 0.;
  A.col(1) = arc_lengths;

  // Linear regression of z on arg_lengths with weight matrix Wz
  Eigen::Matrix<Precision, 2, 1> p = (A.transpose() * Wz * A).inverse() * A.transpose() * Wz * Z;

  // Construction of momentum vector with innermost hit and reconstructed circle center
  Eigen::Matrix<Precision, 3, 1> momVec = Eigen::Matrix<Precision, 3, 1>::Zero();
  momVec(0) = y0 - X.col(1)(0);
  momVec(1) = - (x0 - X.col(0)(0));

  Precision pT = Precision(calcPt(rho));
  momVec = pT * momVec.normalized();

  Eigen::Matrix<Precision, 3, 1> vec01 = X.row(1) - X.row(0);
  vec01(2) = Z(1) - Z(0);
  Precision angle01 = std::acos(vec01.dot(momVec) / momVec.norm() / vec01.norm());
  if (angle01 > 0.5 * M_PI) { momVec *= -1.; }

  // Calculation of a chi2 distributed quantity for the quality of fit of the z component fit.
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> ones = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1);
  Precision chi2_z = ((Z - p(0) * ones - p(1) * arc_lengths).cwiseQuotient(Wz * ones)).transpose() * ((Z - p(0) * ones - p(
                       1) * arc_lengths).cwiseQuotient(Wz * ones));

  // Adding chi2 and chi2_z, thus creating a chi2 distribution with (n-3) + (n-2) = 2n-5 degrees of freedom
  m_results.chiSquared = chi2 + chi2_z;
  B2DEBUG(75, "Chi Squared of extended Riemann = " << * (m_results.chiSquared) << std::endl);

  Precision pZ = pT * p(1);
  momVec(2) = pZ;
  m_results.pt = pT;
  m_results.p = B2Vector3D(momVec(0), momVec(1), momVec(2));
  m_results.curvatureSign = curvatureSign;
  m_results.pocaD = pocaD;

  return TMath::Prob(*(m_results.chiSquared), 2 * measurements.size() - 5);
}

