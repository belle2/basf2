/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


double dCurvOverDSZByISuperLayer[NSUPERLAYERS] = {0.0, 0.0027, 0.0, -0.0017, 0.0, 0.00116, 0.0, -0.000791};

double dPhi0OverDZ0ByISuperLayer[NSUPERLAYERS] = {0.0, -0.0023, 0.0, 0.0012, 0.0, -0.00097, 0.0, 0.00080};

namespace {

  const bool useResidualParameters = true;

  template<class ARecoHit, class ARecoHitSegment>
  TMatrixD calcAmbiguityImpl(const ARecoHitSegment& segment,
                             const CDCTrajectory2D& trajectory2D)
  {

    size_t nHits = segment.size();
    ISuperLayerType iSuperLayer = segment.getISuperLayer();

    double zeta = 0;

    const Vector2D& localOrigin2D = trajectory2D.getLocalOrigin();
    const UncertainPerigeeCircle& localCircle = trajectory2D.getLocalCircle();

    for (const ARecoHit& recoHit : segment) {
      const Vector2D& recoPos2D = recoHit.getRecoPos2D();
      const Vector2D localRecoPos2D = recoPos2D - localOrigin2D;
      //const Vector2D tangential = localCircle.tangential(localRecoPos2D);
      const Vector2D normal = localCircle.normal(localRecoPos2D);

      const Vector3D& wireVector = recoHit.getWire().getWireVector();
      zeta += wireVector.xy().dot(normal) / wireVector.z();

    }

    zeta /= nHits;

    TMatrixD result(3, 5);
    result.Zero();

    using namespace NHelixParameter;
    result(c_Curv, c_Curv) = 1.0;
    result(c_Phi0, c_Phi0) = 1.0;
    result(c_I, c_I)       = 1.0;

    // result(c_Curv,iSZ)    = -2.0 * dPhi0OverDZ0ByISuperLayer[iSuperLayer];
    result(c_Curv, c_TanL)   = dCurvOverDSZByISuperLayer[iSuperLayer];
    result(c_Phi0, c_Z0)   = -dPhi0OverDZ0ByISuperLayer[iSuperLayer];

    result(c_Phi0, c_TanL)   =   zeta;
    result(c_I, c_Z0)      = - zeta;

    // result.Print();

    return result;

  }

  template<class AStartRecoHit, class AEndRecoHit, class AStartRecoHitSegment, class AEndRecoHitSegment>
  CDCTrajectory3D fuseTrajectoriesImpl(const AStartRecoHitSegment& startSegment,
                                       const AEndRecoHitSegment& endSegment)
  {
    if (startSegment.empty()) {
      B2WARNING("Start segment is empty.");
      return CDCTrajectory3D();
    }

    if (endSegment.empty()) {
      B2WARNING("End segment is empty.");
      return CDCTrajectory3D();
    }

    AStartRecoHit lastHitOfStartSegment = startSegment.back();
    AEndRecoHit firstHitOfEndSegment = endSegment.front();

    Vector2D localOrigin2D = Vector2D::average(lastHitOfStartSegment.getRecoPos2D(),
                                               firstHitOfEndSegment.getRecoPos2D());

    Vector3D localOrigin3D(localOrigin2D, 0.0);

    if (not startSegment.getTrajectory2D().isFitted()) {
      // if (startSegment.size() > 3) B2WARNING("Start segment not fitted.");
      CDCTrajectory3D result;
      result.setChi2(NAN);
      return result;
    }

    if (not endSegment.getTrajectory2D().isFitted()) {
      // if (endSegment.size() > 3) B2WARNING("End segment not fitted.");
      CDCTrajectory3D result;
      result.setChi2(NAN);
      return result;
    }

    CDCTrajectory2D startTrajectory2D = startSegment.getTrajectory2D();
    CDCTrajectory2D endTrajectory2D = endSegment.getTrajectory2D();

    // Propagate to a common point
    startTrajectory2D.setLocalOrigin(localOrigin2D);
    endTrajectory2D.setLocalOrigin(localOrigin2D);

    const UncertainPerigeeCircle& startCircle = startTrajectory2D.getLocalCircle();
    const UncertainPerigeeCircle& endCircle = endTrajectory2D.getLocalCircle();

    TMatrixD startH = calcAmbiguityImpl<AStartRecoHit>(startSegment, startTrajectory2D);
    TMatrixD endH = calcAmbiguityImpl<AEndRecoHit>(endSegment, endTrajectory2D);

    UncertainHelix resultHelix = CDCAxialStereoFusion::fuse(startCircle, startH, endCircle, endH);
    return CDCTrajectory3D(localOrigin3D, resultHelix);
  }
}


