/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/KarimakisMethod.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(KarimakisMethod)


namespace {

}



KarimakisMethod::KarimakisMethod() :
  m_lineConstrained(false),
  m_originConstrained(false)
{
}



KarimakisMethod::~KarimakisMethod()
{
}



void KarimakisMethod::update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
{

  updateWithDriftLength(trajectory2D, observations2D);

  // if (observations2D.getNObservationsWithDriftRadius() > 0) {

  // } else {
  //   updateWithoutDriftLength(trajectory2D, observations2D);
  //}

}



void KarimakisMethod::updateWithoutDriftLength(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
{

  CDCObservations2D::EigenObservationMatrix&&  eigenObservation = observations2D.getObservationMatrix();

  size_t nObservations = observations2D.size();


  if (isLineConstrained()) {

    //do a normal line fit
    Matrix<FloatType, Dynamic, 2> points = eigenObservation.leftCols<2>();

    Matrix<FloatType, 1, 2> pointMean;
    //RowVector2d pointMean;
    pointMean << 0.0, 0.0;
    if (!(isOriginConstrained())) {
      // subtract the offset from the origin
      pointMean = points.colwise().mean();

      points = points.rowwise() - pointMean;
    }
    Matrix<FloatType, 2, 2> covarianceMatrix = points.transpose() * points;

    SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(covarianceMatrix);

    if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");

    //the eigenvalues are generated in increasing order
    //we are interested in the lowest one since we want to compute the normal vector of the plane

    Matrix<FloatType, 2, 1> normalToLine = eigensolver.eigenvectors().col(0);

    FloatType offset = -pointMean * normalToLine;

    // set the generalized circle parameters
    // last set to zero constrains to a line
    trajectory2D.setCircle(PerigeeCircle::fromN(offset, normalToLine(0), normalToLine(1), 0));

  } else {

    //lift the points to the projection space
    Matrix<FloatType, Dynamic, 3> projectedPoints(nObservations, 3);

    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();


    Matrix<FloatType, 1, 3> pointMean;
    pointMean << 0.0, 0.0, 0.0;
    if (!(isOriginConstrained())) {
      // subtract the offset from the origin
      pointMean = projectedPoints.colwise().mean();

      projectedPoints = projectedPoints.rowwise() - pointMean;
    }

    Matrix<FloatType, 3, 3> covarianceMatrix = projectedPoints.transpose() * projectedPoints;

    SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(covarianceMatrix);

    if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");

    //the eigenvalues are generated in increasing order
    //we are interested in the lowest one since we want to compute the normal vector of the plane

    Matrix<FloatType, 3, 1> normalToPlane = eigensolver.eigenvectors().col(0);

    FloatType offset = -pointMean * normalToPlane;

    trajectory2D.setCircle(PerigeeCircle::fromN(offset, normalToPlane(0), normalToPlane(1), normalToPlane(2)));
    //fit.setParameters();

  }

  //check if the orientation is alright
  Vector2D directionAtCenter = trajectory2D.getUnitMom2D(Vector2D(0.0, 0.0));


  size_t voteForChangeSign = 0;
  for (size_t iPoint = 0; iPoint < nObservations; ++iPoint) {
    FloatType pointInSameDirection = eigenObservation(iPoint, 0) * directionAtCenter.x() +
                                     eigenObservation(iPoint, 1) * directionAtCenter.y();
    if (pointInSameDirection < 0) ++voteForChangeSign;
  }

  if (voteForChangeSign > nObservations / 2.0) trajectory2D.reverse();

}



void KarimakisMethod::updateWithDriftLength(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
{
  // Assume a central point
  Vector2D referencePoint = observations2D.centralize();
  size_t nObservations = observations2D.size();
  size_t nObservationsWithDriftRadius = observations2D.getNObservationsWithDriftRadius();

  UncertainPerigeeCircle resultCircle;

  // Unpack the informations
  CDCObservations2D::EigenObservationMatrix && eigenObservation = observations2D.getObservationMatrix();

  Matrix< FloatType, Dynamic, 1 > driftLengths = eigenObservation.col(2);
  Matrix< FloatType, Dynamic, 4 > projectedPoints(nObservations, 4);

  const size_t iX = 0;
  const size_t iY = 1;
  const size_t iR2 = 2;
  const size_t iW = 3;

  //all coordiates

  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iR2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - driftLengths.rowwise().squaredNorm();
  projectedPoints.col(iW) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column

  //Array< FloatType, Dynamic, 1 > weights = Array<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
  Matrix< FloatType, Dynamic, 4 > weightedProjectedPoints = projectedPoints; //.array().colwise() * weights;

  Matrix< FloatType, 4, 4 > s =  weightedProjectedPoints.transpose() * projectedPoints;
  Matrix< FloatType, 4, 1 > l = weightedProjectedPoints.transpose() * driftLengths;

  FloatType sw = s(0, 0);
  Matrix< FloatType, 4, 4 > a = s / sw;
  Matrix< FloatType, 3, 1> observationAverage = a.topRightCorner<3, 1>();
  Matrix< FloatType, 3, 3 > c = a.topLeftCorner<3, 3>() - observationAverage * observationAverage.transpose();


  if (nObservationsWithDriftRadius > 0) {
    Matrix< FloatType, 4, 1 > n = s.ldlt().solve(l);
    resultCircle.setN(n(iW), n(iX), n(iY), n(iR2));

  } else {
    B2INFO("Here");
    SelfAdjointEigenSolver< Matrix<FloatType, 4, 4> > eigensolver(s);
    if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    Matrix<FloatType, 4, 1> n = eigensolver.eigenvectors().col(0);
    resultCircle.setN(n(iW), n(iX), n(iY), n(iR2));
    resultCircle.reverse();
    // SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(c);
    // if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    // //the eigenvalues are generated in increasing order
    // //we are interested in the lowest one since we want to compute the normal vector of the plane
    // Matrix<FloatType, 3, 1> normalToPlane = eigensolver.eigenvectors().col(0);
    // FloatType offset = -observationAverage.transpose() * normalToPlane;
    // resultCircle.setN(offset, normalToPlane(iX), normalToPlane(iY), normalToPlane(iR2));

  }

  //B2INFO("s = " << endl << s);
  //B2INFO("l = " << endl << l);
  //B2INFO("n = " << endl << n);

  FloatType curvature = resultCircle.curvature();
  FloatType impact = resultCircle.impact();
  Vector2D tangential = resultCircle.tangential().unit();
  FloatType tangentialPhi = tangential.phi();

  FloatType sx = s(iX, iW);
  FloatType sy = s(iY, iW);
  FloatType sr = s(iR2, iW);

  FloatType sxx = s(iX, iX);
  FloatType sxy = s(iX, iY);
  FloatType sxr = s(iX, iR2);

  FloatType syy = s(iY, iY);
  FloatType syr = s(iY, iR2);

  FloatType srr = s(iR2, iR2);

  FloatType cxx = c(iX, iX);
  FloatType cxy = c(iX, iY);
  FloatType cxr = c(iX, iR2);

  FloatType cyy = c(iY, iY);
  FloatType cyr = c(iY, iR2);

  FloatType crr = c(iR2, iR2);

  TMatrixD inversePerigeeCovariance(3, 3);

  double q1, q2;
  if (not isLineConstrained()) {
    q1 = crr * cxy - cxr * cyr;
    q2 = crr * (cxx - cyy) - cxr * cxr + cyr * cyr;
  } else {
    q1 = cxy;
    q2 = cxx - cyy;
  }

  //double phi = 0.5 * atan2(2. * q1, q2);
  //double sinphi = sin(phi);
  //double cosphi = cos(phi);

  // compare phi with initial track direction
  //if (cosphi * (ax + _xRef) + sinphi * (ay + _yRef) < 0.) {
  // reverse direction
  //   phi -= (phi > 0.) ? M_PI : -M_PI;
  //  cosphi = -cosphi;
  //  sinphi = -sinphi;
  //}

  if (isLineConstrained()) {
    //TODO

  } else {
    B2INFO("Here 2");
    // Karimaki has opposite phi sign convention
    FloatType cosphi = tangential.x();
    FloatType sinphi = tangential.y();


    double kappa = (sinphi * cxr - cosphi * cyr) / crr;
    // double delta = -kappa * ar + sinphi * ax - cosphi * ay;
    // track parameters
    //double rho = -2. * kappa / sqrt(1. - 4. * delta * kappa);
    double rho = curvature;
    //double d = 2. * delta / (1. + sqrt(1. - 4. * delta * kappa));
    double d = impact;

    // _parameters[0] = rho;
    // _parameters[1] = phi;
    // _parameters[2] = d;

    FloatType u = 1. - rho * d;
    FloatType Chi2 = sw * u * u * (sinphi * sinphi * cxx - 2. * sinphi * cosphi * cxy + cosphi * cosphi * cyy - kappa * kappa * crr);
    //cout << " xyfit " << Chi2 << " " << _numPoints << " " <<_npar << ": " << rho << " " << phi << " " << d << endl;

    // calculate covariance matrix
    double sa = sinphi * sx - cosphi * sy;
    double sb = cosphi * sx + sinphi * sy;
    double sc = (sinphi * sinphi - cosphi * cosphi) * sxy + sinphi * cosphi * (sxx - syy);
    double sd = sinphi * sxr - cosphi * syr;
    double saa = sinphi * sinphi * sxx - 2. * sinphi * cosphi * sxy + cosphi * cosphi * syy;
    inversePerigeeCovariance(0, 0) = 0.25 * srr - d * (sd - d * (saa + 0.5 * sr - d * (sa - 0.25 * d * sw)));
    inversePerigeeCovariance(0, 1) = u * (0.5 * (cosphi * sxr + sinphi * syr) - d * (sc - 0.5 * d * sb));
    inversePerigeeCovariance(1, 0) =  inversePerigeeCovariance(0, 1);
    inversePerigeeCovariance(1, 1) = u * u * (cosphi * cosphi * sxx + 2. * cosphi * sinphi * sxy + sinphi * sinphi * syy);
    inversePerigeeCovariance(0, 2) = rho * (-0.5 * sd + d * saa) - 0.5 * u * sr + 0.5 * d * ((3 * u - 1.) * sa - u * d * sw);
    inversePerigeeCovariance(2, 0) =  inversePerigeeCovariance(0, 2);
    inversePerigeeCovariance(1, 2) = -u * (rho * sc + u * sb);
    inversePerigeeCovariance(2, 1) =  inversePerigeeCovariance(1, 2);
    inversePerigeeCovariance(2, 2) = rho * (rho * saa + 2 * u * sa) + u * u * sw;

    //cout << "points : " << endl << projectedPoints << endl;
    TMatrixD perigeeCovariance = inversePerigeeCovariance;
    perigeeCovariance.Invert();
    //perigeeCovariance.Print();
    UncertainPerigeeCircle(curvature, tangentialPhi, impact, perigeeCovariance);

    resultCircle.setPerigeeCovariance(perigeeCovariance);
  }

  resultCircle.passiveMoveBy(-referencePoint);
  trajectory2D.setCircle(resultCircle);
  B2INFO("Reference point" << referencePoint);

  return;

  //cout << "updateWithRightLeft : " << endl;
  //observations always have the structure
  /*
  observables[0][0] == x of first point
  observables[0][1] == y of first point
  observables[0][2] == desired distance of first point
  */

  // Build the covariance matix of
  // * x
  // * y
  // * rho^2 - r^2
}


