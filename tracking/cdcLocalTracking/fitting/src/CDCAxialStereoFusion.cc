/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCAxialStereoFusion.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


TMatrixD CDCLocalTracking::calcAmbiguity(const CDCRecoSegment2D& segment,
                                         const CDCTrajectory2D& trajectory2D)
{

  size_t nHits = segment.size();

  FloatType zeta = 0;

  const Vector2D& localOrigin2D = trajectory2D.getLocalOrigin();
  const UncertainPerigeeCircle& localCircle = trajectory2D.getLocalCircle();

  for (const CDCRecoHit2D & recoHit2D : segment) {
    const Vector2D& recoPos2D = recoHit2D.getRecoPos2D();
    const Vector2D localRecoPos2D = recoPos2D - localOrigin2D;
    //const Vector2D tangential = localCircle.tangential(localRecoPos2D);
    const Vector2D normal = localCircle.normal(localRecoPos2D);

    const Vector3D& wireVector = recoHit2D.getWire().getWireVector();

    // Check sign
    zeta += wireVector.xy().dot(normal) / wireVector.z();

  }

  zeta /= nHits;

  TMatrixD result(3, 5);
  result.Zero();

  result(iCurv, iCurv) = 1.0;
  result(iPhi0, iPhi0) = 1.0;
  result(iI, iI)       = 1.0;

  result(iPhi0, iSZ)   = zeta;
  result(iI, iZ0)      = -zeta;

  return result;

}




CDCTrajectory3D CDCLocalTracking::fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                                   const CDCRecoSegment2D& endSegment)
{

  if (startSegment.empty()) {
    B2WARNING("Start segment is empty.");
    return CDCTrajectory3D();
  }

  if (endSegment.empty()) {
    B2WARNING("End segment is empty.");
    return CDCTrajectory3D();
  }

  CDCRecoHit2D lastHitOfStartSegment = startSegment.back();
  CDCRecoHit2D firstHitOfEndSegment = endSegment.front();

  Vector2D localOrigin2D = Vector2D::average(lastHitOfStartSegment.getRecoPos2D(),
                                             firstHitOfEndSegment.getRecoPos2D());

  Vector3D localOrigin3D(localOrigin2D, 0.0);


  if (not startSegment.getTrajectory2D().isFitted()) {
    B2WARNING("Start segment not fitted.");
    return CDCTrajectory3D();
  }

  if (not endSegment.getTrajectory2D().isFitted()) {
    B2WARNING("End segment not fitted.");
    return CDCTrajectory3D();
  }

  CDCTrajectory2D startTrajectory2D = startSegment.getTrajectory2D();
  CDCTrajectory2D endTrajectory2D = endSegment.getTrajectory2D();

  // Propagate to a common point
  startTrajectory2D.setLocalOrigin(localOrigin2D);
  endTrajectory2D.setLocalOrigin(localOrigin2D);

  const UncertainPerigeeCircle& startCircle = startTrajectory2D.getLocalCircle();
  const UncertainPerigeeCircle& endCircle = endTrajectory2D.getLocalCircle();

  TVectorD startParameters = startCircle.parameters();
  TVectorD endParameters = endCircle.parameters();

  TMatrixDSym startCovMatrix = startCircle.perigeeCovariance();
  TMatrixDSym endCovMatrix = endCircle.perigeeCovariance();



  TMatrixD startH = calcAmbiguity(startSegment, startTrajectory2D);
  TMatrixD endH = calcAmbiguity(endSegment, endTrajectory2D);



  TMatrixD startHTransposed = startH;
  startHTransposed.T();

  TMatrixD endHTransposed = endH;
  endHTransposed.T();



  TMatrixDSym startInvCovMatrix = startCovMatrix;
  startInvCovMatrix.Invert();

  TMatrixDSym endInvCovMatrix = endCovMatrix;
  endInvCovMatrix.Invert();



  TMatrixDSym startInvHelixCovMatrix = startInvCovMatrix;
  startInvHelixCovMatrix.SimilarityT(startH);

  TMatrixDSym endInvHelixCovMatrix = endInvCovMatrix;
  endInvHelixCovMatrix.SimilarityT(endH);

  TMatrixDSym helixCovMatrix = startInvHelixCovMatrix + endInvHelixCovMatrix;
  helixCovMatrix.Invert();


  TVectorD weightedSum =
    startHTransposed * (startInvCovMatrix * startParameters) +
    endHTransposed * (endInvCovMatrix * endParameters);

  TVectorD helixParameters = helixCovMatrix * weightedSum;

  UncertainHelix resultHelix(helixParameters, HelixCovariance(helixCovMatrix));



  TVectorD startPosteriorParameters = startH * helixParameters;
  TVectorD endPosteriorParameters = endH * helixParameters;



  TVectorD startResidual = startParameters - startPosteriorParameters;
  TVectorD endResidual = endParameters - endPosteriorParameters;



  Double_t startChi2 = startInvCovMatrix.Similarity(startResidual);
  Double_t endChi2 = endInvCovMatrix.Similarity(endResidual);



  Double_t chi2 =  startChi2 + endChi2;

  resultHelix.setChi2(chi2);

  return CDCTrajectory3D(localOrigin3D, resultHelix);

}



void CDCLocalTracking::fuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
{
  const CDCRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

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
  axialStereoSegmentPair.setTrajectory3D(trajectory3D);

}