double CDCAxialStereoFusion::average(const TVectorD& startParameters,
                                     const TMatrixDSym& startCovMatrix,
                                     const TVectorD& endParameters,
                                     const TMatrixDSym& endCovMatrix,
                                     TVectorD& avgParameters,
                                     TMatrixDSym& avgCovMatrix)
{
  TMatrixDSym startInvCovMatrix = startCovMatrix;
  startInvCovMatrix.Invert();

  TMatrixDSym endInvCovMatrix = endCovMatrix;
  endInvCovMatrix.Invert();

  avgCovMatrix = startInvCovMatrix + endInvCovMatrix;
  avgCovMatrix.Invert();

  TVectorD weightedSumParameter = startInvCovMatrix * startParameters + endInvCovMatrix * endParameters;

  avgParameters = avgCovMatrix * weightedSumParameter;
  double chi2 = startInvCovMatrix.Similarity(startParameters) + endInvCovMatrix.Similarity(endParameters);
  return chi2;
}


double CDCAxialStereoFusion::average(const TVectorD& startParameters,
                                     const TMatrixDSym& startCovMatrix,
                                     const TMatrixD& startAmbiguityMatrix,
                                     const TVectorD& endParameters,
                                     const TMatrixDSym& endCovMatrix,
                                     const TMatrixD& endAmbiguityMatrix,
                                     TVectorD& avgParameters,
                                     TMatrixDSym& avgCovMatrix)
{
  TMatrixD startAmbiguityMatrixTransposed = startAmbiguityMatrix;
  startAmbiguityMatrixTransposed.T();

  TMatrixD endAmbiguityMatrixTransposed = endAmbiguityMatrix;
  endAmbiguityMatrixTransposed.T();


  TMatrixDSym startInvCovMatrix = startCovMatrix;
  startInvCovMatrix.Invert();

  TMatrixDSym endInvCovMatrix = endCovMatrix;
  endInvCovMatrix.Invert();


  TMatrixDSym startInflatedInvCovMatrix = startInvCovMatrix;
  startInflatedInvCovMatrix.SimilarityT(startAmbiguityMatrix);

  TMatrixDSym endInflatedInvCovMatrix = endInvCovMatrix;
  endInflatedInvCovMatrix.SimilarityT(endAmbiguityMatrix);

  avgCovMatrix = startInflatedInvCovMatrix + endInflatedInvCovMatrix;
  avgCovMatrix.Invert();


  TVectorD weightedSum =
    startAmbiguityMatrixTransposed * (startInvCovMatrix * startParameters) +
    endAmbiguityMatrixTransposed * (endInvCovMatrix * endParameters);

  avgParameters = avgCovMatrix * weightedSum;

  TVectorD startPosteriorParameters = startAmbiguityMatrix * avgParameters;
  TVectorD endPosteriorParameters = endAmbiguityMatrix * avgParameters;

  TVectorD startResidual = startParameters - startPosteriorParameters;
  TVectorD endResidual = endParameters - endPosteriorParameters;

  double startChi2 = startInvCovMatrix.Similarity(startResidual);
  double endChi2 = endInvCovMatrix.Similarity(endResidual);

  double chi2 =  startChi2 + endChi2;
  return chi2;
}


