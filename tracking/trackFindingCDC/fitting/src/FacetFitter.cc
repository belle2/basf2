/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EigenView.h>

#include <Eigen/Core>

#include <Math/Functor.h>
#include <Math/BrentMinimizer1D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  template<int N>
  Vector2D getCenterForwardDirection(const Matrix<double, N, 3>& xyl)
  {
    /// Rotate in forward direction
    Vector2D coordinate(xyl(N - 1, 0) - xyl(0, 0), xyl(N - 1, 1) - xyl(0, 1));
    return coordinate.unit();
  }

  template<int N>
  Vector2D getTangentialForwardDirection(const Matrix<double, N, 3>& xyl)
  {
    /// Rotate in forward direction
    Vector2D fromPos(xyl(0, 0), xyl(0, 1));
    double fromL = xyl(0, 2);

    Vector2D toPos(xyl(N - 1, 0), xyl(N - 1, 1));
    double toL = xyl(N - 1, 2);

    ParameterLine2D tangentLine = ParameterLine2D::touchingCircles(fromPos, fromL, toPos, toL);
    Vector2D coordinate = tangentLine.tangential();
    return coordinate.unit();
  }

  template<int N>
  void rotate(Vector2D coordinate, Matrix<double, N, 3>& xyl)
  {
    Matrix<double, 3, 3> rot = Matrix<double, 3, 3>::Identity();
    rot(0, 0) = coordinate.x();
    rot(0, 1) = -coordinate.y();
    rot(1, 0) = coordinate.y();
    rot(1, 1) = coordinate.x();
    rot(2, 2) = 1; // Drift length remains the same.
    xyl =  xyl * rot;
  }

  void unrotate(Vector2D coordinate, Vector2D& vec)
  {
    // Inverse rotation is accomblished by taking the angle to the opposite
    // which is equivalent to flipping the second coordinate.
    vec = vec.passiveRotatedBy(coordinate.flippedSecond());
  }

  Eigen::Vector2d fitPhiVecZeroSteps(const Eigen::Matrix<double, 3, 3>& xylCov, double& chi2)
  {
    chi2 = xylCov(1, 1) + 2 * xylCov(1, 2) + xylCov(2, 2);
    return Eigen::Vector2d(1, 0);
  }

  Eigen::Vector2d fitPhiVecOneStep(const Eigen::Matrix<double, 3, 3>& xylCov, double& chi2)
  {
    const double phi = (xylCov(0, 1) + xylCov(0, 2)) / xylCov(0, 0);
    chi2 = xylCov(1, 1) + 2 * xylCov(1, 2) + xylCov(2, 2) - phi * (xylCov(0, 1) + xylCov(0, 2));
    return Eigen::Vector2d(std::cos(phi), std::sin(phi));
  }

  Eigen::Vector2d fitPhiVecBrent(const Eigen::Matrix<double, 3, 3>& xylCov, int nIter, double& chi2)
  {
    const Eigen::Matrix< double, 2, 2> A = xylCov.topLeftCorner<2, 2>();
    const Eigen::Matrix< double, 2, 1> b = xylCov.topRightCorner<2, 1>();
    const double c = xylCov(2, 2);

    auto calcReducedChi2 = [&A, &b](double phi) -> double {
      Eigen::Matrix<double, 2, 1> normal(std::sin(phi), -std::cos(phi));
      return ((normal.transpose() * A - 2 * b.transpose()) * normal)[0];
    };

    ROOT::Math::Functor1D functor(calcReducedChi2);
    ROOT::Math::BrentMinimizer1D bm;
    bm.SetFunction(functor, -M_PI / 2, M_PI / 2);
    bm.Minimize(nIter); // #iterations, abs. error, rel. error

    chi2 = bm.FValMinimum() + c;
    const double phi = bm.XMinimum();
    return Eigen::Vector2d(std::cos(phi), std::sin(phi));
  }

  template<int N>
  UncertainParameterLine2D fit(Matrix<double, N, 3> xylIn,
                               Matrix<double, N, 1> wIn,
                               int nSteps)
  {
    /// Rotate in forward direction
    Vector2D coordinate = getTangentialForwardDirection(xylIn);
    // Sometimes the calculation of the tangent fails due to misestimated dirft lengths
    // Make best effort the continue the calculation
    if (coordinate.hasNAN()) {
      coordinate = getCenterForwardDirection(xylIn);
    }

    rotate(coordinate, xylIn);

    auto xyl = mapToEigen(xylIn);
    auto w = mapToEigen(wIn).array();

    Eigen::Array< double, 1, 3> averages = (xyl.array().colwise() * w).colwise().sum() / w.sum();
    Eigen::Matrix< double, N, 3> deltas = xyl.array().rowwise() - averages;
    Eigen::Matrix< double, N, 3> weightedDeltas = deltas.array().colwise() * w;
    Eigen::Matrix< double, 3, 3> covariances = deltas.transpose() * weightedDeltas / w.sum();

    Eigen::Vector2d phiVec;
    double chi2 = 0.0;
    if (nSteps == 0) {
      phiVec = fitPhiVecZeroSteps(covariances, chi2);
    } else if (nSteps == 1) {
      phiVec = fitPhiVecOneStep(covariances, chi2);
    } else {
      phiVec = fitPhiVecBrent(covariances, nSteps, chi2);
    }
    chi2 *= w.sum();

    double meanArcLength = averages.topLeftCorner<1, 2>().matrix() * phiVec;
    double varArcLength = phiVec.transpose() * covariances.topLeftCorner<2, 2>() * phiVec;
    double p = w.sum();

    using namespace NLineParameterIndices;
    LinePrecision linePrecision;
    linePrecision(c_Phi0, c_Phi0) = p * (varArcLength + meanArcLength * meanArcLength);
    linePrecision(c_Phi0, c_I) = p * meanArcLength;
    linePrecision(c_I, c_Phi0) = p * meanArcLength;
    linePrecision(c_I, c_I) = p;
    LineCovariance lineCovariance = LineUtil::covarianceFromFullPrecision(linePrecision);

    Vector2D tangential(phiVec(0), phiVec(1));
    Vector2D n12 = tangential.orthogonal(ERotation::c_Clockwise);
    double n0 = averages(2) - averages(0) * n12.x() - averages(1) * n12.y();
    Vector2D support = -n12 * n0;

    // Transform the normal vector back into the original coordinate system.
    unrotate(coordinate, support);
    unrotate(coordinate, tangential);

    ParameterLine2D parameterLine2D(support, tangential);
    int ndf = N - 2;
    return UncertainParameterLine2D(parameterLine2D, lineCovariance, chi2, ndf);
  }

}

