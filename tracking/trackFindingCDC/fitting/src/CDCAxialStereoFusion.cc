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

#include <tracking/trackFindingCDC/utilities/GetValueType.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  const bool useResidualParameters = true;

  template<class ARecoHitSegment>
  JacobianMatrix<3, 5> calcAmbiguityImpl(const ARecoHitSegment& segment,
                                         const CDCTrajectory2D& trajectory2D)
  {
    using ARecoHit = GetValueType<ARecoHitSegment>;

    size_t nHits = segment.size();

    const Vector2D& localOrigin2D = trajectory2D.getLocalOrigin();
    const UncertainPerigeeCircle& localCircle = trajectory2D.getLocalCircle();

    double zeta = 0;
    for (const ARecoHit& recoHit : segment) {
      const Vector2D& recoPos2D = recoHit.getRecoPos2D();
      const Vector2D localRecoPos2D = recoPos2D - localOrigin2D;
      const Vector2D normal = localCircle->normal(localRecoPos2D);
      const CDCWire& wire = recoHit.getWire();
      zeta += wire.getWireLine().movePerZ().dot(normal);
    }
    zeta /= nHits;

    JacobianMatrix<3, 5> result = JacobianMatrixUtil::zero<3, 5>();

    using namespace NHelixParameterIndices;
    result(c_Curv, c_Curv) = 1.0;
    result(c_Phi0, c_Phi0) = 1.0;
    result(c_I, c_I)       = 1.0;

    result(c_Phi0, c_TanL) =   zeta;
    result(c_I, c_Z0)      = - zeta;

    return result;
  }

  template<class AFromRecoHitSegment, class AToRecoHitSegment>
  CDCTrajectory3D fuseTrajectoriesImpl(const AFromRecoHitSegment& fromSegment,
                                       const AToRecoHitSegment& toSegment)
  {
    using AFromRecoHit = GetValueType<AFromRecoHitSegment>;
    using AToRecoHit   = GetValueType<AToRecoHitSegment>;

    CDCTrajectory3D result;
    result.setChi2(NAN);

    if (fromSegment.empty()) {
      B2WARNING("From segment is empty.");
      return result;
    }
    if (toSegment.empty()) {
      B2WARNING("To segment is empty.");
      return result;
    }

    CDCTrajectory2D fromTrajectory2D = fromSegment.getTrajectory2D();
    CDCTrajectory2D toTrajectory2D = toSegment.getTrajectory2D();

    if (not fromTrajectory2D.isFitted()) {
      return result;
    }

    if (not toTrajectory2D.isFitted()) {
      return result;
    }

    AFromRecoHit lastHitOfFromSegment = fromSegment.back();
    AToRecoHit firstHitOfToSegment = toSegment.front();

    Vector2D localOrigin2D = Vector2D::average(lastHitOfFromSegment.getRecoPos2D(),
                                               firstHitOfToSegment.getRecoPos2D());

    Vector3D localOrigin3D(localOrigin2D, 0.0);

    // Propagate to a common point
    fromTrajectory2D.setLocalOrigin(localOrigin2D);
    toTrajectory2D.setLocalOrigin(localOrigin2D);

    const UncertainPerigeeCircle& fromCircle = fromTrajectory2D.getLocalCircle();
    const UncertainPerigeeCircle& toCircle = toTrajectory2D.getLocalCircle();

    JacobianMatrix<3, 5> fromH = calcAmbiguityImpl(fromSegment, fromTrajectory2D);
    JacobianMatrix<3, 5> toH = calcAmbiguityImpl(toSegment, toTrajectory2D);

    UncertainHelix resultHelix = UncertainHelix::average(fromCircle, fromH, toCircle, toH);
    return CDCTrajectory3D(localOrigin3D, resultHelix);
  }
}

JacobianMatrix<3, 5> CDCAxialStereoFusion::calcAmbiguity(const CDCRecoSegment2D& recoSegment2D,
                                                         const CDCTrajectory2D& trajectory2D)
{
  return calcAmbiguityImpl(recoSegment2D, trajectory2D);
}

JacobianMatrix<3, 5> CDCAxialStereoFusion::calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                                         const CDCTrajectory2D& trajectory2D)
{
  return calcAmbiguityImpl(recoSegment3D, trajectory2D);
}

CDCTrajectory3D CDCAxialStereoFusion::fuseTrajectories(const CDCRecoSegment2D& fromSegment,
                                                       const CDCRecoSegment2D& toSegment)
{
  return fuseTrajectoriesImpl(fromSegment, toSegment);
}