UncertainPerigeeCircle CDCAxialStereoFusion::fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                                  const UncertainPerigeeCircle& endPerigeeCircle)
{
  TVectorD startParameters = startPerigeeCircle.parameters();
  TMatrixDSym startCovMatrix = startPerigeeCircle.perigeeCovariance();

  TVectorD endParameters = endPerigeeCircle.parameters();
  TMatrixDSym endCovMatrix = endPerigeeCircle.perigeeCovariance();

  TMatrixDSym avgCovMatrix(3);
  avgCovMatrix.Zero();

  TVectorD avgParameters(3);
  avgParameters.Zero();

  double chi2 = average(startParameters,
                        startCovMatrix,
                        endParameters,
                        endCovMatrix,
                        avgParameters,
                        avgCovMatrix);

  // Calculating 3 parameters from 6 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainPerigeeCircle(avgParameters, PerigeeCovariance(avgCovMatrix), chi2, ndf);
}


UncertainHelix CDCAxialStereoFusion::fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                          const TMatrixD& startAmbiguityMatrix,
                                          const UncertainPerigeeCircle& endPerigeeCircle,
                                          const TMatrixD& endAmbiguityMatrix)
{
  TVectorD startParameters = startPerigeeCircle.parameters();
  TMatrixDSym startCovMatrix = startPerigeeCircle.perigeeCovariance();

  TVectorD endParameters = endPerigeeCircle.parameters();
  TMatrixDSym endCovMatrix = endPerigeeCircle.perigeeCovariance();

  // Helix covariance
  TMatrixDSym avgCovMatrix(5);
  avgCovMatrix.Zero();

  // Helix parameters
  TVectorD avgParameters(5);
  avgParameters.Zero();

  // Calculating 5 parameters from 6 input parameters. 1 NDF remaining.
  size_t ndf = 1;

  double chi2 = 0;

  if (useResidualParameters) {
    // Use the mean circle parameters as the reference, since the ambiguity matrix is a expansion around that point.
    TVectorD refParameters = startParameters + endParameters;
    refParameters *= 0.5;

    startParameters -= refParameters;
    endParameters -= refParameters;

    // Chi2 value
    chi2 = average(startParameters,
                   startCovMatrix,
                   startAmbiguityMatrix,
                   endParameters,
                   endCovMatrix,
                   endAmbiguityMatrix,
                   avgParameters,
                   avgCovMatrix);

    using namespace NPerigeeParameter;
    avgParameters(c_Curv) += refParameters(c_Curv);
    avgParameters(c_Phi0) += refParameters(c_Phi0);
    avgParameters(c_I) += refParameters(c_I);

  } else {
    // Chi2 value
    chi2 = average(startParameters,
                   startCovMatrix,
                   startAmbiguityMatrix,
                   endParameters,
                   endCovMatrix,
                   endAmbiguityMatrix,
                   avgParameters,
                   avgCovMatrix);
  }

  return UncertainHelix(avgParameters, HelixCovariance(avgCovMatrix), chi2, ndf);
}

UncertainHelix CDCAxialStereoFusion::fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                          const TMatrixD& startAmbiguityMatrix,
                                          const UncertainHelix& endHelix)
{
  TVectorD startParameters = startPerigeeCircle.parameters();
  TMatrixDSym startCovMatrix = startPerigeeCircle.perigeeCovariance();

  TVectorD endParameters = endHelix.parameters();
  TMatrixDSym endCovMatrix = endHelix.helixCovariance();
  TMatrixD endAmbiguityMatrix(5, 5);
  endAmbiguityMatrix.UnitMatrix();

  // Helix covariance
  TMatrixDSym avgCovMatrix(5);
  avgCovMatrix.Zero();

  // Helix parameters
  TVectorD avgParameters(5);
  avgParameters.Zero();

  // Calculating 5 parameters from 8 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  double chi2 = 0;


  if (useResidualParameters) {
    // Use the circle parameters as the reference, since the ambiguity matrix is a expansion around that point.
    TVectorD refParameters = startParameters;
    refParameters *= 0.5;

    startParameters -= refParameters;

    // Only first three coordinates are effected by a change of the reference (expansion) point
    using namespace NPerigeeParameter;
    endParameters(c_Curv) -= refParameters(c_Curv);
    endParameters(c_Phi0) -= refParameters(c_Phi0);
    endParameters(c_I) -= refParameters(c_I);

    // Chi2 value
    chi2 = average(startParameters,
                   startCovMatrix,
                   startAmbiguityMatrix,
                   endParameters,
                   endCovMatrix,
                   endAmbiguityMatrix,
                   avgParameters,
                   avgCovMatrix);

    using namespace NPerigeeParameter;
    avgParameters(c_Curv) += refParameters(c_Curv);
    avgParameters(c_Phi0) += refParameters(c_Phi0);
    avgParameters(c_I) += refParameters(c_I);

  } else {
    // Chi2 value
    chi2 = average(startParameters,
                   startCovMatrix,
                   startAmbiguityMatrix,
                   endParameters,
                   endCovMatrix,
                   endAmbiguityMatrix,
                   avgParameters,
                   avgCovMatrix);
  }

  return UncertainHelix(avgParameters, HelixCovariance(avgCovMatrix), chi2, ndf);
}