double FacetFitter::fit(const CDCFacet& facet, int nSteps)
{
  // Measurement matrix
  Matrix<double, 3, 3> xyl = Matrix<double, 3, 3>::Zero();

  // Weight matrix
  Matrix<double, 3, 1> w = Matrix<double, 3, 1>::Zero();

  const CDCRLWireHit& startRLWireHit = facet.getStartRLWireHit();
  const CDCRLWireHit& middleRLWireHit = facet.getMiddleRLWireHit();
  const CDCRLWireHit& endRLWireHit = facet.getEndRLWireHit();

  const Vector2D support = middleRLWireHit.getWireHit().getRefPos2D();

  const double startDriftLengthVar = startRLWireHit.getRefDriftLengthVariance();
  const Vector2D startWirePos2D = startRLWireHit.getWireHit().getRefPos2D();
  xyl(0, 0) = startWirePos2D.x() - support.x();
  xyl(0, 1) = startWirePos2D.y() - support.y();
  xyl(0, 2) = startRLWireHit.getSignedRefDriftLength();
  w(0) = 1.0 / startDriftLengthVar;

  const double middleDriftLengthVar = middleRLWireHit.getRefDriftLengthVariance();
  const Vector2D middleWirePos2D = middleRLWireHit.getWireHit().getRefPos2D();
  xyl(1, 0) = middleWirePos2D.x() - support.x();
  xyl(1, 1) = middleWirePos2D.y() - support.y();
  xyl(1, 2) = middleRLWireHit.getSignedRefDriftLength();
  w(1) = 1.0 / middleDriftLengthVar;

  const double endDriftLengthVar = endRLWireHit.getRefDriftLengthVariance();
  const Vector2D endWirePos2D = endRLWireHit.getWireHit().getRefPos2D();
  xyl(2, 0) = endWirePos2D.x() - support.x();
  xyl(2, 1) = endWirePos2D.y() - support.y();
  xyl(2, 2) = endRLWireHit.getSignedRefDriftLength();
  w(2) = 1.0 / endDriftLengthVar;

  UncertainParameterLine2D fitLine{ ::fit(std::move(xyl), std::move(w), nSteps) };
  fitLine.passiveMoveBy(-support);
  facet.setFitLine(fitLine);
  return fitLine.chi2();
}


