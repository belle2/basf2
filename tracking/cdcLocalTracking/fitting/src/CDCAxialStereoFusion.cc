/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCSZFitter.h"
#include "../include/CDCRiemannFitter.h"

#include "../include/CDCAxialStereoFusion.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


FloatType dCurvOverDSZByISuperLayer[NSUPERLAYERS] = {0.0, 0.0027, 0.0, -0.0017, 0.0, 0.00116, 0.0, -0.000791};

FloatType dPhi0OverDZ0ByISuperLayer[NSUPERLAYERS] = {0.0, -0.0023, 0.0, 0.0012, 0.0, -0.00097, 0.0, 0.00080};

namespace {

  template<class RecoHit, class RecoHitSegment>
  TMatrixD calcAmbiguityImpl(const RecoHitSegment& segment,
                             const CDCTrajectory2D& trajectory2D)
  {

    size_t nHits = segment.size();
    ISuperLayerType iSuperLayer = segment.getISuperLayer();

    FloatType zeta = 0;

    const Vector2D& localOrigin2D = trajectory2D.getLocalOrigin();
    const UncertainPerigeeCircle& localCircle = trajectory2D.getLocalCircle();

    for (const RecoHit & recoHit : segment) {
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

    result(iCurv, iCurv) = 1.0;
    result(iPhi0, iPhi0) = 1.0;
    result(iI, iI)       = 1.0;

    // result(iCurv,iSZ)    = -2.0 * dPhi0OverDZ0ByISuperLayer[iSuperLayer];
    result(iCurv, iSZ)   = dCurvOverDSZByISuperLayer[iSuperLayer];
    result(iPhi0, iZ0)   = -dPhi0OverDZ0ByISuperLayer[iSuperLayer];

    result(iPhi0, iSZ)   =   zeta;
    result(iI, iZ0)      = - zeta;

    // result.Print();

    return result;

  }



  template<class StartRecoHit, class EndRecoHit, class StartRecoHitSegment, class EndRecoHitSegment>
  CDCTrajectory3D fuseTrajectoriesImpl(const StartRecoHitSegment& startSegment,
                                       const EndRecoHitSegment& endSegment)
  {

    if (startSegment.empty()) {
      B2WARNING("Start segment is empty.");
      return CDCTrajectory3D();
    }

    if (endSegment.empty()) {
      B2WARNING("End segment is empty.");
      return CDCTrajectory3D();
    }

    StartRecoHit lastHitOfStartSegment = startSegment.back();
    EndRecoHit firstHitOfEndSegment = endSegment.front();

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



    TMatrixD startH = calcAmbiguityImpl<StartRecoHit>(startSegment, startTrajectory2D);
    TMatrixD endH = calcAmbiguityImpl<EndRecoHit>(endSegment, endTrajectory2D);



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



void CDCAxialStereoFusion::fuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
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





CDCTrajectory3D CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCRecoSegment2D& startSegment,
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

  bool startIsAxial = startSegment.getStereoType() == AXIAL;

  const CDCAxialRecoSegment2D& axialSegment = startIsAxial ? startSegment : endSegment;
  const CDCStereoRecoSegment2D& stereoSegment = not startIsAxial ? startSegment : endSegment;

  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  // const Vector2D& localOrigin2D = (startIsAxial ? startSegment.back() : endSegment.front()).getRecoPos2D();
  // axialTrajectory2D.setLocalOrigin(localOrigin2D);

  CDCRecoSegment3D stereoSegment3D = CDCRecoSegment3D::reconstruct(stereoSegment, axialTrajectory2D);

  CDCSZFitter szFitter = CDCSZFitter::getFitter();
  CDCTrajectorySZ trajectorySZ = szFitter.fit(stereoSegment3D);

  CDCRecoSegment3D zPriorityStereoSegment3D;
  for (const CDCRecoHit3D & recoHit3D : stereoSegment3D) {
    const FloatType& s = recoHit3D.getPerpS();
    const FloatType z = trajectorySZ.mapSToZ(s);

    const CDCWire& wire = recoHit3D.getWire();
    Vector2D wirePos2DAtZ = wire.getWirePos2DAtZ(z);

    Vector3D recoPos3D(axialTrajectory2D.getClosest(wirePos2DAtZ), z);
    const FloatType sCorrected = axialTrajectory2D.calcPerpS(recoPos3D.xy());

    zPriorityStereoSegment3D.push_back(CDCRecoHit3D(&(recoHit3D.getRLWireHit()), recoPos3D, sCorrected));
  }

  CDCRiemannFitter riemannFitter;
  // riemannFitter.useOnlyOrientation();
  riemannFitter.useOnlyPosition();

  CDCTrajectory2D stereoTrajectory2D = riemannFitter.fit(zPriorityStereoSegment3D);

  zPriorityStereoSegment3D.setTrajectory2D(stereoTrajectory2D);

  CDCTrajectory3D fusedTrajectory3D =
    startIsAxial ?
    fuseTrajectoriesImpl<CDCRecoHit2D, CDCRecoHit3D>(startSegment, zPriorityStereoSegment3D) :
    fuseTrajectoriesImpl<CDCRecoHit3D, CDCRecoHit2D>(zPriorityStereoSegment3D, endSegment) ;

  // Correct the values of szSlope and z0

  FloatType szSlopeShift = trajectorySZ.getSZSlope();

  Vector3D fusedLocalOrigin = fusedTrajectory3D.getLocalOrigin();
  FloatType sOffset = axialTrajectory2D.calcPerpS(fusedLocalOrigin.xy());
  FloatType zShift = trajectorySZ.mapSToZ(sOffset);

  fusedTrajectory3D.shiftSZSlopeIntercept(szSlopeShift, zShift);

  return fusedTrajectory3D;

}





void CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
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


  CDCTrajectory3D trajectory3D = reconstructFuseTrajectories(startSegment, endSegment);
  axialStereoSegmentPair.setTrajectory3D(trajectory3D);




}
