/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRiemannFitter.h"

#include <Eigen/Dense>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRiemannFitter)

CDCRiemannFitter::CDCRiemannFitter() : m_usePosition(true), m_useOrientation(true),
  m_lineConstrained(false) , m_originConstrained(false) {;}

/** Destructor. */
CDCRiemannFitter::~CDCRiemannFitter()
{
}


/*
size_t CDCRiemannFitter::fillObservations(const CDCRecoHit2DSet& recohits, vector<FloatType>& observations) const
{
  size_t result = 0;
  for (CDCRecoHit2DSet::const_iterator itRecoHit = recohits.begin();  itRecoHit != recohits.end(); ++itRecoHit) {
    result += fillObservation(*itRecoHit, observations);
  }
  return result;
  }*/

size_t CDCRiemannFitter::fillObservations(const CDCRecoHit2DVector& recohits, vector<FloatType>& observations) const
{

  size_t result = 0;
  for (CDCRecoHit2DVector::const_iterator itRecoHit = recohits.begin();  itRecoHit != recohits.end(); ++itRecoHit) {
    result += fillObservation(*itRecoHit, observations);
  }
  return result;

}

/*
size_t CDCRiemannFitter::fillObservations(const CDCSegmentTriple& segmentTriple, std::vector<FloatType>& observations) const
{

  const CDCAxialRecoSegment2D* startSegment = segmentTriple.getStart();
  size_t result = startSegment == nullptr ? 0 : fillObservations(*startSegment, observations);

  const CDCAxialRecoSegment2D* endSegment = segmentTriple.getEnd();
  result += endSegment == nullptr ? 0 : fillObservations(*endSegment, observations);

  return result;
}
*/

size_t CDCRiemannFitter::fillObservation(const CDCRecoHit2D& recohit, vector<FloatType>& observations) const
{
  size_t result = 0;
  if (m_usePosition) {

    Vector2D position = recohit.getRefPos2D();

    observations.push_back(position.x());
    observations.push_back(position.y());
    observations.push_back(0.0);

  }
  if (m_useOrientation) {

    const CDCWireHit* wirehit = recohit.getWireHit();
    Vector2D positionOfWire = wirehit->getRefPos2D();

    observations.push_back(positionOfWire.x());
    observations.push_back(positionOfWire.y());
    observations.push_back(SignType(recohit.getRLInfo()) * wirehit->getRefDriftLength());

    result = observations.back() == 0 ? 0 : 1;

  }
  return result;
}


/*
void CDCRiemannFitter::update(CDCTrajectory2D & fit,const CDCRecoSegment & recosegments) const{
  updateGeneric(fit,recosegments.begin(),recosegments.end(),recosegments.size());
}

void CDCRiemannFitter::update(CDCTrajectory2D & fit, const CDCRecoSegment & fromRecoSegment, const CDCRecoSegment & toRecoSegment )const{

  CDCRecoSegment combinedSegment;
  combinedSegment.expand(fromRecoSegment);
  combinedSegment.expand(toRecoSegment);
  updateGeneric(fit,combinedSegment.begin(),combinedSegment.end(),combinedSegment.size());

}*/
/*

template<class RecoHitInputIterator>
void CDCRiemannFitter::updateGeneric(CDCTrajectory2D & fit,
                                     RecoHitInputIterator recohitsBegin , RecoHitInputIterator recohitsEnd,
                                     size_t nRecoHits) const{


  // first find out how many observations we have
  size_t multi = 0;
  if (m_usePosition) ++multi;
  if (m_useOrientation) ++multi;

  size_t nRows = multi*nRecoHits;

  FloatType observations[nRows][3];
  //observations has three columns
  // first for the x coordinate
  // second for the y coordinate
  // third the signed desired distance from that point,
  //   where the sign indicates whether the point lies to the right or left of the fitted trajectory

  //fill the observations
  size_t iArrayRow = 0;

  if ( m_usePosition ){
    TVector2 position;
    for ( RecoHitInputIterator itRecoHit = recohitsBegin;
          itRecoHit != recohitsEnd;
          ++itRecoHit, ++iArrayRow){

      const CDCRecoHitCand & recohit = *itRecoHit;
      position = recohit.getPosition();

      observations[iArrayRow][0] = position.X();
      observations[iArrayRow][1] = position.Y();
      observations[iArrayRow][2] = 0.0;
    }
  }

  size_t nLeftRightInfo = 0;
  if ( m_useOrientation ){
    TVector2 positionOfWire;
    for ( RecoHitInputIterator itRecoHit = recohitsBegin;
          itRecoHit != recohitsEnd;
          ++itRecoHit, ++iArrayRow){

      const CDCRecoHitCand & recohit = *itRecoHit;
      CDCWireHit * wirehit = recohit.getWireHit();

      positionOfWire = wirehit->getPosition();

      observations[iArrayRow][0] = positionOfWire.X();
      observations[iArrayRow][1] = positionOfWire.Y();
      observations[iArrayRow][2] = recohit.getRightLeftInfo() * wirehit->getDriftCircleRadius();
      if( recohit.getRightLeftInfo() != 0) ++nLeftRightInfo;
    }
  }

  const bool usesRightLeftInfo = nLeftRightInfo > 0;

  update(fit,observations,nRows,usesRightLeftInfo);


}  */


