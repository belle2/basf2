/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <cdc/dataobjects/CDCSimHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCRecoHit3D::CDCRecoHit3D(const CDCRLTaggedWireHit& rlWireHit,
                           const Vector3D& recoPos3D,
                           double perpS) :
  m_rlWireHit(rlWireHit),
  m_recoPos3D(recoPos3D),
  m_arcLength2D(perpS)
{
}

CDCRecoHit3D::CDCRecoHit3D(const CDCWireHit* wireHit,
                           ERightLeft rlInfo,
                           const Vector3D& recoPos3D,
                           double perpS) :
  m_rlWireHit(wireHit, rlInfo),
  m_recoPos3D(recoPos3D),
  m_arcLength2D(perpS)
{
}


CDCRecoHit3D CDCRecoHit3D::fromSimHit(const CDCWireHit* wireHit,
                                      const CDCSimHit& simHit)
{
  // prepS cannot be deduced from the flightTime in this context
  double perpS = std::numeric_limits<double>::quiet_NaN();

  return CDCRecoHit3D(CDCRLTaggedWireHit::fromSimHit(wireHit, simHit),
                      Vector3D{simHit.getPosTrack()},
                      perpS);
}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit2D,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = recoHit2D.reconstruct3D(trajectory2D);
  double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(recoHit2D.getRLWireHit(), recoPos3D, perpS);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCWireHit* wireHit,
                                       ERightLeft rlInfo,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = wireHit->reconstruct3D(trajectory2D, rlInfo);
  double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(wireHit, rlInfo, recoPos3D, perpS);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRLTaggedWireHit& rlWireHit,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
  double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(rlWireHit, recoPos3D, perpS);
}



CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit,
                                       const CDCTrajectory3D& trajectory3D)
{
  // This this is quite legacy behaviour - do something smarter.
  CDCTrajectory2D trajectory2D = trajectory3D.getTrajectory2D();
  CDCTrajectorySZ trajectorySZ = trajectory3D.getTrajectorySZ();

  return reconstruct(recoHit, trajectory2D, trajectorySZ);
}


CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit,
                                       const CDCTrajectory2D& trajectory2D,
                                       const CDCTrajectorySZ& trajectorySZ)
{
  EStereoKind stereoType = recoHit.getStereoKind();
  if (stereoType == EStereoKind::c_Axial) {

    Vector2D recoPos2D = trajectory2D.getClosest(recoHit.getRecoPos2D());
    double perpS    = trajectory2D.calcArcLength2D(recoPos2D);
    double z        = trajectorySZ.mapSToZ(perpS);

    Vector3D recoPos3D(recoPos2D, z);
    return CDCRecoHit3D(recoHit.getRLWireHit(), recoPos3D, perpS);

  } else if (stereoType == EStereoKind::c_StereoU or stereoType == EStereoKind::c_StereoV) {
    //the closest approach of a wire line to a helix
    //( in this case representated by the two trajectories )
    //can not be solved as a closed expression
    //in the common case the z fit has been derived from the reconstructed points generated
    //with the reconstruct methode above in the other reconstruct method.
    //sticking to that method but using the average z from the sz fit

    const WireLine wireLine = recoHit.getWireLine();
    Vector3D recoPos3D = trajectory2D.reconstruct3D(wireLine);
    double perpS    = trajectory2D.calcArcLength2D(recoPos3D.xy());
    double z        = trajectorySZ.mapSToZ(perpS);
    recoPos3D.setZ(z);
    return CDCRecoHit3D(recoHit.getRLWireHit(), recoPos3D, perpS);
  }
  B2FATAL("Reconstruction on invalid wire");
}

CDCRecoHit3D CDCRecoHit3D::reconstructNearest(const CDCWireHit* wireHit,
                                              const CDCTrajectory2D& trackTrajectory2D)
{
  B2ASSERT(wireHit->isAxial(), "This function can only be used with axial hits.");
  ERightLeft rlInfo = trackTrajectory2D.isRightOrLeft(wireHit->getRefPos2D());
  CDCRLTaggedWireHit rlWireHit(wireHit, rlInfo);
  return CDCRecoHit3D::reconstruct(rlWireHit, trackTrajectory2D);
}

CDCRecoHit3D CDCRecoHit3D::average(const CDCRecoHit3D& first, const CDCRecoHit3D& second)
{
  if (first.getRLWireHit() == second.getRLWireHit()) {
    return CDCRecoHit3D(first.getRLWireHit(),
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
  const WireLine& wireLine = wire.getWireLine();
  const double recoPosZ = getRecoPos3D().z();

  Vector2D wirePos = wireLine.pos2DAtZ(recoPosZ);
  Vector2D disp2D = getRecoPos3D().xy() - wirePos;
  return disp2D;
}

void CDCRecoHit3D::reverse()
{
  m_rlWireHit.reverse();
}

CDCRecoHit3D CDCRecoHit3D::reversed() const
{
  return CDCRecoHit3D(getRLWireHit().reversed(), getRecoPos3D(), -getArcLength2D());
}
