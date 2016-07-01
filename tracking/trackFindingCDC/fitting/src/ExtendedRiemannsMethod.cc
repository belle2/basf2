/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/ExtendedRiemannsMethod.h>

#include <Eigen/Dense>

using namespace std;
using namespace Belle2::TrackFindingCDC;
using namespace Eigen;



ExtendedRiemannsMethod::ExtendedRiemannsMethod() :
  m_lineConstrained(false),
  m_originConstrained(false)
{
}



void ExtendedRiemannsMethod::update(CDCTrajectory2D& trajectory2D,
                                    CDCObservations2D& observations2D) const
{
  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;

  Vector2D origin = Vector2D(0.0, 0.0);
  Vector2D centralPoint = observations2D.getCentralPoint();

  const Vector2D& ref = isOriginConstrained() ? origin : centralPoint;
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

  trajectory2D = CDCTrajectory2D(ref, perigeeCircle);
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
  constexpr size_t iPhi0 = 1;
  constexpr size_t iI = 2;



  /// Variant with drift circles
  PerigeeCircle fit(const Matrix< double, 5, 5 >& sumMatrix,
                    bool lineConstrained = false,
                    bool originConstrained = false)
  {
    // Solve the normal equation X * n = y
    if (lineConstrained) {
      if (originConstrained) {
        Matrix< double, 2, 2> X = sumMatrix.block<2, 2>(1, 1);
        Matrix< double, 2, 1> y = sumMatrix.block<2, 1>(1, iL);
        Matrix< double, 2, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(0.0, n(0), n(1), 0.0);

      } else {
        Matrix< double, 3, 3> X = sumMatrix.block<3, 3>(0, 0);
        Matrix< double, 3, 1> y = sumMatrix.block<3, 1>(0, iL);
        Matrix< double, 3, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), 0.0);

      }

    } else {
      if (originConstrained) {
        Matrix< double, 3, 3> X = sumMatrix.block<3, 3>(1, 1);
        Matrix< double, 3, 1> y = sumMatrix.block<3, 1>(1, iL);
        Matrix< double, 3, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(0.0, n(0), n(1), n(2));

      } else {
        Matrix< double, 4, 4> X = sumMatrix.block<4, 4>(0, 0);
        Matrix< double, 4, 1> y = sumMatrix.block<4, 1>(0, iL);
        Matrix< double, 4, 1> n = X.ldlt().solve(y);
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

      }
    }
  }


  /// Variant without drift circles
  PerigeeCircle fit(Matrix< double, 4, 4 > sumMatrix,
                    bool lineConstrained = false,
                    bool originConstrained = false)
  {
    // Solve the normal equation min_n  n^T * X * n
    // n is the smallest eigenvector

    if (lineConstrained) {
      if (originConstrained) {
        Matrix< double, 2, 2> X = sumMatrix.block<2, 2>(1, 1);
        SelfAdjointEigenSolver< Matrix<double, 2, 2> > eigensolver(X);
        Matrix<double, 2, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(0.0, n(0), n(1), 0.0);

      } else {
        Matrix< double, 3, 3> X = sumMatrix.block<3, 3>(0, 0);
        SelfAdjointEigenSolver< Matrix<double, 3, 3> > eigensolver(X);
        Matrix<double, 3, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), 0.0);

      }
    } else {
      if (originConstrained) {
        Matrix< double, 3, 3> X = sumMatrix.block<3, 3>(1, 1);
        SelfAdjointEigenSolver< Matrix<double, 3, 3> > eigensolver(X);
        Matrix<double, 3, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }
        return PerigeeCircle::fromN(0.0, n(0), n(1), n(2));

      } else {
        Matrix< double, 4, 4 > X = sumMatrix.block<4, 4>(0, 0);
        SelfAdjointEigenSolver< Matrix<double, 4, 4> > eigensolver(X);
        Matrix<double, 4, 1> n = eigensolver.eigenvectors().col(0);
        if (eigensolver.info() != Success) {
          B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
        }

        return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));
      }
    }
  }

  // Declare function as currently unused to avoid compiler warning
  PerigeeCircle fitSeperateOffset(Matrix< double, 4, 1 > means,
                                  Matrix< double, 4, 4 > c,
                                  bool lineConstrained) __attribute__((__unused__));

  /// Variant without drift circles and seperating the offset before the matrix solving
  PerigeeCircle fitSeperateOffset(Matrix< double, 4, 1 > means,
                                  Matrix< double, 4, 4 > c,
                                  bool lineConstrained = false)
  {
    // Solve the normal equation min_n  n^T * c * n
    // for the plane normal and move the plain by the offset
    // n is the smallest eigenvector

    if (lineConstrained) {
      Matrix< double, 2, 2> X = c.block<2, 2>(1, 1);
      SelfAdjointEigenSolver< Matrix<double, 2, 2> > eigensolver(X);
      if (eigensolver.info() != Success) {
        B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
      }

      //the eigenvalues are generated in increasing order
      //we are interested in the lowest one since we want to compute the normal vector of the plane
      Matrix<double, 4, 1> n;
      n.middleRows<2>(iX) = eigensolver.eigenvectors().col(0);
      n(iW) = -means.middleRows<2>(iX).transpose() * n.middleRows<2>(iX);
      n(iR2) = 0.;
      return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

    } else {
      Matrix< double, 3, 3> X = c.block<3, 3>(1, 1);
      SelfAdjointEigenSolver< Matrix<double, 3, 3> > eigensolver(X);
      if (eigensolver.info() != Success) {
        B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
      }

      //the eigenvalues are generated in increasing order
      //we are interested in the lowest one since we want to compute the normal vector of the plane
      Matrix<double, 4, 1> n;
      n.middleRows<3>(iX) = eigensolver.eigenvectors().col(0);
      n(iW) = -means.middleRows<3>(iX).transpose() * n.middleRows<3>(iX);
      return PerigeeCircle::fromN(n(iW), n(iX), n(iY), n(iR2));

    }

  }


  /// Variant with drift circles
  double calcChi2(const PerigeeCircle& parameters,
                  const Matrix< double, 5, 5 >& s)
  {

    Matrix<double, 5, 1> n;
    n(0) = parameters.n0();
    n(1) = parameters.n1();
    n(2) = parameters.n2();
    n(3) = parameters.n3();
    n(4) = -1;

    double chi2 = n.transpose() * s * n;
    return chi2;

  }


  /// Variant without drift circles
  double calcChi2(const PerigeeCircle& parameters,
                  const Matrix< double, 4, 4 >& s)
  {

    Matrix<double, 4, 1> n;
    n(0) = parameters.n0();
    n(1) = parameters.n1();
    n(2) = parameters.n2();
    n(3) = parameters.n3();

    double chi2 = n.transpose() * s * n;

    return chi2;
  }


  Matrix<double, 3, 3> calcCovariance(const PerigeeCircle& parameters,
                                      const Matrix< double, 4, 4 >& s,
                                      bool lineConstrained = false,
                                      bool originConstrained = false)
  {
    const double n0 = parameters.n0();

    Vector2D n12 = parameters.n12();
    const double n3 = parameters.n3();

    // 1. Passive rotation such that n12 = (n1, 0);

    // n12 in the rotated system will be:
    const double rotN1 = n12.normalize();
    const double rotN2 = 0.0;

    // Setup passive rotation matrix.
    Matrix< double, 4, 4 > rot = Matrix< double, 4, 4 >::Identity();
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
    Matrix< double, 3, 4 > reduce = Matrix< double, 3, 4 >::Zero();
    reduce(iReducedN0, iN0) = 1.;
    reduce(iReducedN2, iN2) = 1.;
    reduce(iReducedN3, iN3) = 1.;

    reduce(iReducedN0, iN1) = 2 * n3 / rotN1;
    reduce(iReducedN2, iN1) = -rotN2 / rotN1;
    reduce(iReducedN3, iN1) = 2 * n0 / rotN1;


    // Instead of doing the two transformations seperatly we combine the matrices and apply a single transformation.
    // This saves one matrix multiplication.
    // Matrix<double, 4, 4> rotS = rot * s * rot.transpose();
    // Matrix<double, 3, 3> reducedNInvV = reduce * rotS * reduce.transpose();

    Matrix< double, 3, 4 > rotAndReduce = reduce * rot;
    Matrix< double, 3, 3> reducedNInvCov = rotAndReduce * s * rotAndReduce.transpose();

    //Zero out the unconsidered parameters before inversion. Keep one on the diagonal.
    if (lineConstrained) {
      reducedNInvCov.row(iReducedN3) = Matrix<double, 1, 3>::Zero();
      reducedNInvCov.col(iReducedN3) = Matrix<double, 3, 1>::Zero();
      reducedNInvCov(iReducedN3, iReducedN3) = 1.;
    }

    if (originConstrained) {
      reducedNInvCov.row(iReducedN0) = Matrix<double, 1, 3>::Zero();
      reducedNInvCov.col(iReducedN0) = Matrix<double, 3, 1>::Zero();
      reducedNInvCov(iReducedN0, iReducedN0) = 1.;
    }

    Matrix< double, 3, 3> reducedNCov = reducedNInvCov.inverse();

    if (lineConstrained) {
      reducedNCov(iReducedN3, iReducedN3) = 0.;
    }

    if (originConstrained) {
      reducedNCov(iReducedN0, iReducedN0) = 0.;
    }

    // 3. Translate to perigee
    Matrix< double, 3, 3 > perigeeJ;

    double normN12 = fabs(rotN1); // = hypot(n1, n2);
    double denominator = (1 + normN12) * (1 + normN12) * normN12;
    perigeeJ(iCurv, iReducedN0) = 0;
    perigeeJ(iCurv, iReducedN2) = 0;
    perigeeJ(iCurv, iReducedN3) = 2;

    perigeeJ(iPhi0, iReducedN0) = 0;
    perigeeJ(iPhi0, iReducedN2) = -1 / normN12;
    perigeeJ(iPhi0, iReducedN3) = 0;

    perigeeJ(iI, iReducedN0) = 2 * (normN12 * (1 + 2 * normN12) - 1) / denominator;
    perigeeJ(iI, iReducedN2) = 0;
    perigeeJ(iI, iReducedN3) =  -4 * n0 * n0 /  denominator;

    Matrix< double, 3, 3 > perigeeCov = perigeeJ * reducedNCov * perigeeJ.transpose();

    return perigeeCov;

  }

} // end anonymuous namespace



