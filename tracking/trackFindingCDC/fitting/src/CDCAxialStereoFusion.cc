/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>
#include <tracking/trackFindingCDC/utilities/GetValueType.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  CDCRecoSegment3D reconstruct(const CDCRecoSegment2D& segment2D,
                               const CDCTrajectory3D& trajectory3D)
  {
    CDCRecoSegment3D result;
    CDCTrajectory2D trajectory2D = trajectory3D.getTrajectory2D();
    CDCTrajectorySZ trajectorySZ = trajectory3D.getTrajectorySZ();

    for (const CDCRecoHit2D& recoHit2D : segment2D) {
      result.push_back(CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D, trajectorySZ));
    }
    return result;
  }

  void reestimateDriftLength(CDCRecoSegment3D& segment3D,
                             const CDCTrajectory3D& trajectory3D)
  {
    double tanLambda = trajectory3D.getTanLambda();
    double theta = M_PI / 2 - std::atan(tanLambda);
    static CDC::RealisticTDCCountTranslator tdcCountTranslator(true);
    const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();
    for (CDCRecoHit3D& recoHit3D : segment3D) {
      Vector2D flightDirection = recoHit3D.getFlightDirection2D();
      Vector2D recoPos2D = recoHit3D.getRecoPos2D();
      double alpha = recoPos2D.angleWith(flightDirection);
      const double beta = 1;
      double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);
      flightTimeEstimate *= hypot2(1, tanLambda);

      const CDCWire& wire = recoHit3D.getWire();
      const CDCHit* hit = recoHit3D.getWireHit().getHit();
      const bool rl = recoHit3D.getRLInfo() == ERightLeft::c_Right;
      double driftLength =
        tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                          wire.getWireID(),
                                          flightTimeEstimate,
                                          rl,
                                          recoHit3D.getRecoZ(),
                                          alpha,
                                          theta,
                                          hit->getADCCount());

      if (driftLength > -2) {
        bool snapRecoPos = true;
        recoHit3D.setRecoDriftLength(driftLength, snapRecoPos);
      }
    }
  }
}

void CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCSegmentPair& segmentPair)
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

  CDCTrajectory3D trajectory3D = reconstructFuseTrajectories(fromSegment, toSegment);
  segmentPair.setTrajectory3D(trajectory3D);
}

void CDCAxialStereoFusion::fusePreliminary(const CDCSegmentPair& segmentPair)
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

  CDCTrajectory3D trajectory3D = fusePreliminary(fromSegment, toSegment);
  segmentPair.setTrajectory3D(trajectory3D);
}


CDCTrajectory3D CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCRecoSegment2D& fromSegment2D,
    const CDCRecoSegment2D& toSegment2D)
{
  CDCTrajectory3D preliminaryTrajectory3D = fusePreliminary(fromSegment2D, toSegment2D);
  return reconstructFuseTrajectories(fromSegment2D, toSegment2D, preliminaryTrajectory3D);
}

CDCTrajectory3D CDCAxialStereoFusion::fusePreliminary(const CDCRecoSegment2D& fromSegment2D,
                                                      const CDCRecoSegment2D& toSegment2D)
{
  if (fromSegment2D.empty()) {
    B2WARNING("From segment is empty.");
    return CDCTrajectory3D();
  }

  if (toSegment2D.empty()) {
    B2WARNING("To segment is empty.");
    return CDCTrajectory3D();
  }

  bool fromIsAxial = fromSegment2D.isAxial();
  const CDCAxialRecoSegment2D& axialSegment2D = fromIsAxial ? fromSegment2D : toSegment2D;
  const CDCStereoRecoSegment2D& stereoSegment2D = not fromIsAxial ? fromSegment2D : toSegment2D;

  CDCTrajectory2D axialTrajectory2D = axialSegment2D.getTrajectory2D();

  Vector2D localOrigin2D = (fromIsAxial ? fromSegment2D.back() : toSegment2D.front()).getRecoPos2D();
  axialTrajectory2D.setLocalOrigin(localOrigin2D);

  CDCRecoSegment3D stereoSegment3D = CDCRecoSegment3D::reconstruct(stereoSegment2D, axialTrajectory2D);

  CDCTrajectorySZ trajectorySZ;
  const bool mcTruthReference = false;
  if (mcTruthReference) {
    const CDCMCSegment2DLookUp& theMCSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
    CDCTrajectory3D axialMCTrajectory3D = theMCSegmentLookUp.getTrajectory3D(&axialSegment2D);
    Vector3D localOrigin3D =  axialMCTrajectory3D.getLocalOrigin();
    localOrigin3D.setXY(axialTrajectory2D.getLocalOrigin());
    axialMCTrajectory3D.setLocalOrigin(localOrigin3D);
    trajectorySZ = axialMCTrajectory3D.getTrajectorySZ();
    stereoSegment3D = CDCRecoSegment3D::reconstruct(stereoSegment2D, axialMCTrajectory3D.getTrajectory2D());

  } else {
    CDCSZFitter szFitter = CDCSZFitter::getFitter();
    trajectorySZ = szFitter.fit(stereoSegment3D);
    if (not trajectorySZ.isFitted()) {
      CDCTrajectory3D result;
      result.setChi2(NAN);
      return result;
    }
  }

  CDCTrajectory3D preliminaryTrajectory3D(axialTrajectory2D, trajectorySZ);
  Vector3D localOrigin3D(localOrigin2D, 0.0);
  preliminaryTrajectory3D.setLocalOrigin(localOrigin3D);
  return preliminaryTrajectory3D;
}

