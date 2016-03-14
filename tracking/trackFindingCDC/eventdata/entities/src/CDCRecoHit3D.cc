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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

TRACKFINDINGCDC_SwitchableClassImp(CDCRecoHit3D)

CDCRecoHit3D::CDCRecoHit3D():
  m_rlWireHit(nullptr),
  m_recoPos3D(Vector3D::getLowest()),
  m_perpS(0.0)
{;}


CDCRecoHit3D::CDCRecoHit3D(const CDCRLWireHit* rlWireHit,
                           const Vector3D& recoPos3D,
                           FloatType perpS) :
  m_rlWireHit(rlWireHit),
  m_recoPos3D(recoPos3D),
  m_perpS(perpS)
{
  if (rlWireHit == nullptr) B2ERROR("Initialization of three dimensional reconstructed hit with nullptr as oriented wire hit");
}

CDCRecoHit3D CDCRecoHit3D::fromSimHit(const CDCWireHit* wireHit,
                                      const CDCSimHit& simHit)
{

  //prepS cannot be deduced from the flightTime in this context
  FloatType perpS = std::numeric_limits<FloatType>::quiet_NaN();

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simHit.getPosWire();
  trackPosToWire.subtract(simHit.getPosTrack());

  Vector3D directionOfFlight = simHit.getMomentum();

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  const CDCRLWireHit* rlWireHit = CDCWireHitTopology::getInstance().getRLWireHit(*wireHit, rlInfo);

  return CDCRecoHit3D(rlWireHit, simHit.getPosTrack(), perpS);

}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit2D,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = recoHit2D.reconstruct3D(trajectory2D);
  FloatType perpS = trajectory2D.calcPerpS(recoPos3D.xy());
  return CDCRecoHit3D(&(recoHit2D.getRLWireHit()), recoPos3D, perpS);
}


CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRLWireHit& rlWireHit,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
  FloatType perpS = trajectory2D.calcPerpS(recoPos3D.xy());
  return CDCRecoHit3D(&rlWireHit, recoPos3D, perpS);
}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit,
                                       const CDCTrajectory2D& trajectory2D,
                                       const CDCTrajectorySZ& trajectorySZ)
{

  StereoType stereoType = recoHit.getStereoType();
  if (stereoType == AXIAL) {
    Vector2D recoPos2D = trajectory2D.getClosest(recoHit.getRecoPos2D());
    FloatType perpS    = trajectory2D.calcPerpS(recoPos2D);
    FloatType z        = trajectorySZ.mapSToZ(perpS);

    Vector3D recoPos3D(recoPos2D, z);
    return CDCRecoHit3D(&(recoHit.getRLWireHit()), recoPos3D, perpS);

  } else if (stereoType == STEREO_U or stereoType == STEREO_V) {
    //the closest approach of a skew line to a helix
    //( in this case representated by the two trajectories )
    //can not be solved as a closed expression
    //in the common case the z fit has been derived from the reconstructed points generated
    //with the reconstruct methode above in the other reconstruct method.
    //sticking to that method but using the average z from the sz fit

    const BoundSkewLine skewLine = recoHit.getSkewLine();
    Vector3D recoPos3D = trajectory2D.reconstruct3D(skewLine);
    FloatType perpS    = trajectory2D.calcPerpS(recoPos3D.xy());
    FloatType z        = trajectorySZ.mapSToZ(perpS);
    recoPos3D.setZ(z);
    return CDCRecoHit3D(&(recoHit.getRLWireHit()), recoPos3D, perpS);

  } else {
    B2ERROR("Reconstruction on invalid wire");
    return CDCRecoHit3D();
  }
}

CDCRecoHit3D CDCRecoHit3D::average(const CDCRecoHit3D& first, const CDCRecoHit3D& second)
{
  if (first.getRLWireHit() == second.getRLWireHit()) {
    return CDCRecoHit3D(&(first.getRLWireHit()),
                        Vector3D::average(first.getRecoPos3D(), second.getRecoPos3D()),
                        (first.getPerpS() + second.getPerpS()) / 2);
  } else {
    B2ERROR("Averaging three dimensional hits which are passed on different oriented wire hits. Return first one unchanged");
    return first;
  }
}


CDCRecoHit3D::~CDCRecoHit3D()
{;}



Vector2D CDCRecoHit3D::getRecoDisp2D() const
{
  const CDCWire& wire = getWire();
  const BoundSkewLine& skewLine = wire.getSkewLine();
  const FloatType& recoPosZ = getRecoPos3D().z();

  Vector2D wirePos = skewLine.pos2DAtZ(recoPosZ);
  Vector2D disp2D = getRecoPos3D().xy() - wirePos;
  return disp2D;
}



bool CDCRecoHit3D::isInCDC() const
{

  const CDCWire& wire = getWire();

  const double forwardZ = wire.getSkewLine().forwardZ();
  const double backwardZ = wire.getSkewLine().backwardZ();

  return (backwardZ < getRecoPos3D().z() and getRecoPos3D().z() < forwardZ);
}



void CDCRecoHit3D::reverse()
{
  const CDCRLWireHit* reverseRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit());
  setRLWireHit(reverseRLWireHit);
}



CDCRecoHit3D CDCRecoHit3D::reversed() const
{
  return CDCRecoHit3D(CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit()), getRecoPos3D(), getPerpS());
}
