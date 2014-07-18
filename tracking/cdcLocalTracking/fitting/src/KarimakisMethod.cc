/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/KarimakisMethod.h"

#include "TMatrixDSym.h"
#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(KarimakisMethod)


KarimakisMethod::KarimakisMethod() :
  m_lineConstrained(false)
{
}



KarimakisMethod::~KarimakisMethod()
{
}



void KarimakisMethod::update(CDCTrajectory2D& trajectory2D,
                             CDCObservations2D& observations2D) const
{
  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;

  Vector2D ref = observations2D.getCentralPoint();
  //B2INFO("Reference point " << ref);

  observations2D.passiveMoveBy(ref);

  UncertainPerigeeCircle perigeeCircle = fit(observations2D);

  FloatType frontX = observations2D.getX(0);
  FloatType frontY = observations2D.getY(0);
  Vector2D frontPos(frontX, frontY);

  FloatType backX = observations2D.getX(nObservations - 1);
  FloatType backY = observations2D.getY(nObservations - 1);
  Vector2D backPos(backX, backY);

  FloatType totalPerps = perigeeCircle.lengthOnCurve(frontPos, backPos);
  if (totalPerps < 0) {
    //B2INFO("Reversed");
    perigeeCircle.reverse();
  }

  //perigeeCircle.passiveMoveBy(-ref);
  trajectory2D.setLocalOrigin(ref);
  trajectory2D.setLocalCircle(perigeeCircle);

  // Logical start position of the travel distance scale

}




namespace {
  /// Helper indices for meaningfull matrix access to the observations matrices
  constexpr size_t iW = 0;
  constexpr size_t iX = 1;
  constexpr size_t iY = 2;
  constexpr size_t iR2 = 3;

  /// Helper indices for meaningfull matrix access to the perigee covariance matrices
  constexpr size_t iCurv = 0;
  constexpr size_t iPhi = 1;
  constexpr size_t iI = 2;

  /// Variant implementing Karimakis method without drift circles.
  UncertainPerigeeCircle fitKarimaki(const FloatType& /*sw*/,
                                     const Matrix< FloatType, 4, 1 >& a,
                                     const Matrix< FloatType, 4, 4 >& c,
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
    phi += phi > 0 ? -PI : PI;
    return PerigeeCircle::fromPerigeeParameters(curv, phi, I);

  }





  /// Variant without drift circles
  FloatType calcChi2Karimaki(const PerigeeCircle& parameters,
                             const FloatType& sw,
                             const Matrix< FloatType, 4, 4 >& c,
                             bool lineConstrained = false)
  {
    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    const Vector2D vecPhi = -parameters.tangential();

    const FloatType& cosphi = vecPhi.x();
    const FloatType& sinphi = vecPhi.y();


    if (lineConstrained) {
      FloatType chi2 = sw * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY, iY));
      return chi2;
    } else {
      // Ternminology Karimaki used in the paper
      const FloatType& rho = parameters.curvature();
      const FloatType& d = parameters.impact();

      const FloatType u = 1 + d * rho;
      const FloatType kappa = 0.5 * rho / u;

      FloatType chi2 =  sw * u * u * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY, iY) - kappa * kappa * c(iR2, iR2));
      return chi2;
    }

  }



  Matrix<FloatType, 3, 3> calcCovarianceKarimaki(const PerigeeCircle& parameters,
                                                 const Matrix< FloatType, 4, 4 >& s,
                                                 bool lineConstrained = false)
  {
    Matrix<FloatType, 3, 3> invV;

    const FloatType& curv = parameters.curvature();
    const FloatType& I =  parameters.impact();

    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    const Vector2D vecPhi = -parameters.tangential();


    // Ternminology Karimaki using in the paper
    const FloatType& cosphi = vecPhi.x();
    const FloatType& sinphi = vecPhi.y();

    const FloatType ssphi = sinphi * sinphi;
    const FloatType scphi = sinphi * cosphi;
    const FloatType ccphi = cosphi * cosphi;

    const FloatType rho = curv;
    const FloatType& d = I;

    const double u = 1. + rho * d;

    if (lineConstrained) {
      invV(iCurv, iCurv) = 1.;
      invV(iCurv, iPhi) = 0.;
      invV(iCurv, iI) = 0.;
      invV(iPhi, iCurv) = 0.;
      invV(iI, iCurv) = 0.;

      invV(iPhi, iPhi) = ccphi * s(iX, iX) + 2. * scphi * s(iX, iY) + ssphi * s(iY, iY);
      invV(iPhi, iI) = -(cosphi * s(iX) + sinphi * s(iY));
      invV(iI, iPhi) = invV(iPhi, iI);
      invV(iI, iI) = s(iW);

      Matrix<FloatType, 3, 3> V = invV.inverse();
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
      invV(iCurv, iPhi) = - u * (0.5 * se - d * (sc - 0.5 * d * sb));
      invV(iPhi, iCurv) = invV(iCurv, iPhi);
      invV(iPhi, iPhi) = u * u * sbb;

      invV(iCurv, iI) = rho * (-0.5 * sd + d * saa) + 0.5 * u * s(iR2) - 0.5 * d * ((2 * u + rho * d) * sa - u * d * s(iW));
      invV(iI, iCurv) = invV(iCurv, iI);
      invV(iPhi, iI) = u * (rho * sc - u * sb);
      invV(iI, iPhi) = invV(iPhi, iI);
      invV(iI, iI) = rho * (rho * saa - 2 * u * sa) + u * u * s(iW);
      return invV.inverse();
    }

  }


} // end anonymuous namespace



UncertainPerigeeCircle KarimakisMethod::fit(CDCObservations2D& observations2D) const
{
  // Matrix of weighted sums
  Matrix< FloatType, 4, 4> sNoL = observations2D.getWXYRSumMatrix();

  // Matrix of averages
  Matrix< FloatType, 4, 4> aNoL = sNoL / sNoL(iW);

  // Measurement means
  Matrix< FloatType, 4, 1> meansNoL = aNoL.row(iW);

  // Covariance matrix
  Matrix< FloatType, 4, 4> cNoL = aNoL - meansNoL * meansNoL.transpose();

  // Parameters to be fitted
  UncertainPerigeeCircle resultCircle;
  FloatType chi2;
  Matrix< FloatType, 3, 3> perigeeCovariance;

  resultCircle = fitKarimaki(sNoL(iW), meansNoL, cNoL, isLineConstrained());
  chi2 = calcChi2Karimaki(resultCircle, sNoL(iW), cNoL);
  perigeeCovariance = calcCovarianceKarimaki(resultCircle, sNoL, isLineConstrained());

  resultCircle.setChi2(chi2);

  TMatrixDSym tPerigeeCovariance(3);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      tPerigeeCovariance(i, j) = perigeeCovariance(i, j);
    }
  }

  resultCircle.setPerigeeCovariance(tPerigeeCovariance);
  return resultCircle;
}
