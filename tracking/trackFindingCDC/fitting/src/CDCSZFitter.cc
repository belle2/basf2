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

#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <framework/logging/Logger.h>

#include <Eigen/Dense>

using namespace Belle2;
using namespace TrackFindingCDC;

using namespace Eigen;

const CDCSZFitter& CDCSZFitter::getFitter()
{
  static CDCSZFitter szFitter;
  return szFitter;
}

namespace {
  UncertainSZLine fitZ(const Matrix<double, 3, 3>& sumMatrixWSZ)
  {
    // Solve the normal equation X * n = y
    Matrix<double, 2, 2> sumMatrixWS = sumMatrixWSZ.block<2, 2>(0, 0);
    Matrix<double, 2, 1> sumVectorZOverWS = sumMatrixWSZ.block<2, 1>(0, 2);
    Matrix<double, 2, 1> nZOverWS = sumMatrixWS.llt().solve(sumVectorZOverWS);
    double chi2 = sumMatrixWSZ(2, 2) - nZOverWS.transpose() * sumVectorZOverWS;

    using namespace NSZParameterIndices;
    SZParameters szParameters;
    szParameters(c_TanL) = nZOverWS(1);
    szParameters(c_Z0) = nZOverWS(0);

    SZPrecision szPrecision;
    szPrecision(c_TanL, c_TanL) = sumMatrixWS(1, 1);
    szPrecision(c_Z0, c_TanL) = sumMatrixWS(0, 1);
    szPrecision(c_TanL, c_Z0) = sumMatrixWS(1, 0);
    szPrecision(c_Z0, c_Z0) = sumMatrixWS(0, 0);

    SZCovariance szCovariance = szPrecision.inverse();
    return UncertainSZLine(szParameters, szCovariance, chi2);
  }

  // Declare function as currently unused to avoid compiler warning
  UncertainSZLine fitSZ(const Matrix<double, 3, 3>& sumMatrixWSZ) __attribute__((__unused__));

  /// Variant without drift circles and seperating the offset before the matrix solving
  UncertainSZLine fitSZ(const Matrix<double, 3, 3>& sumMatrixWSZ)
  {

    // Matrix of averages
    Matrix<double, 3, 3> averageMatrixWSZ = sumMatrixWSZ / sumMatrixWSZ(0);

    // Measurement means
    Matrix<double, 3, 1> meansWSZ = averageMatrixWSZ.row(0);

    // Covariance matrix
    Matrix<double, 3, 3> covMatrixWSZ = averageMatrixWSZ - meansWSZ * meansWSZ.transpose();

    Matrix<double, 2, 2> covMatrixSZ = covMatrixWSZ.block<2, 2>(1, 1);
    Matrix<double, 2, 1> meansSZ = meansWSZ.segment<2>(1);

    SelfAdjointEigenSolver<Matrix<double, 2, 2>> eigensolver(covMatrixSZ);
    if (eigensolver.info() != Success) {
      B2WARNING(
        "SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");
    }

    // the eigenvalues are generated in increasing order
    // we are interested in the lowest one since we want to compute the normal vector of the line
    // here
    Matrix<double, 2, 1> nSZ = eigensolver.eigenvectors().col(0);
    B2INFO("nSZ " << nSZ);
    nSZ /= std::copysign(nSZ.norm(), -nSZ(1)); // Making n2 negative to normalize to forward along s
    Matrix<double, 3, 1> nWSZ;
    nWSZ << -meansSZ.transpose() * nSZ, nSZ;
    B2INFO("nWSZ " << nWSZ);

    double chi2 = nWSZ.transpose() * sumMatrixWSZ * nWSZ;

    PrecisionMatrix<3> precN = sumMatrixWSZ;

    using namespace NSZParameterIndices;
    SZParameters szParameters;
    szParameters(c_TanL) = -nWSZ(1) / nWSZ(2);
    szParameters(c_Z0) = -nWSZ(0) / nWSZ(2);

    Matrix<double, 3, 2> ambiguitySZToN = Matrix<double, 3, 2>::Zero();
    ambiguitySZToN(0, c_Z0) = -nWSZ(2);
    ambiguitySZToN(0, c_TanL) = -nWSZ(1) * nWSZ(0);
    ambiguitySZToN(1, c_TanL) = -nWSZ(2) * nWSZ(2) * nWSZ(2);
    ambiguitySZToN(2, c_TanL) = nWSZ(1) * nWSZ(2) * nWSZ(2);

    SZPrecision szPrecision = ambiguitySZToN.transpose() * precN * ambiguitySZToN;

    SZCovariance szCovariance = szPrecision.inverse();
    return UncertainSZLine(szParameters, szCovariance, chi2);
  }
}