UncertainHelix CDCAxialStereoFusion::fuse(const UncertainHelix& startHelix,
                                          const UncertainHelix& endHelix)
{
  TVectorD startParameters = startHelix.parameters();
  TMatrixDSym startCovMatrix = startHelix.helixCovariance();

  TVectorD endParameters = endHelix.parameters();
  TMatrixDSym endCovMatrix = endHelix.helixCovariance();

  TMatrixDSym avgCovMatrix(5);
  avgCovMatrix.Zero();

  TVectorD avgParameters(5);
  avgParameters.Zero();

  double chi2 = average(startParameters,
                        startCovMatrix,
                        endParameters,
                        endCovMatrix,
                        avgParameters,
                        avgCovMatrix);

  // Calculating 5 parameters from 10 input parameters. 5 NDF remaining.
  size_t ndf = 5;

  return UncertainHelix(avgParameters, HelixCovariance(avgCovMatrix), chi2, ndf);
}



TMatrixD CDCAxialStereoFusion::calcAmbiguity(const CDCRecoSegment2D& recoSegment2D,
                                             const CDCTrajectory2D& trajectory2D)
{
  return calcAmbiguityImpl<CDCRecoHit2D>(recoSegment2D, trajectory2D);
}



TMatrixD CDCAxialStereoFusion::calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                             const CDCTrajectory2D& trajectory2D)
{
  return calcAmbiguityImpl<CDCRecoHit3D>(recoSegment3D, trajectory2D);
}




CDCTrajectory3D CDCAxialStereoFusion::fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                                       const CDCRecoSegment2D& endSegment)
{
  return fuseTrajectoriesImpl<CDCRecoHit2D, CDCRecoHit2D>(startSegment, endSegment);
}