void CDCAxialStereoFusion::fuseTrajectories(const CDCSegmentPair& segmentPair)
{
  const CDCRecoSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrToSegment = segmentPair.getToSegment();

  if (not ptrFromSegment) {
    B2WARNING("From segment unset.");
    return;
  }

  if (not ptrToSegment) {
    B2WARNING("To segment unset.");
    return;
  }

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  CDCTrajectory3D trajectory3D = fuseTrajectories(fromSegment, toSegment);
  segmentPair.setTrajectory3D(trajectory3D);
}



CDCTrajectory3D CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCRecoSegment2D& fromSegment,
    const CDCRecoSegment2D& toSegment,
    bool priorityOnSZ)
{
  if (fromSegment.empty()) {
    B2WARNING("From segment is empty.");
    return CDCTrajectory3D();
  }

  if (toSegment.empty()) {
    B2WARNING("To segment is empty.");
    return CDCTrajectory3D();
  }

  bool fromIsAxial = fromSegment.getStereoKind() == EStereoKind::c_Axial;

  const CDCAxialRecoSegment2D& axialSegment = fromIsAxial ? fromSegment : toSegment;
  const CDCStereoRecoSegment2D& stereoSegment = not fromIsAxial ? fromSegment : toSegment;

  const CDCTrajectory2D& axialTrajectory2D = axialSegment.getTrajectory2D();

  // const Vector2D& localOrigin2D = (fromIsAxial ? fromSegment.back() : toSegment.front()).getRecoPos2D();
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
    stereoSegment3D = CDCRecoSegment3D::reconstruct(stereoSegment, axialMCTrajectory3D.getTrajectory2D());

  } else {
    CDCSZFitter szFitter = CDCSZFitter::getFitter();
    trajectorySZ = szFitter.fit(stereoSegment3D);
    if (not trajectorySZ.isFitted()) {
      CDCTrajectory3D result;
      result.setChi2(NAN);
      return result;
    }
  }

  CDCRiemannFitter riemannFitter;
  // riemannFitter.useOnlyOrientation();
  riemannFitter.useOnlyPosition();

  if (priorityOnSZ) {
    // To reconstructed point in the three dimensional stereo segment all lie exactly on the circle they are reconstructed onto.
    // This part draws them away from the circle onto the sz trajectory instead leaving all the residuals visible in the xy projection.
    // Hence the two dimensional fit, which is used for the fusion afterwards can react to residuals and rtoer the covariances of the stereo segment broader.
    for (CDCRecoHit3D& recoHit3D : stereoSegment3D) {
      const CDCWire& wire = recoHit3D.getWire();

      const double s = recoHit3D.getArcLength2D();
      const double newZ = trajectorySZ.mapSToZ(s);

      const Vector2D recoWirePos2D = wire.getWireLine().pos2DAtZ(newZ);
      const Vector2D correctedRecoPos2D = axialTrajectory2D.getClosest(recoWirePos2D);
      const Vector3D correctedRecoPos3D(correctedRecoPos2D, newZ);
      const double correctedPerpS = axialTrajectory2D.calcArcLength2D(correctedRecoPos2D);

      recoHit3D.setRecoPos3D(Vector3D(correctedRecoPos2D, newZ));
      recoHit3D.setArcLength2D(correctedPerpS);
      recoHit3D.snapToDriftCircle();
    }
  }

  CDCTrajectory2D stereoTrajectory2D = riemannFitter.fit(stereoSegment3D);
  stereoSegment3D.setTrajectory2D(stereoTrajectory2D);
  if (not stereoTrajectory2D.isFitted()) {
    return CDCTrajectory3D();
  }

  // Correct the values of tan lambda and z0
  CDCTrajectory3D fusedTrajectory3D =
    fromIsAxial ?
    fuseTrajectoriesImpl(fromSegment, stereoSegment3D) :
    fuseTrajectoriesImpl(stereoSegment3D, toSegment);

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
  const CDCRecoSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCRecoSegment2D* ptrToSegment = segmentPair.getToSegment();

  if (not ptrFromSegment) {
    B2WARNING("From segment unset.");
    return;
  }

  if (not ptrToSegment) {
    B2WARNING("To segment unset.");
    return;
  }

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  CDCTrajectory3D trajectory3D = reconstructFuseTrajectories(fromSegment, toSegment, priorityOnSZ);
  segmentPair.setTrajectory3D(trajectory3D);
}
