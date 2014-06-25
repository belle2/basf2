/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PortedKarimakisMethod.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PortedKarimakisMethod)


PortedKarimakisMethod::PortedKarimakisMethod() :
  _curved(true),
  _npar(_curved ? 3 : 2),
  _parameters(_npar),
  _covariance(_npar)
{
}



PortedKarimakisMethod::~PortedKarimakisMethod()
{
}



void PortedKarimakisMethod::update(CDCTrajectory2D& trajectory2D,
                                   CDCObservations2D& observations2D) const
{

  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;

  FloatType xRef = observations2D.getX(0);
  FloatType yRef = observations2D.getY(0);

  //Vector2D ref(xRef, yRef);
  //Vector2D ref = Vector2D(0.0, 0.0);
  Vector2D ref = observations2D.getCentralPoint();
  //Vector2D ref = Vector2D(0.5, 0.0);
  B2INFO("Reference point " << ref);

  observations2D.centralize(ref);

  double Chi2 = 0;
  int nPoints = 0;

  fit(observations2D, Chi2);

  FloatType curvature = _parameters(0);
  FloatType tangentialPhi = _parameters(1) + (_parameters(1) > 0 ? - PI : + PI);
  tangentialPhi = _parameters(1);
  FloatType impact = _parameters(2);

  UncertainPerigeeCircle perigeeCircle(curvature, tangentialPhi, impact, _covariance);

  FloatType frontX = observations2D.getX(0);
  FloatType frontY = observations2D.getY(0);
  Vector2D frontPos(frontX, frontY);

  FloatType backX = observations2D.getX(nObservations - 1);
  FloatType backY = observations2D.getY(nObservations - 1);
  Vector2D backPos(backX, backY);

  FloatType totalPerps = perigeeCircle.lengthOnCurve(frontPos, backPos);
  if (totalPerps < 0) {
    B2INFO("Reversed");
    perigeeCircle.reverse();
  }
  perigeeCircle.passiveMoveBy(-ref);
  trajectory2D.setCircle(perigeeCircle);

}




namespace {
  /// Helper indices for meaningfull matrix access
  constexpr size_t iW = 0;
  constexpr size_t iX = 1;
  constexpr size_t iY = 2;
  constexpr size_t iR2 = 3;
  constexpr size_t iL = 4;


  UncertainPerigeeCircle fitWithDriftLength(const Matrix< FloatType, 5, 5 >& sumMatrix)
  {
    //Solve the normal equation X * n = y
    Matrix< FloatType, 4, 4 > X = sumMatrix.block<4, 4>(0, 0);
    Matrix< FloatType, 4, 1 > y = sumMatrix.block<4, 1>(0, iL);
    Matrix< FloatType, 4, 1 > n = X.ldlt().solve(y);
    return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));
  }



  UncertainPerigeeCircle fitWithoutDriftLength(Matrix< FloatType, 5, 5 > sumMatrix)
  {
    // Solve the normal equation min_n  n^T * X * n
    // n is the smallest eigenvector
    Matrix< FloatType, 4, 4 > X = sumMatrix.block<4, 4>(0, 0);
    SelfAdjointEigenSolver< Matrix<FloatType, 4, 4> > eigensolver(X);
    Matrix<FloatType, 4, 1> n = eigensolver.eigenvectors().col(0);

    if (eigensolver.info() != Success) {
      B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    }
    return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));
  }



  UncertainPerigeeCircle fitWithoutDriftLengthSeperateOffset(Matrix< FloatType, 5, 1 > means,
                                                             Matrix< FloatType, 5, 5 > c)
  {
    // Solve the normal equation min_n  n^T * c * n
    // for the plane normal and move the plain by the offset
    // n is the smallest eigenvector
    Matrix< FloatType, 3, 3 > X = c.block<3, 3>(1, 1);
    SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(X);
    if (eigensolver.info() != Success) {
      B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    }

    //the eigenvalues are generated in increasing order
    //we are interested in the lowest one since we want to compute the normal vector of the plane
    Matrix<FloatType, 4, 1> n;
    n.middleRows<3>(iX) = eigensolver.eigenvectors().col(0);
    n(iW) = -means.middleRows<3>(iX).transpose() * n.middleRows<3>(iX);
    return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

  }



  UncertainPerigeeCircle fitWithoutDriftLengthOriginal(FloatType sw,
                                                       const Matrix< FloatType, 5, 1 >& a,
                                                       const Matrix< FloatType, 5, 5 >& c,
                                                       bool lineConstrained = false)
  {
    double q1, q2 = 0.0;
    if (not lineConstrained) {
      q1 = c(iX, iY) * c(iR2, iR2) - c(iX, iR2) * c(iY, iR2);
      q2 = (c(iX, iX) - c(iY, iY)) * c(iR2, iR2) - c(iX, iR2) * c(iX, iR2) + c(iY, iR2) * c(iY, iR2);
    } else {
      q1 = c(iX, iY);
      q2 = c(iX, iX) - c(iY, iY);
    }

    double phi = 0.5 * atan2(2. * q1, q2);

    double sinphi = sin(phi);
    double cosphi = cos(phi);

    double curv, I = 0.0;
    if (not lineConstrained) {
      double kappa = (sinphi * c(iX, iR2) - cosphi * c(iY, iR2)) / c(iR2, iR2);
      double delta = -kappa * a(iR2) + sinphi * a(iX) - cosphi * a(iY);

      curv = 2. * kappa / sqrt(1. - 4. * delta * kappa);
      I = 2. * delta / (1. + sqrt(1. - 4. * delta * kappa));

      double u = 1. + curv * I;

      double chi2 =  sw * u * u * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY, iY) - kappa * kappa * c(iR2, iR2));

    } else {
      curv = 0.0; //line
      I = sinphi * a(iX) - cosphi * a(iY);

      double chi2 = sw * (sinphi * sinphi * c(iX, iX) - 2. * sinphi * cosphi * c(iX, iY) + cosphi * cosphi * c(iY, iY));
    }

    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    phi += phi > 0 ? -PI : PI;

    return PerigeeCircle::fromPerigeeParameters(curv, phi, I);
  }
}



