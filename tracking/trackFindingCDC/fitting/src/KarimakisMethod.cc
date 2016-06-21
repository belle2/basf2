/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/KarimakisMethod.h>

#include <TMatrixDSym.h>
#include <Eigen/Dense>

using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace TrackFindingCDC;

KarimakisMethod::KarimakisMethod() :
  m_lineConstrained(false)
{
}



void KarimakisMethod::update(CDCTrajectory2D& trajectory2D,
                             CDCObservations2D& observations2D) const
{
  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;

  Vector2D ref = observations2D.getCentralPoint();
  observations2D.passiveMoveBy(ref);

  UncertainPerigeeCircle perigeeCircle = fitInternal(observations2D);

  double frontX = observations2D.getX(0);
  double frontY = observations2D.getY(0);
  Vector2D frontPos(frontX, frontY);

  double backX = observations2D.getX(nObservations - 1);
  double backY = observations2D.getY(nObservations - 1);
  Vector2D backPos(backX, backY);

  double totalPerps = perigeeCircle.arcLengthBetween(frontPos, backPos);
  if (totalPerps < 0) {
    perigeeCircle.reverse();
  }

  trajectory2D.setLocalOrigin(ref);
  trajectory2D.setLocalCircle(perigeeCircle);
}




namespace {
  /// Helper indices for meaningfull matrix access to the observations matrices
  constexpr size_t iW = 0;
  constexpr size_t iX = 1;
  constexpr size_t iY = 2;
  constexpr size_t iR2 = 3;

  /// Helper indices for meaningfull matrix access to the perigee covariance matrices
  constexpr size_t iCurv = 0;
  constexpr size_t iPhi0 = 1;
  constexpr size_t iI = 2;

  /// Variant implementing Karimakis method without drift circles.
  UncertainPerigeeCircle fitKarimaki(const double /*sw*/,
                                     const Matrix< double, 4, 1 >& a,
                                     const Matrix< double, 4, 4 >& c,
                                     bool lineConstrained = false)
  {
    double q1, q2 = 0.0;
    if (lineConstrained) {
      q1 = c(iX, iY);
      q2 = c(iX, iX) - c(iY, iY);
    } else {
      q1 = c(iX, iY) * c(iR2, iR2) - c(iX, iR2) * c(iY, iR2);
      q2 = (c(iX, iX) - c(iY, iY)) * c(iR2, iR2) - c(iX, iR2) * c(iX, iR2) + c(iY, iR2) * c(iY, iR2);
    }

    double phi = 0.5 * atan2(2. * q1, q2);

    double sinphi = sin(phi);
    double cosphi = cos(phi);

    double curv, I = 0.0;
    if (lineConstrained) {
      curv = 0.0; //line
      I = sinphi * a(iX) - cosphi * a(iY);

    } else {
      double kappa = (sinphi * c(iX, iR2) - cosphi * c(iY, iR2)) / c(iR2, iR2);
      double delta = -kappa * a(iR2) + sinphi * a(iX) - cosphi * a(iY);
      curv = 2. * kappa / sqrt(1. - 4. * delta * kappa);
      I = 2. * delta / (1. + sqrt(1. - 4. * delta * kappa));

    }

    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    phi += phi > 0 ? -M_PI : M_PI;
    return PerigeeCircle::fromPerigeeParameters(curv, phi, I);

  }