void CDCRiemannFitter::update(CDCTrajectory2D& fit, std::vector<FloatType>& observations, bool usesRightLeftInfo) const
{

  size_t nObservations = observations.size() / 3;
  FloatType* rawObservations = &(observations.front());

  if (usesRightLeftInfo) {
    updateWithRightLeft(fit, rawObservations, nObservations);

  } else {

    updateWithOutRightLeft(fit, rawObservations, nObservations);

  }
}


void CDCRiemannFitter::updateWithOutRightLeft(CDCTrajectory2D& fit,
                                              FloatType* observations,
                                              size_t nObservations) const
{

  //observations always have the structure
  /*
  observables[0][0] == x of first point
  observables[0][1] == y of first point
  observables[0][2] == desired distance of first point
  */

  //c++ arrays are always row major
  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservation(observations, nObservations, 3);

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
    fit.setGenCircle(GeneralizedCircle(offset, normalToLine(0), normalToLine(1), 0));

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

    fit.setGenCircle(GeneralizedCircle(offset, normalToPlane(0), normalToPlane(1), normalToPlane(2)));
    //fit.setParameters();

  }

  //check if the orientation is alright
  Vector2D directionAtCenter = fit.getUnitMom2D(Vector2D(0.0, 0.0));


  size_t voteForChangeSign = 0;
  for (size_t iPoint = 0; iPoint < nObservations; ++iPoint) {
    FloatType pointInSameDirection = eigenObservation(iPoint, 0) * directionAtCenter.x() +
                                     eigenObservation(iPoint, 1) * directionAtCenter.y();
    if (pointInSameDirection < 0) ++voteForChangeSign;
  }

  if (voteForChangeSign > nObservations / 2.0) fit.reverse();

}


void CDCRiemannFitter::updateWithRightLeft(CDCTrajectory2D& fit,
                                           FloatType* observations,
                                           size_t nObservations) const
{

  //cout << "updateWithRightLeft : " << endl;
  //observations always have the structure
  /*
  observables[0][0] == x of first point
  observables[0][1] == y of first point
  observables[0][2] == desired distance of first point
  */


  //c++ arrays are always row major
  Map< Matrix<FloatType, Dynamic, Dynamic , RowMajor > > eigenObservation(observations, nObservations, 3);

  Matrix< FloatType, Dynamic, 1 > distances = eigenObservation.col(2);

  //cout << "distances : " << endl << distances << endl;

  if ((isLineConstrained()) && (isOriginConstrained())) {


    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 2);

    //all coordiates
    //projectedPoints.col(0) = Matrix<FloatType,Dynamic,1>::Constant(nObservations,1.0);
    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    //projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 2, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(0.0, parameters(0), parameters(1), 0.0));
  }

  else if ((! isLineConstrained()) && (isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 3);

    //all coordiates
    //projectedPoints.col(0) = Matrix<FloatType,Dynamic,1>::Constant(1.0);
    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 3, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(0.0, parameters(0), parameters(1), parameters(2)));
  }

  else if ((isLineConstrained()) && (! isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 3);

    //all coordiates
    projectedPoints.col(0) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
    projectedPoints.col(1) = eigenObservation.col(0);
    projectedPoints.col(2) = eigenObservation.col(1);
    //projectedPoints.col(3) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 3, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(parameters(0), parameters(1), parameters(2), 0.0));
    //fit.setParameters(parameters(0),parameters(1),parameters(2),0.0);

  }

  else if ((! isLineConstrained()) && (! isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 4);

    //all coordiates
    projectedPoints.col(0) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
    projectedPoints.col(1) = eigenObservation.col(0);
    projectedPoints.col(2) = eigenObservation.col(1);
    projectedPoints.col(3) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    //cout << "points : " << endl << projectedPoints << endl;

    Matrix< FloatType, 4, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(parameters(0), parameters(1), parameters(2), parameters(3)));

  }


}