UncertainParameterLine2D FacetFitter::fit(const CDCFacet& fromFacet,
                                          const CDCFacet& toFacet,
                                          int nSteps)
{
  // Observations matrix
  Matrix<double, 6, 3> xyl = Matrix<double, 6, 3>::Zero();

  // Weight matrix
  Matrix<double, 6, 1> w = Matrix<double, 6, 1>::Zero();

  const Vector2D support = Vector2D::average(fromFacet.getMiddleWireHit().getRefPos2D(),
                                             toFacet.getMiddleWireHit().getRefPos2D());
  {
    const CDCRLWireHit& startRLWireHit = fromFacet.getStartRLWireHit();
    const CDCRLWireHit& middleRLWireHit = fromFacet.getMiddleRLWireHit();
    const CDCRLWireHit& endRLWireHit = fromFacet.getEndRLWireHit();

    const double startDriftLengthVar = startRLWireHit.getRefDriftLengthVariance();
    const Vector2D startWirePos2D = startRLWireHit.getWireHit().getRefPos2D();
    xyl(0, 0) = startWirePos2D.x() - support.x();
    xyl(0, 1) = startWirePos2D.y() - support.y();
    xyl(0, 2) = startRLWireHit.getSignedRefDriftLength();
    w(0) = 1.0 / startDriftLengthVar;

    const double middleDriftLengthVar = middleRLWireHit.getRefDriftLengthVariance();
    const Vector2D middleWirePos2D = middleRLWireHit.getWireHit().getRefPos2D();
    xyl(1, 0) = middleWirePos2D.x() - support.x();
    xyl(1, 1) = middleWirePos2D.y() - support.y();
    xyl(1, 2) = middleRLWireHit.getSignedRefDriftLength();
    w(1) = 1.0 / middleDriftLengthVar;

    const double endDriftLengthVar = endRLWireHit.getRefDriftLengthVariance();
    const Vector2D endWirePos2D = endRLWireHit.getWireHit().getRefPos2D();
    xyl(2, 0) = endWirePos2D.x() - support.x();
    xyl(2, 1) = endWirePos2D.y() - support.y();
    xyl(2, 2) = endRLWireHit.getSignedRefDriftLength();
    w(2) = 1.0 / endDriftLengthVar;
  }

  {
    const CDCRLWireHit& startRLWireHit = toFacet.getStartRLWireHit();
    const CDCRLWireHit& middleRLWireHit = toFacet.getMiddleRLWireHit();
    const CDCRLWireHit& endRLWireHit = toFacet.getEndRLWireHit();

    const double startDriftLengthVar = startRLWireHit.getRefDriftLengthVariance();
    const Vector2D startWirePos2D = startRLWireHit.getWireHit().getRefPos2D();
    xyl(3, 0) = startWirePos2D.x() - support.x();
    xyl(3, 1) = startWirePos2D.y() - support.y();
    xyl(3, 2) = startRLWireHit.getSignedRefDriftLength();
    w(3) = 1.0 / startDriftLengthVar;

    const double middleDriftLengthVar = middleRLWireHit.getRefDriftLengthVariance();
    const Vector2D middleWirePos2D = middleRLWireHit.getWireHit().getRefPos2D();
    xyl(4, 0) = middleWirePos2D.x() - support.x();
    xyl(4, 1) = middleWirePos2D.y() - support.y();
    xyl(4, 2) = middleRLWireHit.getSignedRefDriftLength();
    w(4) = 1.0 / middleDriftLengthVar;

    const double endDriftLengthVar = endRLWireHit.getRefDriftLengthVariance();
    const Vector2D endWirePos2D = endRLWireHit.getWireHit().getRefPos2D();
    xyl(5, 0) = endWirePos2D.x() - support.x();
    xyl(5, 1) = endWirePos2D.y() - support.y();
    xyl(5, 2) = endRLWireHit.getSignedRefDriftLength();
    w(5) = 1.0 / endDriftLengthVar;
  }

  UncertainParameterLine2D fitLine{ ::fit(std::move(xyl), std::move(w), nSteps) };
  fitLine.passiveMoveBy(-support);
  return fitLine;
}


UncertainParameterLine2D FacetFitter::fit(Matrix<double, 3, 3> xyl,
                                          Matrix<double, 3, 1> w,
                                          int nSteps)
{
  return ::fit(std::move(xyl), std::move(w), nSteps);
}
