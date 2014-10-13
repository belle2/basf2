/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCObservations2D.h"

using namespace std;
using namespace Eigen;

using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCObservations2D)


CDCObservations2D::CDCObservations2D()
{
}



CDCObservations2D::~CDCObservations2D()
{
}



size_t CDCObservations2D::getNObservationsWithDriftRadius() const
{
  size_t result = 0;
  Index nObservations = size();

  for (Index iObservation = 0; iObservation < nObservations; ++iObservation) {
    const FloatType& driftLength = getDriftLength(iObservation);
    bool hasDriftLength = (driftLength != 0.0);
    result += hasDriftLength ? 1 : 0;
  }

  return result;
}



CDCObservations2D::EigenObservationMatrix CDCObservations2D::getObservationMatrix()
{

  size_t nObservations = size();
  FloatType* rawObservations = &(m_observations.front());

  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservations(rawObservations, nObservations, 4);
  return eigenObservations;

}


Vector2D CDCObservations2D::getCentralPoint() const
{
  size_t nObservations = size();
  if (nObservations == 0) return Vector2D(NAN, NAN);

  size_t iCentralObservation = nObservations / 2;

  FloatType centralX = getX(iCentralObservation);
  FloatType centralY = getY(iCentralObservation);

  return Vector2D(centralX, centralY);

  // May refine somehow ?
  // EigenObservationMatrix eigenObservations = getObservationMatrix();
  // RowVector2f meanPoint = eigenObservations.leftCols<2>.colwise().mean();
}



void CDCObservations2D::passiveMoveBy(const Vector2D& origin)
{
  Matrix< FloatType, 1, 2 >  eigenOrigin(origin.x(), origin.y());
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




Eigen::Matrix<FloatType, 5, 5> CDCObservations2D::getWXYRLSumMatrix()
{
  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  //B2INFO("Matrix of observations: " << endl << eigenObservation);

  Matrix< FloatType, Dynamic, 5 > projectedPoints(nObservations, 5);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iR2 = 3;
  const size_t iL = 4;

  projectedPoints.col(iW) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iR2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - eigenObservation.col(2).rowwise().squaredNorm();
  projectedPoints.col(iL) = eigenObservation.col(2);

  Array< FloatType, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< FloatType, Dynamic, 5 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< FloatType, 5, 5 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  //B2INFO("Matrix of sums: " << endl << sumMatrix);

  return sumMatrix;
}





Eigen::Matrix<FloatType, 4, 4> CDCObservations2D::getWXYLSumMatrix()
{

  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< FloatType, Dynamic, 4 > projectedPoints(nObservations, 4);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iL = 3;

  projectedPoints.col(iW) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iL) = eigenObservation.col(2);

  Array< FloatType, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< FloatType, Dynamic, 4 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< FloatType, 4, 4 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;




}



Eigen::Matrix<FloatType, 4, 4> CDCObservations2D::getWXYRSumMatrix()
{

  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< FloatType, Dynamic, 4 > projectedPoints(nObservations, 4);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;
  const size_t iR2 = 3;

  projectedPoints.col(iW) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);
  projectedPoints.col(iR2) = eigenObservation.leftCols<2>().rowwise().squaredNorm() - eigenObservation.col(2).rowwise().squaredNorm();

  Array< FloatType, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< FloatType, Dynamic, 4 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< FloatType, 4, 4 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;
  return sumMatrix;

}


Eigen::Matrix<FloatType, 3, 3> CDCObservations2D::getWXYSumMatrix()
{
  CDCObservations2D::EigenObservationMatrix&& eigenObservation = getObservationMatrix();
  size_t nObservations = size();

  Matrix< FloatType, Dynamic, 3 > projectedPoints(nObservations, 3);

  const size_t iW = 0;
  const size_t iX = 1;
  const size_t iY = 2;

  projectedPoints.col(iW) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0); //Offset column
  projectedPoints.col(iX) = eigenObservation.col(0);
  projectedPoints.col(iY) = eigenObservation.col(1);

  Array< FloatType, Dynamic, 1 > weights = eigenObservation.col(3);
  Matrix< FloatType, Dynamic, 3 > weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Matrix< FloatType, 3, 3 > sumMatrix  =  weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;
}

