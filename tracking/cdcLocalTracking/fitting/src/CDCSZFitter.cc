/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCSZFitter.h"


#include <Eigen/Dense>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

using namespace Eigen;

ClassImpInCDCLocalTracking(CDCSZFitter)

CDCSZFitter::CDCSZFitter()
{
}

/** Destructor. */
CDCSZFitter::~CDCSZFitter()
{
}

void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         const CDCStereoRecoSegment2D& stereoSegment,
                         const CDCTrajectory2D& axialTrajectory2D) const
{
  //recostruct the stereo segment
  CDCRecoSegment3D reconstructedStereoSegment;
  for (const CDCRecoHit2D & recoHit2D : stereoSegment) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, axialTrajectory2D);
    reconstructedStereoSegment.push_back(recoHit3D);
  }

  update(trajectorySZ, reconstructedStereoSegment);

}



void CDCSZFitter::updateOptimizeSZDistance(CDCTrajectorySZ& trajectorySZ,
                                           FloatType* observations,
                                           size_t nObservations) const
{

  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservation(observations, nObservations, 2);

  //do a normal line fit
  Matrix<FloatType, Dynamic, 2> points = eigenObservation.leftCols<2>();

  Matrix<FloatType, 1, 2> pointMean;
  //RowVector2d pointMean;
  pointMean << 0.0, 0.0;

  // subtract the offset from the origin
  pointMean = points.colwise().mean();

  points = points.rowwise() - pointMean;

  Matrix<FloatType, 2, 2> covarianceMatrix = points.transpose() * points;

  SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(covarianceMatrix);

  if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");

  //the eigenvalues are generated in increasing order
  //we are interested in the lowest one since we want to compute the normal vector of the line

  Matrix<FloatType, 2, 1> normalToLine = eigensolver.eigenvectors().col(0);
  //Matrix<FloatType,2,1> parallelToLine = eigensolver.eigenvectors().col(1);

  FloatType offset = -pointMean * normalToLine;

  // line equation is normalToLine(0) * s + normalToLine(1) * z + offset = 0
  // the point mean lies in the line

  trajectorySZ.setN(offset, normalToLine(0) , normalToLine(1));

}


void CDCSZFitter::updateOptimizeZDistance(CDCTrajectorySZ& trajectorySZ,
                                          FloatType* observations,
                                          size_t nObservations) const
{

  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservation(observations, nObservations, 2);

  Matrix< FloatType, Dynamic, 1 > zObservations = eigenObservation.rightCols<1>();
  Matrix< FloatType, Dynamic, Dynamic > offsetAndS(nObservations, 2);

  //all coordiates
  offsetAndS.col(0) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
  offsetAndS.col(1) = eigenObservation.leftCols<1>();

  Matrix< FloatType, 2, 1> interceptAndSlope = offsetAndS.jacobiSvd(ComputeThinU | ComputeThinV).solve(zObservations);

  trajectorySZ.setSlopeIntercept(interceptAndSlope(1), interceptAndSlope(0));

}


