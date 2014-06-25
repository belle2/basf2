/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/ExtendedRiemannsMethod.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(ExtendedRiemannsMethod)


ExtendedRiemannsMethod::ExtendedRiemannsMethod() :
  m_lineConstrained(false),
  m_originConstrained(false)
{
}



ExtendedRiemannsMethod::~ExtendedRiemannsMethod()
{
}



void ExtendedRiemannsMethod::update(CDCTrajectory2D& trajectory2D,
                                    CDCObservations2D& observations2D) const
{
  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;

  Vector2D ref = observations2D.getCentralPoint();
  B2INFO("Reference point " << ref);

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
    B2INFO("Reversed");
    perigeeCircle.reverse();
  }

  perigeeCircle.passiveMoveBy(-ref);

  trajectory2D.setCircle(perigeeCircle);

  // Logical start position of the travel distance scale
  trajectory2D.setStartPos2D(ref);
}




namespace {
  /// Helper indices for meaningfull matrix access to the observations matrices
  constexpr size_t iW = 0;
  constexpr size_t iX = 1;
  constexpr size_t iY = 2;
  constexpr size_t iR2 = 3;
  constexpr size_t iL = 4;

  /// Helper indices for meaningfull matrix access to the full generalized parameter covariance matrices
  constexpr size_t iN0 = 0;
  constexpr size_t iN1 = 1;
  constexpr size_t iN2 = 2;
  constexpr size_t iN3 = 3;

  /// Helper indices for meaningfull matrix access to the reduced generalized parameter covariance matrices
  constexpr size_t iReducedN0 = 0;
  constexpr size_t iReducedN2 = 1;
  constexpr size_t iReducedN3 = 2;

  /// Helper indices for meaningfull matrix access to the perigee covariance matrices
  constexpr size_t iCurv = 0;
  constexpr size_t iPhi = 1;
  constexpr size_t iI = 2;



