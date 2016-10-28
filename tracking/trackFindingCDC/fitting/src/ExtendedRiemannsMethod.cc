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

  double totalPerps = perigeeCircle->arcLengthBetween(frontPos, backPos);
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


  PerigeePrecision calcPrecision(const PerigeeCircle& parameters,
                                 const Matrix< double, 4, 4 >& s,
                                 bool lineConstrained = false,
                                 bool originConstrained = false)
  {
    const double impact = parameters.impact();
    const Vector2D& phi0Vec = parameters.tangential();
    const double curvature = parameters.curvature();

    using namespace NPerigeeParameterIndices;
    Matrix<double, 4, 3> ambiguity = Matrix<double, 4, 3> ::Zero();

    ambiguity(0, c_Curv) = impact * impact / 2;
    ambiguity(1, c_Curv) = phi0Vec.y() * impact;
    ambiguity(2, c_Curv) = -phi0Vec.x() * impact;
    ambiguity(3, c_Curv) = 1.0 / 2.0;

    ambiguity(0, c_Phi0) = 0;
    ambiguity(1, c_Phi0) = phi0Vec.x() * (1 + curvature * impact);
    ambiguity(2, c_Phi0) = phi0Vec.y() * (1 + curvature * impact);
    ambiguity(3, c_Phi0) = 0;

    ambiguity(0, c_I) = 1 + curvature * impact;
    ambiguity(1, c_I) = phi0Vec.y() * curvature;
    ambiguity(2, c_I) = -phi0Vec.x() * curvature;
    ambiguity(3, c_I) = 0;

    PerigeePrecision perigeePrecision = ambiguity.transpose() * s * ambiguity;
    // Zero out the unfitted parameters from the precision matrix
    if (lineConstrained) {
      perigeePrecision.row(c_Curv) = Matrix<double, 1, 3>::Zero();
      perigeePrecision.col(c_Curv) = Matrix<double, 3, 1>::Zero();
    }

    if (originConstrained) {
      perigeePrecision.row(c_I) = Matrix<double, 1, 3>::Zero();
      perigeePrecision.col(c_I) = Matrix<double, 3, 1>::Zero();
    }

    return perigeePrecision;
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

  size_t nObservationsWithDriftRadius = observations2D.getNObservationsWithDriftRadius();
  if (nObservationsWithDriftRadius > 0) {
    resultCircle = ::fit(s, isLineConstrained(), isOriginConstrained());
    chi2 = calcChi2(resultCircle, s);
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

  }

  // Covariance calculation does not need the drift lengths, which is why we do not forward them
  PerigeePrecision perigeePrecision = calcPrecision(resultCircle, sNoL,
                                                    isLineConstrained(),
                                                    isOriginConstrained());

  // Use in pivotingin caset the matrix is not full rank as is for the constrained cases-
  PerigeeCovariance perigeeCovariance = perigeePrecision.colPivHouseholderQr().inverse();

  resultCircle.setNDF(ndf);
  resultCircle.setChi2(chi2);
  resultCircle.setPerigeeCovariance(perigeeCovariance);
  return resultCircle;
}
