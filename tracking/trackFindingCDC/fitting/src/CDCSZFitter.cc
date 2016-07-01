/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <Eigen/Dense>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

using namespace Eigen;



const CDCSZFitter& CDCSZFitter::getFitter()
{
  static CDCSZFitter szFitter;
  return szFitter;
}



namespace {
  UncertainSZLine fitZ(const Matrix< double, 3, 3 >& sumMatrixWSZ)
  {
    // Solve the normal equation X * n = y

    Matrix< double, 2, 2> sumMatrixWS = sumMatrixWSZ.block<2, 2>(0, 0);
    Matrix< double, 2, 2> invSumMatrixWS = sumMatrixWS.inverse();

    Matrix< double, 2, 1> sumVectorZOverWS = sumMatrixWSZ.block<2, 1>(0, 2);
    Matrix< double, 2, 1> nZOverWS = invSumMatrixWS * sumVectorZOverWS;

    double z0Intercept = nZOverWS(0);
    double tanLambda = nZOverWS(1);

    SZCovariance szCovariance;

    using namespace NHelixParameter;
    szCovariance(c_TanL, c_TanL) = invSumMatrixWS(1, 1);
    szCovariance(c_Z0, c_TanL) = invSumMatrixWS(0, 1); // Should be symmetric.
    szCovariance(c_TanL, c_Z0) = invSumMatrixWS(1, 0); // Should be symmetric.
    szCovariance(c_Z0, c_Z0) = invSumMatrixWS(0, 0);

    double chi2 = sumMatrixWSZ(2, 2) - z0Intercept * sumMatrixWSZ(0, 2) -  sumMatrixWSZ(1, 2) * tanLambda;

    return UncertainSZLine(tanLambda, z0Intercept, szCovariance, chi2);
  }


  // Declare function as currently unused to avoid compiler warning
  UncertainSZLine fitSZ(const Matrix< double, 3, 3 >& sumMatrixWSZ) __attribute__((__unused__));

  /// Variant without drift circles and seperating the offset before the matrix solving
  UncertainSZLine fitSZ(const Matrix< double, 3, 3 >& sumMatrixWSZ)
  {

    // Matrix of averages
    Matrix< double, 3, 3> averageMatrixWSZ = sumMatrixWSZ / sumMatrixWSZ(0);

    // Measurement means
    Matrix< double, 3, 1> meansWSZ = averageMatrixWSZ.row(0);

    // Covariance matrix
    Matrix< double, 3, 3> covMatrixWSZ = averageMatrixWSZ - meansWSZ * meansWSZ.transpose();


    Matrix< double, 2, 2> covMatrixSZ = covMatrixWSZ.block<2, 2>(1, 1);
    SelfAdjointEigenSolver< Matrix<double, 2, 2> > eigensolver(covMatrixSZ);
    if (eigensolver.info() != Success) {
      B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    }

    // the eigenvalues are generated in increasing order
    // we are interested in the lowest one since we want to compute the normal vector of the line here

    Matrix<double, 3, 1> nWSZ;
    nWSZ.middleRows<2>(1) = eigensolver.eigenvectors().col(0);
    nWSZ(0) = -meansWSZ.middleRows<2>(1).transpose() * nWSZ.middleRows<2>(1);

    Line2D szLine(nWSZ(0), nWSZ(1), nWSZ(2));


    Matrix< double, 3, 3> covN = covMatrixWSZ.inverse();
    Matrix< double, 2, 3> jacobianNToSlopeIntercept;

    jacobianNToSlopeIntercept(0, 0) = -1.0 / nWSZ(2);
    jacobianNToSlopeIntercept(1, 1) = -1.0 / nWSZ(2);

    jacobianNToSlopeIntercept(0, 2) = nWSZ(0) / nWSZ(2) / nWSZ(2);
    jacobianNToSlopeIntercept(1, 2) = nWSZ(1) / nWSZ(2) / nWSZ(2);

    Matrix<double, 2, 2 > covInterceptSlope = jacobianNToSlopeIntercept * covN * jacobianNToSlopeIntercept.transpose();

    SZCovariance szCovariance;

    using namespace NHelixParameter;
    szCovariance(c_TanL, c_TanL) = covInterceptSlope(1, 1);
    szCovariance(c_Z0, c_TanL) = covInterceptSlope(0, 1); // Should be symmetric.
    szCovariance(c_TanL, c_Z0) = covInterceptSlope(1, 0); // Should be symmetric.
    szCovariance(c_Z0, c_Z0) = covInterceptSlope(0, 0);

    double chi2 = nWSZ.transpose() * sumMatrixWSZ * nWSZ;

    return UncertainSZLine(szLine, szCovariance, chi2);

  }

}

CDCTrajectorySZ CDCSZFitter::fitUsingSimplifiedTheilSen(const CDCRecoSegment3D& segment3D) const
{
  CDCTrajectorySZ trajectorySZ;
  CDCObservations2D observationsSZ;
  appendSZ(observationsSZ, segment3D);


  if (observationsSZ.size() > 4) {
    CDCObservations2D observationsSZFiltered;

    double meanTanLambda = 0;
    double meanStartZ = 0;

    for (unsigned int i = 0; i < observationsSZ.size(); i++) {
      for (unsigned int j = 0; j < observationsSZ.size(); j++) {
        if (i != j) {
          observationsSZFiltered.fill(observationsSZ.getX(j), observationsSZ.getY(j), observationsSZ.getDriftLength(j),
                                      observationsSZ.getWeight(j));
        }

      }

      update(trajectorySZ, observationsSZFiltered);
      meanTanLambda += trajectorySZ.getTanLambda();
      meanStartZ += trajectorySZ.getStartZ();
    }

    return CDCTrajectorySZ(meanTanLambda / observationsSZ.size(), meanStartZ / observationsSZ.size());
  } else {
    return CDCTrajectorySZ::basicAssumption();
  }
}

void CDCSZFitter::update(const CDCSegmentPair& segmentPair) const
{
  const CDCStereoRecoSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();
  const CDCAxialRecoSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();

  if (not ptrStereoSegment or not ptrAxialSegment) return;

  const CDCStereoRecoSegment2D& stereoSegment = *ptrStereoSegment;
  const CDCAxialRecoSegment2D& axialSegment = *ptrAxialSegment;
  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  CDCTrajectorySZ trajectorySZ;
  update(trajectorySZ, stereoSegment, axialTrajectory2D);

  CDCTrajectory3D trajectory3D(axialTrajectory2D, trajectorySZ);
  segmentPair.setTrajectory3D(trajectory3D);
}




void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         const CDCStereoRecoSegment2D& stereoSegment,
                         const CDCTrajectory2D& axialTrajectory2D) const
{
  //recostruct the stereo segment
  CDCObservations2D observationsSZ;
  for (const CDCRecoHit2D& recoHit2D : stereoSegment) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, axialTrajectory2D);
    appendSZ(observationsSZ, recoHit3D);
  }

  update(trajectorySZ, observationsSZ);

}

void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         CDCObservations2D& observations2D) const
{
  //trajectorySZ.clear();
  if (observations2D.size() < 3) {
    B2WARNING("Skipping sz fit.");
    return;
  }

  // Matrix of weighted sums
  Matrix< double, 3, 3 > sumMatrixWSZ = observations2D.getWXYSumMatrix();
  UncertainSZLine uncertainSZLine = fitZ(sumMatrixWSZ);

  // Determine NDF : Line fit eats up 2 degrees of freedom.
  size_t ndf = observations2D.size() - 2;
  uncertainSZLine.setNDF(ndf);

  trajectorySZ.setSZLine(uncertainSZLine);




}
