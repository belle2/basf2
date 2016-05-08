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

double FacetFitter::fit(const CDCFacet& facet)
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
  ParameterLine2D fitLine(fit(xyl, w, chi2));
  fitLine.passiveMoveBy(-support);

  Vector2D startClosest = fitLine.closest(startWirePos2D);
  Vector2D middleClosest = fitLine.closest(middleWirePos2D);
  Vector2D endClosest = fitLine.closest(endWirePos2D);

  facet.setStartToMiddleLine(ParameterLine2D::throughPoints(startClosest, middleClosest));
  facet.setStartToEndLine(ParameterLine2D::throughPoints(startClosest, endClosest));
  facet.setMiddleToEndLine(ParameterLine2D::throughPoints(middleClosest, endClosest));

  return chi2;
}


Line2D FacetFitter::fit(Matrix<double, 3, 3> xyl,
                        Array<double, 3, 1> w,
                        double& chi2)
{
  /// Rotate in forward direction
  Vector2D coordinate(xyl(2, 0) - xyl(0, 0), xyl(2, 1) - xyl(0, 1));
  coordinate.normalize();

  Matrix<double, 3, 3> rot = Matrix<double, 3, 3>::Identity();
  rot(0, 0) = coordinate.x();
  rot(0, 1) = -coordinate.y();
  rot(1, 0) = coordinate.y();
  rot(1, 1) = coordinate.x();
  rot(2, 2) = 1; // Drift length remains the same.
  xyl =  xyl * rot;

  Array< double, 1, 3> averages = (xyl.array().colwise() * w).colwise().sum() / w.sum();
  Matrix< double, 3, 3> deltas = xyl.array().rowwise() - averages;
  Matrix< double, 3, 3> weightedDeltas = deltas.array().colwise() * w;
  Matrix< double, 3, 3> covariances = deltas.transpose() * weightedDeltas / w.sum();

  const Matrix< double, 2, 2> A = covariances.topLeftCorner<2, 2>();
  const Matrix< double, 2, 1> b = covariances.topRightCorner<2, 1>();
  const double c = covariances(2, 2);

  auto calcReducedChi2 = [&A, &b](double phi) -> double {
    Matrix<double, 2, 1> normal(std::sin(phi), -std::cos(phi));
    double chi2 = ((normal.transpose() * A - 2 * b.transpose()) * normal)[0];
    return chi2;
  };

  ROOT::Math::Functor1D functor(calcReducedChi2);
  ROOT::Math::BrentMinimizer1D bm;
  bm.SetFunction(functor, -M_PI / 2, M_PI / 2);
  bm.Minimize(100); // #iterations, abs. error, rel. error

  chi2 = (bm.FValMinimum() + c) * w.sum();
  const double phi = bm.XMinimum();
  const Vector2D tangential = Vector2D::Phi(phi);
  const Vector2D normal = tangential.orthogonal(ERotation::c_Clockwise);

  const double n0 = averages(2) - averages(0) * normal.x() - averages(1) * normal.y();

  // Transform the normal vector back into the original coordinate system.
  Vector2D n12 = normal.passiveRotatedBy(coordinate.flippedSecond());

  Line2D fitLine(n0, n12);
  return  fitLine;
}