UncertainPerigeeCircle ExtendedRiemannsMethod::fitInternal(CDCObservations2D& observations2D) const
{
  // Matrix of weighted sums
  Matrix< double, 5, 5 > s = observations2D.getWXYRLSumMatrix();

  // The same as above without drift lengths
  Matrix<double, 4, 4> sNoL = s.block<4, 4>(0, 0);

  // Determine NDF : Circle fit eats up to 3 degrees of freedom debpending on the constraints
  size_t ndf = observations2D.size() - 1;

  if (not isOriginConstrained()) {
    --ndf;
  }

  if (not isLineConstrained()) {
    --ndf;
  }

  // Parameters to be fitted
  UncertainPerigeeCircle resultCircle;
  double chi2;
  Matrix< double, 3, 3> cov3;

  size_t nObservationsWithDriftRadius = observations2D.getNObservationsWithDriftRadius();
  if (nObservationsWithDriftRadius > 0) {
    resultCircle = ::fit(s, isLineConstrained(), isOriginConstrained());
    chi2 = calcChi2(resultCircle, s);

    // Covariance calculation does not need the drift lengths, which is why we do not forward them
    cov3 = calcCovariance(resultCircle, sNoL, isLineConstrained(), isOriginConstrained());

  } else {
    if (not isOriginConstrained()) {
      // Alternative implementation for comparision
      // Matrix of averages
      Matrix< double, 4, 4> aNoL = sNoL / sNoL(iW);
      // Measurement means
      Matrix< double, 4, 1> meansNoL = aNoL.row(iW);
      // Covariance matrix
      Matrix< double, 4, 4> cNoL = aNoL - meansNoL * meansNoL.transpose();
      resultCircle = fitSeperateOffset(meansNoL, cNoL, isLineConstrained());

    } else {
      resultCircle = ::fit(sNoL, isLineConstrained(), isOriginConstrained());
    }

    chi2 = calcChi2(resultCircle, sNoL);
    cov3 = calcCovariance(resultCircle, sNoL, isLineConstrained(), isOriginConstrained());
  }

  resultCircle.setChi2(chi2);
  resultCircle.setNDF(ndf);

  PerigeeCovariance perigeeCovariance;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      perigeeCovariance(i, j) = cov3(i, j);
    }
  }

  resultCircle.setPerigeeCovariance(perigeeCovariance);
  return resultCircle;
}