int PortedKarimakisMethod::fit(CDCObservations2D& observations2D, double& Chi2) const
{

  Matrix< FloatType, 5, 5 > s = observations2D.getWXYRLSumMatrix();
  Matrix< FloatType, 5, 5 > a = s / s(iW);

  Matrix< FloatType, 5, 1> means = a.row(iW);
  Matrix< FloatType, 5, 5> c = a - means * means.transpose();

  /// Fit te parameters
  UncertainPerigeeCircle resultCircle;

  size_t nObservationsWithDriftRadius = observations2D.getNObservationsWithDriftRadius();
  if (nObservationsWithDriftRadius > 0) {
    resultCircle = fitWithDriftLength(s);
  } else {
    //resultCircle = fitWithoutDriftLength(s);
    //resultCircle = fitWithoutDriftLengthSeperateOffset(means, c);
    resultCircle = fitWithoutDriftLengthOriginal(s(iW), means, c);
  }

  const FloatType& curv = resultCircle.curvature();
  const FloatType& I = resultCircle.impact();
  const FloatType& phi = resultCircle.tangentialPhi();

  _parameters[0] = curv;
  _parameters[1] = phi;
  _parameters[2] = I;

  B2INFO("Curvature " << curv);
  B2INFO("Tangential phi " << phi);
  B2INFO("Impact " << I);


  {
    // Karimaki uses the opposite sign for phi in contrast to the convention of this framework !!!
    const Vector2D vecPhi = -resultCircle.tangential();

    const FloatType& cosphi = vecPhi.x();
    const FloatType& sinphi = vecPhi.y();

    const FloatType ssphi = sinphi * sinphi;
    const FloatType scphi = sinphi * cosphi;
    const FloatType ccphi = cosphi * cosphi;

    const FloatType rho = curv;
    const FloatType& d = I;

    double u = 1. + rho * d;

    if (_curved) {
      // calculate covariance matrix
      double sa = sinphi * s(iX) - cosphi * s(iY);
      double sb = cosphi * s(iX) + sinphi * s(iY);
      double sc = (ssphi - ccphi) * s(iX, iY) + scphi * (s(iX, iX) - s(iY, iY));

      double sd = sinphi * s(iX, iR2) - cosphi * s(iY, iR2);

      double saa = ssphi * s(iX, iX) - 2. * scphi * s(iX, iY) + ccphi * s(iY, iY);

      // Not in the Karimaki paper, but factors a similar term.
      double se = cosphi * s(iX, iR2) + sinphi * s(iY, iR2);
      double sbb = ccphi * s(iX, iX) + 2. * scphi * s(iX, iY) + ssphi * s(iY, iY);

      _covariance(0, 0) = 0.25 * s(iR2, iR2) - d * (sd - d * (saa + 0.5 * s(iR2) - d * (sa - 0.25 * d * s(iW))));
      _covariance(0, 1) = u * (0.5 * se - d * (sc - 0.5 * d * sb));
      _covariance(1, 0) = _covariance(0, 1);
      _covariance(1, 1) = u * u * sbb;

      _covariance(0, 2) = rho * (0.5 * sd - d * saa) - 0.5 * u * s(iR2) + 0.5 * d * ((3.  * u - 1.) * sa - u * d * s(iW));
      _covariance(2, 0) = _covariance(0, 2);
      _covariance(1, 2) = u * (rho * sc - u * sb);
      _covariance(2, 1) = _covariance(1, 2);
      _covariance(2, 2) = rho * (rho * saa - 2 * u * sa) + u * u * s(iW);

    } else {
      _covariance(0, 0) = 0.;
      _covariance(0, 1) = 0.;
      _covariance(0, 2) = 0.;
      _covariance(1, 0) = 0.;
      _covariance(2, 0) = 0.;

      _covariance(1, 1) = ccphi * s(iX, iX) + 2. * scphi * s(iX, iY) + ssphi * s(iY, iY);
      _covariance(1, 2) = -(cosphi * s(iX) + sinphi * s(iY));
      _covariance(2, 1) = _covariance(0, 1);
      _covariance(2, 2) = s(iW);
    }
    _covariance.Invert();

    resultCircle.setPerigeeCovariance(_covariance);
  }



  //nPoints = _numPoints;
  return _npar;
}
