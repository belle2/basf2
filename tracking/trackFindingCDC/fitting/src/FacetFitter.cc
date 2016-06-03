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

#include <Eigen/Dense>

#include <Math/Functor.h>
#include <Math/BrentMinimizer1D.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace Eigen;

double FacetFitter::fit(const CDCFacet& facet, bool singleStep)
{
  // Measurement matrix
  Matrix< double, 3, 3> xyl;

  // Weight matrix
  Array< double, 3, 1> w;

  const CDCRLWireHit& startRLWireHit = facet.getStartRLWireHit();
  const CDCRLWireHit& middleRLWireHit = facet.getMiddleRLWireHit();
  const CDCRLWireHit& endRLWireHit = facet.getEndRLWireHit();

  const Vector2D support = middleRLWireHit.getWire().getRefPos2D();

  const double startDriftLengthVar = startRLWireHit.getRefDriftLengthVariance();
  const Vector2D startWirePos2D = startRLWireHit.getWire().getRefPos2D();
  xyl(0, 0) = startWirePos2D.x() - support.x();
  xyl(0, 1) = startWirePos2D.y() - support.y();
  xyl(0, 2) = startRLWireHit.getSignedRefDriftLength();
  w(0) = 1.0 / startDriftLengthVar;

  const double middleDriftLengthVar = middleRLWireHit.getRefDriftLengthVariance();
  const Vector2D middleWirePos2D = middleRLWireHit.getWire().getRefPos2D();
  xyl(1, 0) = middleWirePos2D.x() - support.x();
  xyl(1, 1) = middleWirePos2D.y() - support.y();
  xyl(1, 2) = middleRLWireHit.getSignedRefDriftLength();
  w(1) = 1.0 / middleDriftLengthVar;

  const double endDriftLengthVar = endRLWireHit.getRefDriftLengthVariance();
  const Vector2D endWirePos2D = endRLWireHit.getWire().getRefPos2D();
  xyl(2, 0) = endWirePos2D.x() - support.x();
  xyl(2, 1) = endWirePos2D.y() - support.y();
  xyl(2, 2) = endRLWireHit.getSignedRefDriftLength();
  w(2) = 1.0 / endDriftLengthVar;

  double chi2 = 0;
  ParameterLine2D fitLine{ fit(std::move(xyl), std::move(w), chi2, singleStep) };
  fitLine.passiveMoveBy(-support);

  Vector2D startClosest = fitLine.closest(startWirePos2D);
  Vector2D middleClosest = fitLine.closest(middleWirePos2D);
  Vector2D endClosest = fitLine.closest(endWirePos2D);

  facet.setStartToMiddleLine(ParameterLine2D::throughPoints(startClosest, middleClosest));
  facet.setStartToEndLine(ParameterLine2D::throughPoints(startClosest, endClosest));
  facet.setMiddleToEndLine(ParameterLine2D::throughPoints(middleClosest, endClosest));

  return chi2;
}

namespace {

  Vector2D getCenterForwardDirection(const Matrix<double, 3, 3>& xyl)
  {
    /// Rotate in forward direction
    Vector2D coordinate(xyl(2, 0) - xyl(0, 0), xyl(2, 1) - xyl(0, 1));
    return coordinate.unit();
  }

  Vector2D getTangentialForwardDirection(const Matrix<double, 3, 3>& xyl)
  {
    /// Rotate in forward direction
    Vector2D from(xyl(0, 0), xyl(0, 1));
    Vector2D to(xyl(2, 0), xyl(2, 1));
    ParameterLine2D tangentLine = ParameterLine2D::touchingCircles(from, xyl(0, 2), to, xyl(2, 2));
    Vector2D coordinate = tangentLine.tangential();
    return coordinate.unit();
  }

  void rotate(Vector2D coordinate,  Matrix<double, 3, 3>& xyl)
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

  double fitPhiOneStep(const Matrix<double, 3, 3>& xylCov, double& chi2)
  {
    const double phi = (xylCov(0, 1) + xylCov(0, 2)) / xylCov(0, 0);
    chi2 = (xylCov(1, 1) + 2 * xylCov(1, 2) + xylCov(2, 2) - phi * (xylCov(0, 1) + xylCov(0, 2)));
    return phi;
  }

  double fitPhiBrent(const Matrix<double, 3, 3>& xylCov, int nIter, double& chi2)
  {
    const Matrix< double, 2, 2> A = xylCov.topLeftCorner<2, 2>();
    const Matrix< double, 2, 1> b = xylCov.topRightCorner<2, 1>();
    const double c = xylCov(2, 2);

    auto calcReducedChi2 = [&A, &b](double phi) -> double {
      Matrix<double, 2, 1> normal(std::sin(phi), -std::cos(phi));
      double chi2 = ((normal.transpose() * A - 2 * b.transpose()) * normal)[0];
      return chi2;
    };

    ROOT::Math::Functor1D functor(calcReducedChi2);
    ROOT::Math::BrentMinimizer1D bm;
    bm.SetFunction(functor, -M_PI / 2, M_PI / 2);
    bm.Minimize(nIter); // #iterations, abs. error, rel. error

    chi2 = bm.FValMinimum() + c;
    const double phi = bm.XMinimum();
    return phi;
  }

}

Line2D FacetFitter::fit(Matrix<double, 3, 3> xyl,
                        Array<double, 3, 1> w,
                        double& chi2,
                        bool singleStep)
{
  /// Rotate in forward direction
  Vector2D coordinate = getTangentialForwardDirection(xyl);
  // Sometimes the calculation of the tangent fails due to misestimated dirft lengths
  // Make best effort the continue the calculation
  if (coordinate.hasNAN()) {
    coordinate = getCenterForwardDirection(xyl);
  }

  rotate(coordinate, xyl);

  Array< double, 1, 3> averages = (xyl.array().colwise() * w).colwise().sum() / w.sum();
  Matrix< double, 3, 3> deltas = xyl.array().rowwise() - averages;
  Matrix< double, 3, 3> weightedDeltas = deltas.array().colwise() * w;
  Matrix< double, 3, 3> covariances = deltas.transpose() * weightedDeltas / w.sum();

  const int nIter = 100;

  double phi = 0.0;
  if (singleStep) {
    phi = fitPhiOneStep(covariances, chi2);
  } else {
    phi = fitPhiBrent(covariances, nIter, chi2);
  }
  chi2 *= w.sum();

  const Vector2D tangential = Vector2D::Phi(phi);

  Vector2D n12 = tangential.orthogonal(ERotation::c_Clockwise);
  double n0 = averages(2) - averages(0) * n12.x() - averages(1) * n12.y();

  // Transform the normal vector back into the original coordinate system.
  unrotate(coordinate, n12);

  Line2D fitLine(n0, n12);
  return  fitLine;
}