  /// Variant with drift circles
  UncertainPerigeeCircle fit(const Matrix< FloatType, 5, 5 >& sumMatrix,
                             bool lineConstrained = false,
                             bool originConstrained = false)
  {
    // Solve the normal equation X * n = y
    if (lineConstrained) {
      if (originConstrained) {
        Matrix< FloatType, 2, 2> X = sumMatrix.block<2, 2>(1, 1);
        Matrix< FloatType, 2, 1> y = sumMatrix.block<2, 1>(1, iL);
        Matrix< FloatType, 2, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(0.0, n(0), n(1), 0.0);

      } else {
        Matrix< FloatType, 3, 3> X = sumMatrix.block<3, 3>(0, 0);
        Matrix< FloatType, 3, 1> y = sumMatrix.block<3, 1>(0, iL);
        Matrix< FloatType, 3, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), 0.0);

      }

    } else {
      if (originConstrained) {
        Matrix< FloatType, 3, 3> X = sumMatrix.block<3, 3>(1, 1);
        Matrix< FloatType, 3, 1> y = sumMatrix.block<3, 1>(1, iL);
        Matrix< FloatType, 3, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(0.0, n(0), n(1), n(2));

      } else {
        Matrix< FloatType, 4, 4> X = sumMatrix.block<4, 4>(0, 0);
        Matrix< FloatType, 4, 1> y = sumMatrix.block<4, 1>(0, iL);
        Matrix< FloatType, 4, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

      }
    }
  }


  /// Variant without drift circles
  UncertainPerigeeCircle fit(Matrix< FloatType, 4, 4 > sumMatrix,
                             bool lineConstrained = false,
                             bool originConstrained = false)
  {
    // Solve the normal equation min_n  n^T * X * n
    // n is the smallest eigenvector

    if (lineConstrained) {
      if (originConstrained) {
        Matrix< FloatType, 2, 2> X = sumMatrix.block<2, 2>(1, 1);
        SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(X);
        Matrix<FloatType, 2, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(0.0, n(0), n(1), 0.0);

      } else {
        Matrix< FloatType, 3, 3> X = sumMatrix.block<3, 3>(0, 0);
        SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(X);
        Matrix<FloatType, 3, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), 0.0);

      }
    } else {
      if (originConstrained) {
        Matrix< FloatType, 3, 3> X = sumMatrix.block<3, 3>(1, 1);
        SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(X);
        Matrix<FloatType, 3, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(0.0, n(0), n(1), n(2));

      } else {
        Matrix< FloatType, 4, 4 > X = sumMatrix.block<4, 4>(0, 0);
        SelfAdjointEigenSolver< Matrix<FloatType, 4, 4> > eigensolver(X);
        Matrix<FloatType, 4, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

      }
    }

  }


  /// Variant without drift circles and seperating the offset before the matrix solving
  UncertainPerigeeCircle fitSeperateOffset(Matrix< FloatType, 4, 1 > means,
                                           Matrix< FloatType, 4, 4 > c,
                                           bool lineConstrained = false)
  {
    // Solve the normal equation min_n  n^T * c * n
    // for the plane normal and move the plain by the offset
    // n is the smallest eigenvector

    if (lineConstrained) {
      Matrix< FloatType, 2, 2> X = c.block<2, 2>(1, 1);
      SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(X);
      if (eigensolver.info() != Success) {
        B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
      }

      //the eigenvalues are generated in increasing order
      //we are interested in the lowest one since we want to compute the normal vector of the plane
      Matrix<FloatType, 4, 1> n;
      n.middleRows<2>(iX) = eigensolver.eigenvectors().col(0);
      n(iW) = -means.middleRows<2>(iX).transpose() * n.middleRows<2>(iX);
      n(iR2) = 0.;
      return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

    } else {
      Matrix< FloatType, 3, 3> X = c.block<3, 3>(1, 1);
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

  }


  /// Variant with drift circles
  FloatType calcChi2(const PerigeeCircle& parameters,
                     const Matrix< FloatType, 5, 5 >& s)
  {

    Matrix<FloatType, 5, 1> n;
    n(0) = parameters.n0();
    n(1) = parameters.n1();
    n(2) = parameters.n2();
    n(3) = parameters.n3();
    n(4) = -1;

    FloatType chi2 = n.transpose() * s * n;
    return chi2;

  }


  /// Variant without drift circles
  FloatType calcChi2(const PerigeeCircle& parameters,
                     const Matrix< FloatType, 4, 4 >& s)
  {

    Matrix<FloatType, 4, 1> n;
    n(0) = parameters.n0();
    n(1) = parameters.n1();
    n(2) = parameters.n2();
    n(3) = parameters.n3();

    FloatType chi2 = n.transpose() * s * n;

    return chi2;
  }


  Matrix<FloatType, 3, 3> calcCovariance(const PerigeeCircle& parameters,
                                         const Matrix< FloatType, 4, 4 >& s,
                                         bool lineConstrained = false,
                                         bool originConstrained = false)
  {
    const FloatType n0 = parameters.n0();

    Vector2D n12 = parameters.n12();
    const FloatType n3 = parameters.n3();

    // 1. Passive rotation such that n12 = (n1, 0);

    // n12 in the rotated system will be:
    const FloatType rotN1 = n12.normalize();
    const FloatType rotN2 = 0.0;

    // Setup passive rotation matrix.
    Matrix< FloatType, 4, 4 > rot = Matrix< FloatType, 4, 4 >::Identity();
    rot(iX, iX) = n12.x();
    rot(iX, iY) = n12.y();

    rot(iY, iX) = -n12.y();
    rot(iY, iY) = n12.x();

    // 2. Reduce the four generalized parameters to three parameters lifting the normalization constraint
    // n1 * n1 + n2 * n2 - 4 n0*n3 = 1
    // in the rotated system
    // The rotation is needed because to assure n1 != 0 at the point the constraint needs to be inverted.
    // In case n12 = (0, 0) this breaks down.
    // However this correspondes to the case when the circle is centered on the origin, where the perigee parameterization is ill conditioned in the first place.
    Matrix< FloatType, 3, 4 > reduce = Matrix< FloatType, 3, 4 >::Zero();
    reduce(iReducedN0, iN0) = 1.;
    reduce(iReducedN2, iN2) = 1.;
    reduce(iReducedN3, iN3) = 1.;

    reduce(iReducedN0, iN1) = 2 * n3 / rotN1;
    reduce(iReducedN2, iN1) = -rotN2 / rotN1;
    reduce(iReducedN3, iN1) = 2 * n0 / rotN1;


    // Instead of doing the two transformations seperatly we combine matrices and apply a single transformation
    // This saves one matrix multiplication.
    // Matrix<FloatType, 4, 4> rotS = rot * s * rot.transpose();
    // Matrix<FloatType, 3, 3> reducedNInvV = reduce * rotS * reduce.transpose();

    Matrix< FloatType, 3, 4 > rotAndReduce = reduce * rot;
    Matrix< FloatType, 3, 3> reducedNInvCov = rotAndReduce * s * rotAndReduce.transpose();

    //Zero out the unconsidered Parameters before inversion. Keep one the diagonal.
    if (lineConstrained) {
      reducedNInvCov.row(iReducedN3) = Matrix<FloatType, 1, 3>::Zero();
      reducedNInvCov.col(iReducedN3) = Matrix<FloatType, 3, 1>::Zero();
      reducedNInvCov(iReducedN0, iReducedN3) = 1.;
    }

    if (originConstrained) {
      reducedNInvCov.row(iReducedN0) = Matrix<FloatType, 1, 3>::Zero();
      reducedNInvCov.col(iReducedN0) = Matrix<FloatType, 3, 1>::Zero();
      reducedNInvCov(iReducedN0, iReducedN0) = 1.;
    }

    Matrix< FloatType, 3, 3> reducedNCov = reducedNInvCov.inverse();

    if (lineConstrained) {
      reducedNCov(iReducedN3, iReducedN3) = 0.;
    }

    if (originConstrained) {
      reducedNCov(iReducedN0, iReducedN0) = 0.;
    }

    // 3. Translate to perigee
    Matrix< FloatType, 3, 3 > perigeeJ;

    FloatType normN12 = fabs(rotN1); // = 1 / hypot(n1, n2);
    FloatType denominator = (1 + normN12) * (1 + normN12) * normN12;
    perigeeJ(iCurv, iReducedN0) = 0;
    perigeeJ(iCurv, iReducedN2) = 0;
    perigeeJ(iCurv, iReducedN3) = 2;

    perigeeJ(iPhi, iReducedN0) = 0;
    perigeeJ(iPhi, iReducedN2) = -1 / normN12;
    perigeeJ(iPhi, iReducedN3) = 0;

    perigeeJ(iI, iReducedN0) = 2 * (normN12 * (1 + 2 * normN12) - 1) / denominator;
    perigeeJ(iI, iReducedN2) = 0;
    perigeeJ(iI, iReducedN3) =  -4 * n0 * n0 /  denominator;

    Matrix< FloatType, 3, 3 > perigeeCov = perigeeJ * reducedNCov * perigeeJ.transpose();

    return perigeeCov;

  }

} // end anonymuous namespace