void CDCAxialStereoFusion::fuseTrajectories(const CDCSegmentPair& segmentPair)
{
  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  if (not ptrStartSegment) {
    B2WARNING("Start segment unset.");
    return;
  }

  if (not ptrEndSegment) {
    B2WARNING("End segment unset.");
    return;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  CDCTrajectory3D trajectory3D = fuseTrajectories(startSegment, endSegment);
  segmentPair.setTrajectory3D(trajectory3D);



}


CDCTrajectory3D CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCRecoSegment2D& startSegment,
    const CDCRecoSegment2D& endSegment,
    bool priorityOnSZ)
{

  if (startSegment.empty()) {
    B2WARNING("Start segment is empty.");
    return CDCTrajectory3D();
  }

  if (endSegment.empty()) {
    B2WARNING("End segment is empty.");
    return CDCTrajectory3D();
  }

  bool startIsAxial = startSegment.getStereoKind() == EStereoKind::c_Axial;

  const CDCAxialRecoSegment2D& axialSegment = startIsAxial ? startSegment : endSegment;
  const CDCStereoRecoSegment2D& stereoSegment = not startIsAxial ? startSegment : endSegment;

  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  // const Vector2D& localOrigin2D = (startIsAxial ? startSegment.back() : endSegment.front()).getRecoPos2D();
  // axialTrajectory2D.setLocalOrigin(localOrigin2D);

  CDCRecoSegment3D stereoSegment3D = CDCRecoSegment3D::reconstruct(stereoSegment, axialTrajectory2D);


  CDCTrajectorySZ trajectorySZ;

  const bool mcTruthReference = false;
  if (mcTruthReference) {
    const CDCMCSegmentLookUp& theMCSegmentLookUp = CDCMCSegmentLookUp::getInstance();
    CDCTrajectory3D axialMCTrajectory3D = theMCSegmentLookUp.getTrajectory3D(&axialSegment);
    Vector3D localOrigin3D =  axialMCTrajectory3D.getLocalOrigin();
    localOrigin3D.setXY(axialTrajectory2D.getLocalOrigin());
    axialMCTrajectory3D.setLocalOrigin(localOrigin3D);
    trajectorySZ = axialMCTrajectory3D.getTrajectorySZ();

  } else {
    CDCSZFitter szFitter = CDCSZFitter::getFitter();
    trajectorySZ = szFitter.fit(stereoSegment3D);

  }

  CDCRiemannFitter riemannFitter;
  // riemannFitter.useOnlyOrientation();
  riemannFitter.useOnlyPosition();

  if (priorityOnSZ) {
    // To reconstructed point in the three dimensional stereo segment all lie exactly on the circle they are reconstructed onto.
    // This part draws them away from the circle onto the sz trajectory instead leaving all the residuals visible in the xy projection.
    // Hence the two dimensional fit, which is used for the fusion afterwards can react to residuals and render the covariances of the stereo segment broader.
    for (CDCRecoHit3D& recoHit3D : stereoSegment3D) {
      const CDCWire& wire = recoHit3D.getWire();

      const double oldZ = recoHit3D.getRecoZ();

      const double s = recoHit3D.getArcLength2D();
      const double newZ = trajectorySZ.mapSToZ(s);

      Vector3D recoPos3DCorrection = wire.getWireVector();
      recoPos3DCorrection *= (newZ - oldZ) / wire.getWireVector().z();

      Vector3D correctedRecoPos3D = recoHit3D.getRecoPos3D() + recoPos3DCorrection;

      const double correctedPerpS = axialTrajectory2D.calcArcLength2D(correctedRecoPos3D.xy());

      recoHit3D.setRecoPos3D(correctedRecoPos3D);
      recoHit3D.setArcLength2D(correctedPerpS);
    }
  }

  CDCTrajectory2D stereoTrajectory2D = riemannFitter.fit(stereoSegment3D);
  stereoSegment3D.setTrajectory2D(stereoTrajectory2D);

  // Correct the values of tanLambda and z0
  CDCTrajectory3D fusedTrajectory3D =
    startIsAxial ?
    fuseTrajectoriesImpl<CDCRecoHit2D, CDCRecoHit3D>(startSegment, stereoSegment3D) :
    fuseTrajectoriesImpl<CDCRecoHit3D, CDCRecoHit2D>(stereoSegment3D, endSegment) ;

  double tanLambdaShift = trajectorySZ.getTanLambda();

  Vector3D fusedLocalOrigin = fusedTrajectory3D.getLocalOrigin();
  double sOffset = axialTrajectory2D.calcArcLength2D(fusedLocalOrigin.xy());
  double zShift = trajectorySZ.mapSToZ(sOffset);

  fusedTrajectory3D.shiftTanLambdaIntercept(tanLambdaShift, zShift);

  return fusedTrajectory3D;

}


void CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCSegmentPair& segmentPair,
                                                       bool priorityOnSZ)
{
  const CDCRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

  if (not ptrStartSegment) {
    B2WARNING("Start segment unset.");
    return;
  }

  if (not ptrEndSegment) {
    B2WARNING("End segment unset.");
    return;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  CDCTrajectory3D trajectory3D = reconstructFuseTrajectories(startSegment, endSegment, priorityOnSZ);
  segmentPair.setTrajectory3D(trajectory3D);
}
