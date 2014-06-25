/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCKarimakiFitter.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCKarimakiFitter)


namespace {
  CDCKarimakiFitter* g_fitter = nullptr;
  CDCKarimakiFitter* g_lineFitter = nullptr;
  CDCKarimakiFitter* g_originCircleFitter = nullptr;
}



const CDCKarimakiFitter& CDCKarimakiFitter::getFitter()
{
  if (not g_fitter) {
    g_fitter = new CDCKarimakiFitter();
  }
  return *g_fitter;
}



const CDCKarimakiFitter& CDCKarimakiFitter::getLineFitter()
{
  if (not g_lineFitter) {
    g_lineFitter = new CDCKarimakiFitter();
    g_lineFitter->setLineConstrained();
  }
  return *g_lineFitter;
}



const CDCKarimakiFitter& CDCKarimakiFitter::getOriginCircleFitter()
{
  if (not g_originCircleFitter) {
    g_originCircleFitter = new CDCKarimakiFitter();
    g_originCircleFitter->setOriginConstrained();
  }
  return *g_originCircleFitter;
}




CDCKarimakiFitter::CDCKarimakiFitter() : m_usePosition(true), m_useOrientation(true),
  m_lineConstrained(false) , m_originConstrained(false) {;}

CDCKarimakiFitter::~CDCKarimakiFitter()
{

}




CDCTrajectory2D CDCKarimakiFitter::fit(const CDCRecoSegment2D& recoSegment2D) const
{
  CDCTrajectory2D result;
  update(result, recoSegment2D);
  return result;
}


CDCTrajectory2D CDCKarimakiFitter::fit(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const
{
  CDCTrajectory2D result;
  update(result, axialAxialSegmentPair);
  return result;
}



void CDCKarimakiFitter::update(CDCTrajectory2D& trajectory2D, const CDCRecoSegment2D& recoSegment2D) const
{

  CDCObservations2D observations2D;
  if (m_usePosition) {
    observations2D.append(recoSegment2D, true);
  }
  if (m_useOrientation) {
    observations2D.append(recoSegment2D, false);
  }

  update(trajectory2D, observations2D);

  //Set transverse s reference
  // trajectory2D.setStartPos2D(recoSegment2D.front().getRecoPos2D()) ;
  trajectory2D.setStartPos2D(recoSegment2D.getCenterOfMass2D()) ;

  //Check if fit is forward
  if (not recoSegment2D.isForwardTrajectory(trajectory2D)) trajectory2D.reverse();
  if (not recoSegment2D.isForwardTrajectory(trajectory2D)) B2WARNING("Fit cannot be oriented correctly");

}

void CDCKarimakiFitter::update(CDCTrajectory2D& trajectory2D,
                               const CDCRecoSegment2D& firstRecoSegment2D,
                               const CDCRecoSegment2D& secondRecoSegment2D) const
{

  CDCObservations2D observations2D;
  if (m_usePosition) {
    observations2D.append(firstRecoSegment2D, true);
    observations2D.append(secondRecoSegment2D, true);
  }
  if (m_useOrientation) {
    observations2D.append(firstRecoSegment2D, false);
    observations2D.append(secondRecoSegment2D, false);
  }

  update(trajectory2D, observations2D);

  //set transverse s reference
  //trajectory2D.setStartPos2D(firstRecoSegment2D.front().getRecoPos2D()) ;
  trajectory2D.setStartPos2D(firstRecoSegment2D.getCenterOfMass2D()) ;

  //check if fit is forward
  if (not firstRecoSegment2D.isForwardTrajectory(trajectory2D)) trajectory2D.reverse();

  if (not(firstRecoSegment2D.isForwardTrajectory(trajectory2D) and secondRecoSegment2D.isForwardTrajectory(trajectory2D)))
    B2WARNING("Fit cannot be oriented correctly");

}



void CDCKarimakiFitter::update(CDCTrajectory2D& trajectory2D, const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const
{
  update(trajectory2D, *(axialAxialSegmentPair.getStart()), *(axialAxialSegmentPair.getEnd()));
}



void CDCKarimakiFitter::update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
{

  if (observations2D.getNObservationsWithDriftRadius() > 0) {
    updateWithRightLeft(trajectory2D, observations2D);
  } else {
    updateWithOutRightLeft(trajectory2D, observations2D);
  }

}





void CDCKarimakiFitter::updateWithOutRightLeft(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
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





void CDCKarimakiFitter::updateWithRightLeft(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
{
  // Assume a central point
  Vector2D referencePoint = observations2D.centralize();

  // Unpack the informations
  CDCObservations2D::EigenObservationMatrix && eigenObservation = observations2D.getObservationMatrix();
  size_t nObservations = observations2D.size();

  Matrix< FloatType, Dynamic, 1 > driftLengths = eigenObservation.col(2);

  Matrix< FloatType, Dynamic, 4 > projectedPoints(nObservations, 4);

  //all coordiates
  projectedPoints.col(0) = eigenObservation.col(0);
  projectedPoints.col(1) = eigenObservation.col(1);
  projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - driftLengths.rowwise().squaredNorm();
  projectedPoints.col(3) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column

  Array< FloatType, Dynamic, 1 > weights = Array<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
  Matrix< FloatType, Dynamic, 4 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;

  Matrix< FloatType, 4, 4 > s = projectedPoints.transpose() * weightedProjectedPoints;

  Matrix< FloatType, 4, 1 > l = weightedProjectedPoints .transpose() * driftLengths;
  Matrix< FloatType, 4, 1 > n = s.ldlt().solve(l);

  PerigeeCircle resultCircle = PerigeeCircle::fromN(n(0), n(1), n(2), n(3));

  FloatType curvature = resultCircle.curvature();
  FloatType impact = resultCircle.impact();
  Vector2D tangential = resultCircle.tangential().unit();
  FloatType tangentialPhi = tangential.phi();

  FloatType cosphi = tangential.x();
  FloatType sinphi = tangential.y();

  trajectory2D.setCircle(resultCircle);
  return;

  FloatType sw = s(3, 3);
  Matrix< FloatType, 4, 4 > a = s / sw;
  Matrix< FloatType, 3, 1> observationAverage = a.topRightCorner<3, 1>();

  Matrix< FloatType, 3, 3 > c = a.topLeftCorner<3, 3>() - observationAverage * observationAverage.transpose();

  FloatType sx = s(0, 3);
  FloatType sy = s(1, 3);
  FloatType sr = s(2, 3);

  FloatType sxx = s(0, 0);
  FloatType sxy = s(0, 1);
  FloatType sxr = s(0, 2);

  FloatType syy = s(1, 1);
  FloatType syr = s(1, 2);

  FloatType srr = s(2, 2);

  FloatType cxx = c(0, 0);
  FloatType cxy = c(0, 1);
  FloatType cxr = c(0, 2);

  FloatType cyy = c(1, 1);
  FloatType cyr = c(1, 2);

  FloatType crr = c(2, 2);

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

    UncertainPerigeeCircle(curvature, tangentialPhi, impact, perigeeCovariance);
  }

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