CDCTrajectorySZ CDCSZFitter::fitWithStereoHits(const CDCTrack& track) const
{
  const bool onlyStereo = true;
  CDCSZObservations observationsSZ(EFitVariance::c_Proper, onlyStereo);
  observationsSZ.appendRange(track);
  if (observationsSZ.size() > 3) {
    CDCTrajectorySZ szTrajectory;
    update(szTrajectory, observationsSZ);
    return szTrajectory;
  } else {
    return CDCTrajectorySZ::basicAssumption();
  }
}

CDCTrajectorySZ CDCSZFitter::fit(const CDCSegment2D& stereoSegment,
                                 const CDCTrajectory2D& axialTrajectory2D) const
{
  B2ASSERT("Expected stereo segment", not stereoSegment.isAxial());

  CDCTrajectorySZ trajectorySZ;
  update(trajectorySZ, stereoSegment, axialTrajectory2D);
  return trajectorySZ;
}

CDCTrajectorySZ CDCSZFitter::fit(const CDCSegment3D& segment3D) const
{
  CDCSZObservations observationsSZ;
  observationsSZ.appendRange(segment3D);
  return fit(std::move(observationsSZ));
}

CDCTrajectorySZ CDCSZFitter::fit(CDCSZObservations observationsSZ) const
{
  CDCTrajectorySZ trajectorySZ;
  update(trajectorySZ, observationsSZ);
  return trajectorySZ;
}

CDCTrajectorySZ CDCSZFitter::fitUsingSimplifiedTheilSen(const CDCSegment3D& segment3D) const
{
  CDCSZObservations observationsSZ;
  observationsSZ.appendRange(segment3D);

  CDCTrajectorySZ trajectorySZ;
  if (observationsSZ.size() > 4) {
    CDCSZObservations observationsSZFiltered;

    double meanTanLambda = 0;
    double meanStartZ = 0;

    for (unsigned int i = 0; i < observationsSZ.size(); i++) {
      for (unsigned int j = 0; j < observationsSZ.size(); j++) {
        if (i != j) {
          observationsSZFiltered.fill(observationsSZ.getS(j),
                                      observationsSZ.getZ(j),
                                      observationsSZ.getWeight(j));
        }
      } // for j

      update(trajectorySZ, observationsSZFiltered);
      meanTanLambda += trajectorySZ.getTanLambda();
      meanStartZ += trajectorySZ.getZ0();
    } // for i

    return CDCTrajectorySZ(meanTanLambda / observationsSZ.size(),
                           meanStartZ / observationsSZ.size());
  } else {
    return CDCTrajectorySZ::basicAssumption();
  }
}

void CDCSZFitter::update(const CDCSegmentPair& segmentPair) const
{
  const CDCSegment2D* ptrStereoSegment = segmentPair.getStereoSegment();
  const CDCSegment2D* ptrAxialSegment = segmentPair.getAxialSegment();

  assert(ptrStereoSegment);
  assert(ptrAxialSegment);

  const CDCSegment2D& stereoSegment = *ptrStereoSegment;
  const CDCSegment2D& axialSegment = *ptrAxialSegment;
  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  CDCTrajectorySZ trajectorySZ;
  update(trajectorySZ, stereoSegment, axialTrajectory2D);

  CDCTrajectory3D trajectory3D(axialTrajectory2D, trajectorySZ);
  segmentPair.setTrajectory3D(trajectory3D);
}

CDCTrajectorySZ CDCSZFitter::fit(const CDCObservations2D& observations2D) const
{
  CDCSZObservations szObservations;
  for (size_t i = 0; i < observations2D.size(); ++i) {
    const double s = observations2D.getX(i);
    const double z = observations2D.getY(i);
    szObservations.fill(s, z);
  }
  return fit(std::move(szObservations));
}

void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ,
                         const CDCSegment2D& stereoSegment,
                         const CDCTrajectory2D& axialTrajectory2D) const
{
  B2ASSERT("Expected stereo segment", not stereoSegment.isAxial());

  // recostruct the stereo segment
  CDCSZObservations observationsSZ;
  for (const CDCRecoHit2D& recoHit2D : stereoSegment) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, axialTrajectory2D);
    observationsSZ.append(recoHit3D);
  }

  update(trajectorySZ, observationsSZ);
}

void CDCSZFitter::update(CDCTrajectorySZ& trajectorySZ, CDCSZObservations& observationsSZ) const
{
  trajectorySZ.clear();
  if (observationsSZ.size() < 3) {
    return;
  }

  // Determine NDF : Line fit eats up 2 degrees of freedom.
  size_t ndf = observationsSZ.size() - 2;

  // Matrix of weighted sums
  Matrix<double, 3, 3> sumMatrixWSZ = observationsSZ.getWSZSumMatrix();
  UncertainSZLine uncertainSZLine = fitZ(sumMatrixWSZ);

  uncertainSZLine.setNDF(ndf);
  trajectorySZ.setSZLine(uncertainSZLine);
}