UncertainPerigeeCircle ExtendedRiemannsMethod::fit(CDCObservations2D& observations2D) const
{
  // Matrix of weighted sums
  Matrix< FloatType, 5, 5 > s = observations2D.getWXYRLSumMatrix();

  // The same as above without drift lengths
  Matrix<FloatType, 4, 4> sNoL = s.block<4, 4>(0, 0);

  // Matrix of averages
  //Matrix< FloatType, 4, 4> aNoL = sNoL / sNoL(iW);
  // Measurement means
  //Matrix< FloatType, 4, 1> meansNoL = aNoL.row(iW);
  // Covariance matrix
  //Matrix< FloatType, 4, 4> cNoL = aNoL - meansNoL * meansNoL.transpose();

  // Parameters to be fitted
  UncertainPerigeeCircle resultCircle;
  FloatType chi2;
  Matrix< FloatType, 3, 3> perigeeCovariance;


  size_t nObservationsWithDriftRadius = observations2D.getNObservationsWithDriftRadius();
  if (nObservationsWithDriftRadius > 0) {
    resultCircle = ::fit(s, isLineConstrained(), isOriginConstrained());
    chi2 = calcChi2(resultCircle, s);

    // Covariance calculation does not need the drift lengths, which is why we do not forward them
    perigeeCovariance = calcCovariance(resultCircle, sNoL, isLineConstrained(), isOriginConstrained());

  } else {
    resultCircle = ::fit(sNoL, isLineConstrained(), isOriginConstrained());
    //Alternative implementations for comparision
    // if (not isOriginConstrained()){
    //   resultCircle = fitSeperateOffset(meansNoL, cNoL, isLineConstrained());
    //}

    chi2 = calcChi2(resultCircle, sNoL);
    perigeeCovariance = calcCovariance(resultCircle, sNoL, isLineConstrained(), isOriginConstrained());
  }

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