  /// Variant without drift circles
  double calcChi2Karimaki(const PerigeeCircle& parameters,
                          const double sw,
                          const Matrix< double, 4, 4 >& c,
                          bool lineConstrained = false)
  {
    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    const Vector2D vecPhi = -parameters.tangential();

    const double cosphi = vecPhi.x();
    const double sinphi = vecPhi.y();


    if (lineConstrained) {
      double chi2 = sw * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY, iY));
      return chi2;
    } else {
      // Terminology Karimaki used in the paper
      const double rho = parameters.curvature();
      const double d = parameters.impact();

      const double u = 1 + d * rho;
      const double kappa = 0.5 * rho / u;

      double chi2 =  sw * u * u * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY,
                                   iY) - kappa * kappa * c(iR2, iR2));
      return chi2;
    }

  }



  Matrix<double, 3, 3> calcCovarianceKarimaki(const PerigeeCircle& parameters,
                                              const Matrix< double, 4, 4 >& s,
                                              bool lineConstrained = false)
  {
    Matrix<double, 3, 3> invV;

    const double curv = parameters.curvature();
    const double I =  parameters.impact();

    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    const Vector2D vecPhi = -parameters.tangential();


    // Ternminology Karimaki using in the paper
    const double cosphi = vecPhi.x();
    const double sinphi = vecPhi.y();

    const double ssphi = sinphi * sinphi;
    const double scphi = sinphi * cosphi;
    const double ccphi = cosphi * cosphi;

    const double rho = curv;
    const double d = I;

    const double u = 1. + rho * d;

    if (lineConstrained) {
      invV(iCurv, iCurv) = 1.;
      invV(iCurv, iPhi0) = 0.;
      invV(iCurv, iI) = 0.;
      invV(iPhi0, iCurv) = 0.;
      invV(iI, iCurv) = 0.;

      invV(iPhi0, iPhi0) = ccphi * s(iX, iX) + 2. * scphi * s(iX, iY) + ssphi * s(iY, iY);
      invV(iPhi0, iI) = -(cosphi * s(iX) + sinphi * s(iY));
      invV(iI, iPhi0) = invV(iPhi0, iI);
      invV(iI, iI) = s(iW);

      Matrix<double, 3, 3> V = invV.inverse();
      V(iCurv, iCurv) = 0.;
      return V;
    } else {
      double sa = sinphi * s(iX) - cosphi * s(iY);
      double sb = cosphi * s(iX) + sinphi * s(iY);
      double sc = (ssphi - ccphi) * s(iX, iY) + scphi * (s(iX, iX) - s(iY, iY));

      double sd = sinphi * s(iX, iR2) - cosphi * s(iY, iR2);

      double saa = ssphi * s(iX, iX) - 2. * scphi * s(iX, iY) + ccphi * s(iY, iY);

      // Not in the Karimaki paper, but factors a similar term.
      double se = cosphi * s(iX, iR2) + sinphi * s(iY, iR2);
      double sbb = ccphi * s(iX, iX) + 2. * scphi * s(iX, iY) + ssphi * s(iY, iY);

      invV(iCurv, iCurv) = 0.25 * s(iR2, iR2) - d * (sd - d * (saa + 0.5 * s(iR2) - d * (sa - 0.25 * d * s(iW))));
      invV(iCurv, iPhi0) = - u * (0.5 * se - d * (sc - 0.5 * d * sb));
      invV(iPhi0, iCurv) = invV(iCurv, iPhi0);
      invV(iPhi0, iPhi0) = u * u * sbb;

      invV(iCurv, iI) = rho * (-0.5 * sd + d * saa) + 0.5 * u * s(iR2) - 0.5 * d * ((2 * u + rho * d) * sa - u * d * s(iW));
      invV(iI, iCurv) = invV(iCurv, iI);
      invV(iPhi0, iI) = u * (rho * sc - u * sb);
      invV(iI, iPhi0) = invV(iPhi0, iI);
      invV(iI, iI) = rho * (rho * saa - 2 * u * sa) + u * u * s(iW);
      return invV.inverse();
    }

  }


} // end anonymuous namespace



UncertainPerigeeCircle KarimakisMethod::fitInternal(CDCObservations2D& observations2D) const
{
  // Matrix of weighted sums
  Matrix< double, 4, 4> sNoL = observations2D.getWXYRSumMatrix();

  // Matrix of averages
  Matrix< double, 4, 4> aNoL = sNoL / sNoL(iW);

  // Measurement means
  Matrix< double, 4, 1> meansNoL = aNoL.row(iW);

  // Covariance matrix
  Matrix< double, 4, 4> cNoL = aNoL - meansNoL * meansNoL.transpose();

  // Determine NDF : Circle fit eats up to 3 degrees of freedom debpending on the constraints
  size_t ndf = observations2D.size() - 3;

  if (not isLineConstrained()) {
    --ndf;
  }

  // Parameters to be fitted
  UncertainPerigeeCircle resultCircle;
  double chi2;
  Matrix< double, 3, 3> perigeeCovariance;

  resultCircle = fitKarimaki(sNoL(iW), meansNoL, cNoL, isLineConstrained());
  chi2 = calcChi2Karimaki(resultCircle, sNoL(iW), cNoL);
  perigeeCovariance = calcCovarianceKarimaki(resultCircle, sNoL, isLineConstrained());



  resultCircle.setChi2(chi2);
  resultCircle.setNDF(ndf);

  TMatrixDSym tPerigeeCovariance(3);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      tPerigeeCovariance(i, j) = perigeeCovariance(i, j);
    }
  }

  resultCircle.setPerigeeCovariance(PerigeeCovariance(tPerigeeCovariance));
  return resultCircle;
}
