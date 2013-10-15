/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoHit3D.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRecoHit3D)

CDCRecoHit3D::CDCRecoHit3D():
  m_wirehit(&(CDCWireHit::getLowest())),
  m_position(Vector3D::getLowest()),
  m_rlInfo(LEFT),
  m_perpS(0.0)
{;}


CDCRecoHit3D::CDCRecoHit3D(
  const CDCWireHit* wirehit,
  const Vector3D& position,
  RightLeftInfo rlInfo,
  FloatType perpS
) :
  m_wirehit(wirehit),
  m_position(position) ,
  m_rlInfo(rlInfo),
  m_perpS(perpS)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}

CDCRecoHit3D CDCRecoHit3D::fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit)
{

  //prepS can not be deduced from the flightTime in this context
  FloatType perpS = std::numeric_limits<FloatType>::quiet_NaN();

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simhit.getPosWire();
  trackPosToWire.subtract(simhit.getPosTrack());

  Vector3D directionOfFlight = simhit.getMomentum();

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  return CDCRecoHit3D(wirehit, simhit.getPosTrack(), rlInfo, perpS);

}


CDCRecoHit3D CDCRecoHit3D::reconstruct(
  const CDCRecoHit2D& recohit,
  const CDCTrajectory2D& trajectory2D
)
{

  AxialType axialType = recohit->getAxialType();
  if (axialType == STEREO_V or axialType == STEREO_U) {

    BoundSkewLine skewLine = recohit.getSkewLine();
    Vector3D reconstructedPoint = trajectory2D.reconstruct3D(skewLine);
    FloatType perpS = trajectory2D.calcPerpS(reconstructedPoint.xy());
    return CDCRecoHit3D(recohit.getWireHit(), reconstructedPoint, recohit.getRLInfo(), perpS);

  } else if (axialType == AXIAL) {
    Vector2D recoPos2D = trajectory2D.getClosest(recohit->getRefPos2D());
    FloatType perpS    = trajectory2D.calcPerpS(recoPos2D);

    //for axial wire we can not determine the z coordinate by looking at the xy projection only
    //we set it to a not a number here
    FloatType z        = std::numeric_limits<FloatType>::quiet_NaN();

    return CDCRecoHit3D(recohit.getWireHit(), Vector3D(recoPos2D, z), recohit.getRLInfo(), perpS);
  } else {
    return CDCRecoHit3D();
  }
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(
  const CDCRecoHit2D& recohit,
  const CDCTrajectory2D& trajectory2D,
  const CDCTrajectorySZ& trajectorySZ
)
{

  AxialType axialType = recohit->getAxialType();
  if (axialType == AXIAL) {
    Vector2D recoPos2D = trajectory2D.getClosest(recohit->getRefPos2D());
    FloatType perpS    = trajectory2D.calcPerpS(recoPos2D);
    FloatType z        = trajectorySZ.mapSToZ(perpS);

    Vector3D recoPos3D(recoPos2D, z);
    return CDCRecoHit3D(recohit.getWireHit(), recoPos3D, recohit.getRLInfo(), perpS);

  } else if (axialType == STEREO_U or axialType == STEREO_V) {
    //the closest approach of a skew line to a helix
    //( in this case representated by the two trajectories )
    //can not be solved as a closed expression
    //in the common case the z fit has been derived from the reconstructed points generated
    //with the reconstruct methode above in the other reconstruct method.
    //sticking to that method but using the average z from the sz fit

    const BoundSkewLine skewLine = recohit.getSkewLine();
    Vector3D recoPos3D = trajectory2D.reconstruct3D(skewLine);
    FloatType perpS    = trajectory2D.calcPerpS(recoPos3D.xy());
    FloatType z        = trajectorySZ.mapSToZ(perpS);
    recoPos3D.setZ(z);
    return CDCRecoHit3D(recohit.getWireHit(), recoPos3D, recohit.getRLInfo(), perpS);

  } else {
    return CDCRecoHit3D();
  }
}

CDCRecoHit3D CDCRecoHit3D::average(const CDCRecoHit3D& first , const CDCRecoHit3D& second)
{
  return first.getWireHit() == second.getWireHit() ?
         CDCRecoHit3D(first.getWireHit(), Vector3D::average(first.getPos3D(), second.getPos3D()),
                      averageInfo(first.getRLInfo(), second.getRLInfo()),
                      (first.getPerpS() + second.getPerpS()) / 2) :
         first;
}


CDCRecoHit3D::~CDCRecoHit3D()
{;}

/** indicator if the hit is in the cdc or already outside its boundaries.
    Checks for z to be in the range of the wire. */
bool CDCRecoHit3D::isInCDC() const
{

  const CDCWire* wire = getWire();
  if (wire == nullptr) return false;

  const double forwardZ = wire->getSkewLine().forwardZ();
  const double backwardZ = wire->getSkewLine().backwardZ();

  return (backwardZ < getPos3D().z() and getPos3D().z() < forwardZ);
}

// Implement all functions

