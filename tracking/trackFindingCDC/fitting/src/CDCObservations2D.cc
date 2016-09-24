/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

using namespace Eigen;

using namespace Belle2;
using namespace TrackFindingCDC;

size_t CDCObservations2D::getNObservationsWithDriftRadius() const
{
  size_t result = 0;
  Index nObservations = size();

  for (Index iObservation = 0; iObservation < nObservations; ++iObservation) {
    const double driftLength = getDriftLength(iObservation);
    bool hasDriftLength = (driftLength != 0.0);
    result += hasDriftLength ? 1 : 0;
  }

  return result;
}



CDCObservations2D::EigenObservationMatrix CDCObservations2D::getObservationMatrix()
{

  size_t nObservations = size();
  double* rawObservations = &(m_observations.front());

  Map< Matrix< double, Dynamic, Dynamic, RowMajor > > eigenObservations(rawObservations, nObservations, 4);
  return eigenObservations;

}


Vector2D CDCObservations2D::getCentralPoint() const
{
  size_t n = size();
  if (n == 0) return Vector2D(NAN, NAN);
  size_t i = n / 2;

  if (isEven(n)) {
    // For even number of observations use the middle one with the bigger distance from IP
    Vector2D center1(getX(i), getY(i));
    Vector2D center2(getX(i - 1), getY(i - 1));
    return center1.normSquared() > center2.normSquared() ? center1 : center2;
  } else {
    Vector2D center1(getX(i), getY(i));
    return center1;
  }
}



void CDCObservations2D::passiveMoveBy(const Vector2D& origin)
{
  Matrix< double, 1, 2 >  eigenOrigin(origin.x(), origin.y());
  EigenObservationMatrix eigenObservations = getObservationMatrix();
  eigenObservations.leftCols<2>().rowwise() -= eigenOrigin;
}



Vector2D CDCObservations2D::centralize()
{
  // Pick an observation at the center
  Vector2D centralPoint = getCentralPoint();
  passiveMoveBy(centralPoint);
  return centralPoint;

}




Eigen::Matrix<double, 5, 5> CDCObservations2D::getWXYRLSumMatrix()
{
  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  //B2INFO("Matrix of observations: " << endl << eigenObservation);

  Matrix< double, Dynamic, 5 > projectedPoints(nObservations, 5);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iR2 = 3;
  const size_t iL = 4;

  projectedPoints.col(iW) = Matrix<double, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iR2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - eigenObservation.col(2).rowwise().squaredNorm();
  projectedPoints.col(iL) = eigenObservation.col(2);

  Array< double, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< double, Dynamic, 5 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< double, 5, 5 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  //B2INFO("Matrix of sums: " << endl << sumMatrix);

  return sumMatrix;
}





Eigen::Matrix<double, 4, 4> CDCObservations2D::getWXYLSumMatrix()
{

  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< double, Dynamic, 4 > projectedPoints(nObservations, 4);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iL = 3;

  projectedPoints.col(iW) = Matrix<double, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iL) = eigenObservation.col(2);

  Array< double, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< double, Dynamic, 4 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< double, 4, 4 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;




}



Eigen::Matrix<double, 4, 4> CDCObservations2D::getWXYRSumMatrix()
{

  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< double, Dynamic, 4 > projectedPoints(nObservations, 4);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iR2 = 3;

  projectedPoints.col(iW) = Matrix<double, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iR2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - eigenObservation.col(2).rowwise().squaredNorm();

  Array< double, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< double, Dynamic, 4 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< double, 4, 4 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;
  return sumMatrix;

}


Eigen::Matrix<double, 3, 3> CDCObservations2D::getWXYSumMatrix()
{
  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< double, Dynamic, 3 > projectedPoints(nObservations, 3);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;

  projectedPoints.col(iW) = Matrix<double, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);

  Array< double, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< double, Dynamic, 3 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< double, 3, 3 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;
}

