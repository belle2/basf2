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

CDCLOCALTRACKING_SwitchableClassImp(CDCSZFitter)





CDCSZFitter::CDCSZFitter()
{
}





CDCSZFitter::~CDCSZFitter()
{
}





namespace {
  CDCSZFitter* g_szFitter = nullptr;
}





const CDCSZFitter& CDCSZFitter::getFitter()
{
  if (not g_szFitter) {
    g_szFitter = new CDCSZFitter();
  }
  return *g_szFitter;
}





namespace {
  UncertainSZLine fitZ(const Matrix< FloatType, 3, 3 >& sumMatrixWSZ)
  {
    // Solve the normal equation X * n = y

    Matrix< FloatType, 2, 2> sumMatrixWS = sumMatrixWSZ.block<2, 2>(0, 0);
    Matrix< FloatType, 2, 2> invSumMatrixWS = sumMatrixWS.inverse();

    Matrix< FloatType, 2, 1> sumVectorZOverWS = sumMatrixWSZ.block<2, 1>(0, 2);
    Matrix< FloatType, 2, 1> nZOverWS = invSumMatrixWS * sumVectorZOverWS;

    FloatType z0Intercept = nZOverWS(0);
    FloatType szSlope = nZOverWS(1);

    SZCovariance szCovariance;

    szCovariance(iSZ, iSZ) = invSumMatrixWS(1, 1);
    szCovariance(iZ0, iSZ) = invSumMatrixWS(0, 1); // Should be symmetric.
    szCovariance(iSZ, iZ0) = invSumMatrixWS(1, 0); // Should be symmetric.
    szCovariance(iZ0, iZ0) = invSumMatrixWS(0, 0);

    FloatType chi2 = sumMatrixWSZ(2, 2) - z0Intercept * sumMatrixWSZ(0, 2) -  sumMatrixWSZ(1, 2) * szSlope;

    return UncertainSZLine(szSlope, z0Intercept, szCovariance, chi2);
  }


  // Declare function as currently unused to avoid compiler warning
  UncertainSZLine fitSZ(const Matrix< FloatType, 3, 3 >& sumMatrixWSZ) __attribute__((__unused__));

  /// Variant without drift circles and seperating the offset before the matrix solving
  UncertainSZLine fitSZ(const Matrix< FloatType, 3, 3 >& sumMatrixWSZ)
  {

    // Matrix of averages
    Matrix< FloatType, 3, 3> averageMatrixWSZ = sumMatrixWSZ / sumMatrixWSZ(0);

    // Measurement means
    Matrix< FloatType, 3, 1> meansWSZ = averageMatrixWSZ.row(0);

    // Covariance matrix
    Matrix< FloatType, 3, 3> covMatrixWSZ = averageMatrixWSZ - meansWSZ * meansWSZ.transpose();


    Matrix< FloatType, 2, 2> covMatrixSZ = covMatrixWSZ.block<2, 2>(1, 1);
    SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(covMatrixSZ);
    if (eigensolver.info() != Success) {
      B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    }

    // the eigenvalues are generated in increasing order
    // we are interested in the lowest one since we want to compute the normal vector of the line here

    Matrix<FloatType, 3, 1> nWSZ;
    nWSZ.middleRows<2>(1) = eigensolver.eigenvectors().col(0);
    nWSZ(0) = -meansWSZ.middleRows<2>(1).transpose() * nWSZ.middleRows<2>(1);

    Line2D szLine(nWSZ(0), nWSZ(1), nWSZ(2));


    Matrix< FloatType, 3, 3> covN = covMatrixWSZ.inverse();
    Matrix< FloatType, 2, 3> jacobianNToSlopeIntercept;

    jacobianNToSlopeIntercept(0, 0) = -1.0 / nWSZ(2);
    jacobianNToSlopeIntercept(1, 1) = -1.0 / nWSZ(2);

    jacobianNToSlopeIntercept(0, 2) = nWSZ(0) / nWSZ(2) / nWSZ(2);
    jacobianNToSlopeIntercept(1, 2) = nWSZ(1) / nWSZ(2) / nWSZ(2);

    Matrix<FloatType, 2, 2 > covInterceptSlope = jacobianNToSlopeIntercept * covN * jacobianNToSlopeIntercept.transpose();

    SZCovariance szCovariance;

    szCovariance(iSZ, iSZ) = covInterceptSlope(1, 1);
    szCovariance(iZ0, iSZ) = covInterceptSlope(0, 1); // Should be symmetric.
    szCovariance(iSZ, iZ0) = covInterceptSlope(1, 0); // Should be symmetric.
    szCovariance(iZ0, iZ0) = covInterceptSlope(0, 0);

    FloatType chi2 = nWSZ.transpose() * sumMatrixWSZ * nWSZ;

    return UncertainSZLine(szLine, szCovariance, chi2);

  }

}




void CDCSZFitter::update(const CDCAxialStereoSegmentPair& axialStereoSegmentPair) const
{
  const CDCStereoRecoSegment2D* ptrStereoSegment = axialStereoSegmentPair.getStereoSegment();
  const CDCAxialRecoSegment2D* ptrAxialSegment = axialStereoSegmentPair.getAxialSegment();

  if (not ptrStereoSegment or not ptrAxialSegment) return;

  const CDCStereoRecoSegment2D& stereoSegment = *ptrStereoSegment;
  const CDCAxialRecoSegment2D& axialSegment = *ptrAxialSegment;
  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  CDCTrajectorySZ trajectorySZ;
  trajectorySZ.clear();

  update(trajectorySZ, stereoSegment, axialTrajectory2D);

  CDCTrajectory3D trajectory3D(axialTrajectory2D, trajectorySZ);

  axialStereoSegmentPair.setTrajectory3D(trajectory3D);

}




void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         const CDCStereoRecoSegment2D& stereoSegment,
                         const CDCTrajectory2D& axialTrajectory2D) const
{
  //recostruct the stereo segment
  CDCObservations2D observationsSZ;
  for (const CDCRecoHit2D & recoHit2D : stereoSegment) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, axialTrajectory2D);
    appendSZ(observationsSZ, recoHit3D);
  }

  update(trajectorySZ, observationsSZ);

}





void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         CDCObservations2D& observations2D) const
{
  trajectorySZ.clear();
  if (observations2D.size() < 3) {
    B2WARNING("Skipping sz fit.");
    return;
  }

  // Matrix of weighted sums
  Matrix< FloatType, 3, 3 > sumMatrixWSZ = observations2D.getWXYSumMatrix();
  UncertainSZLine uncertainSZLine = fitZ(sumMatrixWSZ);

  // Determine NDF : Line fit eats up 2 degrees of freedom.
  size_t ndf = observations2D.size() - 2;
  uncertainSZLine.setNDF(ndf);

  trajectorySZ.setSZLine(uncertainSZLine);




}
