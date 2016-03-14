/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <cdc/dataobjects/CDCSimHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRecoHit3D::CDCRecoHit3D():
  m_rlWireHit(nullptr),
  m_recoPos3D(Vector3D::getLowest()),
  m_arcLength2D(0.0)
{;}


CDCRecoHit3D::CDCRecoHit3D(const CDCRLWireHit* rlWireHit,
                           const Vector3D& recoPos3D,
                           double perpS) :
  m_rlWireHit(rlWireHit),
  m_recoPos3D(recoPos3D),
  m_arcLength2D(perpS)
{
  if (rlWireHit == nullptr) B2ERROR("Initialization of three dimensional reconstructed hit with nullptr as oriented wire hit");
}

CDCRecoHit3D CDCRecoHit3D::fromSimHit(const CDCWireHit* wireHit,
                                      const CDCSimHit& simHit)
{

  //prepS cannot be deduced from the flightTime in this context
  double perpS = std::numeric_limits<double>::quiet_NaN();

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simHit.getPosWire() - simHit.getPosTrack()};

  Vector3D directionOfFlight{simHit.getMomentum()};

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  const CDCRLWireHit* rlWireHit = CDCWireHitTopology::getInstance().getRLWireHit(*wireHit, rlInfo);

  return CDCRecoHit3D(rlWireHit, Vector3D{simHit.getPosTrack()}, perpS);

}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit2D,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = recoHit2D.reconstruct3D(trajectory2D);
  double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(&(recoHit2D.getRLWireHit()), recoPos3D, perpS);
}


CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRLWireHit& rlWireHit,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
  double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(&rlWireHit, recoPos3D, perpS);
}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit,
                                       const CDCTrajectory2D& trajectory2D,
                                       const CDCTrajectorySZ& trajectorySZ)
{

  StereoType stereoType = recoHit.getStereoType();
  if (stereoType == StereoType_c::Axial) {
    Vector2D recoPos2D = trajectory2D.getClosest(recoHit.getRecoPos2D());
    double perpS    = trajectory2D.calcArcLength2D(recoPos2D);
    double z        = trajectorySZ.mapSToZ(perpS);

    Vector3D recoPos3D(recoPos2D, z);
    return CDCRecoHit3D(&(recoHit.getRLWireHit()), recoPos3D, perpS);

  } else if (stereoType == StereoType_c::StereoU or stereoType == StereoType_c::StereoV) {
    //the closest approach of a skew line to a helix
    //( in this case representated by the two trajectories )
    //can not be solved as a closed expression
    //in the common case the z fit has been derived from the reconstructed points generated
    //with the reconstruct methode above in the other reconstruct method.
    //sticking to that method but using the average z from the sz fit

    const WireLine skewLine = recoHit.getSkewLine();
    Vector3D recoPos3D = trajectory2D.reconstruct3D(skewLine);
    double perpS    = trajectory2D.calcArcLength2D(recoPos3D.xy());
    double z        = trajectorySZ.mapSToZ(perpS);
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
                        (first.getArcLength2D() + second.getArcLength2D()) / 2);
  } else {
    B2ERROR("Averaging three dimensional hits which are passed on different oriented wire hits. Return first one unchanged");
    return first;
  }
}


Vector2D CDCRecoHit3D::getRecoDisp2D() const
{
  const CDCWire& wire = getWire();
  const WireLine& skewLine = wire.getSkewLine();
  const double recoPosZ = getRecoPos3D().z();

  Vector2D wirePos = skewLine.pos2DAtZ(recoPosZ);
  Vector2D disp2D = getRecoPos3D().xy() - wirePos;
  return disp2D;
}



void CDCRecoHit3D::reverse()
{
  const CDCRLWireHit* reverseRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit());
  setRLWireHit(reverseRLWireHit);
}



CDCRecoHit3D CDCRecoHit3D::reversed() const
{
  return CDCRecoHit3D(CDCWireHitTopology::getInstance().getReverseOf(getRLWireHit()), getRecoPos3D(), getArcLength2D());
}
