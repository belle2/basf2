/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/EigenObservationMatrix.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::EigenObservationMatrix
TrackFindingCDC::getEigenObservationMatrix(CDCObservations2D* observations2D)
{
  std::size_t nObservations = observations2D->size();
  double* rawObservations = observations2D->data();
  EigenObservationMatrix eigenObservations(rawObservations, nObservations, 4);
  return eigenObservations;
}

TrackFindingCDC::EigenObservationMatrix
TrackFindingCDC::getEigenObservationMatrix(CDCSZObservations* szObservations)
{
  std::size_t nObservations = szObservations->size();
  double* rawObservations = szObservations->data();
  EigenObservationMatrix eigenObservations(rawObservations, nObservations, 3);
  return eigenObservations;
}

Eigen::Matrix<double, 5, 5> TrackFindingCDC::getWXYRLSumMatrix(CDCObservations2D& observations2D)
{
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(&observations2D);
  assert(eigenObservations.cols() == 4);
  std::size_t nObservations = eigenObservations.rows();

  // B2INFO("Matrix of observations: " << endl << eigenObservations);

  Eigen::Matrix<double, Eigen::Dynamic, 5> projectedPoints(nObservations, 5);

  const std::size_t iW = 0;
  const std::size_t iX = 1;
  const std::size_t iY = 2;
  const std::size_t iR2 = 3;
  const std::size_t iL = 4;

  projectedPoints.col(iW) =
    Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(nObservations, 1.0); // Offset column
  projectedPoints.col(iX) = eigenObservations.col(0);
  projectedPoints.col(iY) = eigenObservations.col(1);
  projectedPoints.col(iR2) = eigenObservations.leftCols<2>().rowwise().squaredNorm() -
                             eigenObservations.col(2).rowwise().squaredNorm();
  projectedPoints.col(iL) = eigenObservations.col(2);

  Eigen::Array<double, Eigen::Dynamic, 1> weights = eigenObservations.col(3);
  Eigen::Matrix<double, Eigen::Dynamic, 5> weightedProjectedPoints =
    projectedPoints.array().colwise() * weights;
  Eigen::Matrix<double, 5, 5> sumMatrix = weightedProjectedPoints.transpose() * projectedPoints;

  // B2INFO("Matrix of sums: " << endl << sumMatrix);

  return sumMatrix;
}

Eigen::Matrix<double, 4, 4> TrackFindingCDC::getWXYLSumMatrix(CDCObservations2D& observations2D)
{
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(&observations2D);
  assert(eigenObservations.cols() == 4);
  std::size_t nObservations = eigenObservations.rows();

  Eigen::Matrix<double, Eigen::Dynamic, 4> projectedPoints(nObservations, 4);

  const std::size_t iW = 0;
  const std::size_t iX = 1;
  const std::size_t iY = 2;
  const std::size_t iL = 3;

  projectedPoints.col(iW) =
    Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(nObservations, 1.0); // Offset column
  projectedPoints.col(iX) = eigenObservations.col(0);
  projectedPoints.col(iY) = eigenObservations.col(1);
  projectedPoints.col(iL) = eigenObservations.col(2);

  Eigen::Array<double, Eigen::Dynamic, 1> weights = eigenObservations.col(3);
  Eigen::Matrix<double, Eigen::Dynamic, 4> weightedProjectedPoints =
    projectedPoints.array().colwise() * weights;
  Eigen::Matrix<double, 4, 4> sumMatrix = weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;
}

Eigen::Matrix<double, 4, 4> TrackFindingCDC::getWXYRSumMatrix(CDCObservations2D& observations2D)
{
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(&observations2D);
  assert(eigenObservations.cols() == 4);
  std::size_t nObservations = eigenObservations.rows();

  Eigen::Matrix<double, Eigen::Dynamic, 4> projectedPoints(nObservations, 4);

  const std::size_t iW = 0;
  const std::size_t iX = 1;
  const std::size_t iY = 2;
  const std::size_t iR2 = 3;

  projectedPoints.col(iW) =
    Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(nObservations, 1.0); // Offset column
  projectedPoints.col(iX) = eigenObservations.col(0);
  projectedPoints.col(iY) = eigenObservations.col(1);
  projectedPoints.col(iR2) = eigenObservations.leftCols<2>().rowwise().squaredNorm() -
                             eigenObservations.col(2).rowwise().squaredNorm();

  Eigen::Array<double, Eigen::Dynamic, 1> weights = eigenObservations.col(3);
  Eigen::Matrix<double, Eigen::Dynamic, 4> weightedProjectedPoints =
    projectedPoints.array().colwise() * weights;
  Eigen::Matrix<double, 4, 4> sumMatrix = weightedProjectedPoints.transpose() * projectedPoints;
  return sumMatrix;
}

Eigen::Matrix<double, 3, 3> TrackFindingCDC::getWXYSumMatrix(CDCObservations2D& observations2D)
{
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(&observations2D);
  assert(eigenObservations.cols() == 4);
  std::size_t nObservations = eigenObservations.rows();

  Eigen::Matrix<double, Eigen::Dynamic, 3> projectedPoints(nObservations, 3);

  const std::size_t iW = 0;
  const std::size_t iX = 1;
  const std::size_t iY = 2;

  projectedPoints.col(iW) =
    Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(nObservations, 1.0); // Offset column
  projectedPoints.col(iX) = eigenObservations.col(0);
  projectedPoints.col(iY) = eigenObservations.col(1);

  Eigen::Array<double, Eigen::Dynamic, 1> weights = eigenObservations.col(3);
  Eigen::Matrix<double, Eigen::Dynamic, 3> weightedProjectedPoints =
    projectedPoints.array().colwise() * weights;
  Eigen::Matrix<double, 3, 3> sumMatrix = weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;
}

Eigen::Matrix<double, 3, 3> TrackFindingCDC::getWSZSumMatrix(CDCSZObservations& szObservations)
{
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(&szObservations);
  assert(eigenObservations.cols() == 3);
  std::size_t nObservations = eigenObservations.rows();

  Eigen::Matrix<double, Eigen::Dynamic, 3> projectedPoints(nObservations, 3);

  const std::size_t iW = 0;
  const std::size_t iS = 1;
  const std::size_t iZ = 2;

  projectedPoints.col(iW) = Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(nObservations, 1.0); // Offset column
  projectedPoints.col(iS) = eigenObservations.col(0);
  projectedPoints.col(iZ) = eigenObservations.col(1);

  Eigen::Array<double, Eigen::Dynamic, 1> weights = eigenObservations.col(2);
  Eigen::Matrix<double, Eigen::Dynamic, 3> weightedProjectedPoints = projectedPoints.array().colwise() * weights;
  Eigen::Matrix<double, 3, 3> sumMatrix = weightedProjectedPoints.transpose() * projectedPoints;

  return sumMatrix;
}
