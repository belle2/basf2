/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCFacet.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/geometry/UncertainParameterLine2D.h>
#include <tracking/trackingUtilities/geometry/ParameterLine2D.h>
#include <tracking/trackingUtilities/geometry/Vector2D.h>

#include <tracking/trackingUtilities/numerics/EigenView.h>

#include <Eigen/Core>

#include <cmath>

#include <Math/Functor.h>
#include <Math/BrentMinimizer1D.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

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

  /**
   *  Intermediate results of the line fit sufficient to construct the fitted line lazily.
   *
   *  Keeping the construction of the line and its covariance matrix separate allows
   *  callers to compute only the chi2 and to build the line only when it is needed.
   */
  struct LineFitPrecursor {
    /// The forward direction to which the observations were rotated
    Vector2D coordinate;

    /// The weighted averages of the rotated observations
    Eigen::Array<double, 1, 3> averages;

    /// The weighted covariances of the rotated observations
    Eigen::Matrix<double, 3, 3> covariances;

    /// The fitted direction relative to the rotated coordinate system
    Eigen::Vector2d phiVec;

    /// The chi2 of the fit
    double chi2 = 0.0;

    /// The sum of weights of the observations
    double sumW = 0.0;
  };

  template<int N>
  LineFitPrecursor fitPrecursor(Matrix<double, N, 3> xylIn,
                                Matrix<double, N, 1> wIn,
                                int nSteps)
  {
    LineFitPrecursor precursor;

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

    precursor.coordinate = coordinate;
    precursor.averages = averages;
    precursor.covariances = covariances;
    precursor.phiVec = phiVec;
    precursor.chi2 = chi2;
    precursor.sumW = w.sum();
    return precursor;
  }

  /// Construct the fitted line from the precursor of a fit with N observations
  UncertainParameterLine2D lineFromPrecursor(const LineFitPrecursor& precursor, int ndf)
  {
    const Vector2D& coordinate = precursor.coordinate;
    const Eigen::Array<double, 1, 3>& averages = precursor.averages;
    const Eigen::Matrix<double, 3, 3>& covariances = precursor.covariances;
    const Eigen::Vector2d& phiVec = precursor.phiVec;
    const double chi2 = precursor.chi2;

    double meanArcLength = averages.topLeftCorner<1, 2>().matrix() * phiVec;
    double varArcLength = phiVec.transpose() * covariances.topLeftCorner<2, 2>() * phiVec;
    double p = precursor.sumW;

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
    return UncertainParameterLine2D(parameterLine2D, lineCovariance, chi2, ndf);
  }

  template<int N>
  UncertainParameterLine2D fit(Matrix<double, N, 3> xylIn,
                               Matrix<double, N, 1> wIn,
                               int nSteps)
  {
    LineFitPrecursor precursor = fitPrecursor(std::move(xylIn), std::move(wIn), nSteps);
    int ndf = N - 2;
    return lineFromPrecursor(precursor, ndf);
  }

}

namespace {
  /// Fill the observation and weight matrices for the fit of a single facet
  Vector2D fillFacetObservations(const CDCFacet& facet,
                                 Matrix<double, 3, 3>& xyl,
                                 Matrix<double, 3, 1>& w)
  {
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

    return support;
  }
}

double FacetFitter::fit(const CDCFacet& facet, int nSteps, double maxChi2)
{
  // Measurement matrix
  Matrix<double, 3, 3> xyl = Matrix<double, 3, 3>::Zero();

  // Weight matrix
  Matrix<double, 3, 1> w = Matrix<double, 3, 1>::Zero();

  const Vector2D support = fillFacetObservations(facet, xyl, w);

  LineFitPrecursor precursor = fitPrecursor(std::move(xyl), std::move(w), nSteps);

  // Construct and commit the fit line only when it is wanted: always for an
  // infinite maxChi2, otherwise only if the fit passes the cut. Skipping it for
  // failing facets avoids building the line and its covariance matrix. A NaN chi2
  // fails "chi2 <= maxChi2", so it is committed only in the unbounded case.
  if (std::isinf(maxChi2) or precursor.chi2 <= maxChi2) {
    const int ndf = 1;
    UncertainParameterLine2D fitLine = lineFromPrecursor(precursor, ndf);
    fitLine.passiveMoveBy(-support);
    facet.setFitLine(fitLine);
  }
  return precursor.chi2;
}


namespace {
  /// Fill the observation and weight matrices for the fit of a facet pair
  Vector2D fillFacetPairObservations(const CDCFacet& fromFacet,
                                     const CDCFacet& toFacet,
                                     Matrix<double, 6, 3>& xyl,
                                     Matrix<double, 6, 1>& w)
  {
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

    return support;
  }
}

UncertainParameterLine2D FacetFitter::fit(const CDCFacet& fromFacet,
                                          const CDCFacet& toFacet,
                                          int nSteps)
{
  // Observations matrix
  Matrix<double, 6, 3> xyl = Matrix<double, 6, 3>::Zero();

  // Weight matrix
  Matrix<double, 6, 1> w = Matrix<double, 6, 1>::Zero();

  const Vector2D support = fillFacetPairObservations(fromFacet, toFacet, xyl, w);

  UncertainParameterLine2D fitLine{ ::fit(std::move(xyl), std::move(w), nSteps) };
  fitLine.passiveMoveBy(-support);
  return fitLine;
}

double FacetFitter::fitChi2(const CDCFacet& fromFacet,
                            const CDCFacet& toFacet)
{
  // Observations matrix
  Matrix<double, 6, 3> xyl = Matrix<double, 6, 3>::Zero();

  // Weight matrix
  Matrix<double, 6, 1> w = Matrix<double, 6, 1>::Zero();

  fillFacetPairObservations(fromFacet, toFacet, xyl, w);

  // The chi2 is invariant against the translation by the support point
  constexpr const int nSteps = 0;
  return fitPrecursor(std::move(xyl), std::move(w), nSteps).chi2;
}


UncertainParameterLine2D FacetFitter::fit(TrackingUtilities::Matrix<double, 3, 3> xyl,
                                          TrackingUtilities::Matrix<double, 3, 1> w,
                                          int nSteps)
{
  return ::fit(std::move(xyl), std::move(w), nSteps);
}