CDCTrajectory3D CDCAxialStereoFusion::reconstructFuseTrajectories(const CDCRecoSegment2D& fromSegment2D,
    const CDCRecoSegment2D& toSegment2D,
    const CDCTrajectory3D& preliminaryTrajectory3D)
{
  Vector3D localOrigin3D = preliminaryTrajectory3D.getLocalOrigin();
  Vector2D localOrigin2D = localOrigin3D.xy();

  CDCRiemannFitter riemannFitter;
  //riemannFitter.useOnlyOrientation();
  riemannFitter.useOnlyPosition();

  CDCRecoSegment3D fromSegment3D = reconstruct(fromSegment2D, preliminaryTrajectory3D);
  CDCRecoSegment3D toSegment3D   = reconstruct(toSegment2D, preliminaryTrajectory3D);

  if (m_reestimateDriftLength) {
    reestimateDriftLength(fromSegment3D,  preliminaryTrajectory3D);
    reestimateDriftLength(toSegment3D,  preliminaryTrajectory3D);
  }

  CDCTrajectory2D fromTrajectory2D = riemannFitter.fit(fromSegment3D);
  CDCTrajectory2D toTrajectory2D   = riemannFitter.fit(toSegment3D);

  fromTrajectory2D.setLocalOrigin(localOrigin2D);
  toTrajectory2D.setLocalOrigin(localOrigin2D);

  SZParameters refSZ = preliminaryTrajectory3D.getLocalSZLine().szParameters();

  const UncertainPerigeeCircle& fromCircle = fromTrajectory2D.getLocalCircle();
  const UncertainPerigeeCircle& toCircle   = toTrajectory2D.getLocalCircle();

  JacobianMatrix<3, 5> fromH = calcAmbiguity(fromSegment3D, fromTrajectory2D);
  JacobianMatrix<3, 5> toH   = calcAmbiguity(toSegment3D,   toTrajectory2D);

  UncertainHelix resultHelix = UncertainHelix::average(fromCircle, fromH, toCircle, toH, refSZ);
  return CDCTrajectory3D(localOrigin3D, resultHelix);
}

JacobianMatrix<3, 5> CDCAxialStereoFusion::calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                                         const CDCTrajectory2D& trajectory2D)
{
  size_t nHits = recoSegment3D.size();

  const Vector2D& localOrigin2D = trajectory2D.getLocalOrigin();
  const UncertainPerigeeCircle& localCircle = trajectory2D.getLocalCircle();

  double zeta = 0;
  for (const CDCRecoHit3D& recoHit3D : recoSegment3D) {
    const Vector2D& recoPos2D = recoHit3D.getRecoPos2D();
    const Vector2D localRecoPos2D = recoPos2D - localOrigin2D;
    const Vector2D normal = localCircle->normal(localRecoPos2D);
    const CDCWire& wire = recoHit3D.getWire();
    zeta += wire.getWireLine().sagMovePerZ(recoHit3D.getRecoZ()).dot(normal);
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
