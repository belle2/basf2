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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <cdc/dataobjects/CDCSimHit.h>

#include <framework/logging/Logger.h>

#include <cmath>
#include <climits>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoHit3D::CDCRecoHit3D(const CDCRLWireHit& rlWireHit,
                           const Vector3D& recoPos3D,
                           double arcLength2D)
  : m_rlWireHit(rlWireHit)
  , m_recoPos3D(recoPos3D)
  , m_arcLength2D(arcLength2D)
{
}

CDCRecoHit3D CDCRecoHit3D::fromSimHit(const CDCWireHit* wireHit, const CDCSimHit& simHit)
{
  // arc length cannot be deduced from the flightTime in this context
  double arcLength2D = std::numeric_limits<double>::quiet_NaN();

  return CDCRecoHit3D(CDCRLWireHit::fromSimHit(wireHit, simHit),
                      Vector3D{simHit.getPosTrack()},
                      arcLength2D);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit2D,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = recoHit2D.reconstruct3D(trajectory2D);
  double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(recoHit2D.getRLWireHit(), recoPos3D, arcLength2D);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCWireHit* wireHit,
                                       ERightLeft rlInfo,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = wireHit->reconstruct3D(trajectory2D, rlInfo);
  double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
  CDCRLWireHit rlWireHit(wireHit, rlInfo);
  return CDCRecoHit3D(rlWireHit, recoPos3D, arcLength2D);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRLWireHit& rlWireHit,
                                       const CDCTrajectory2D& trajectory2D)
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
  double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
  return CDCRecoHit3D(rlWireHit, recoPos3D, arcLength2D);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit,
                                       const CDCTrajectory3D& trajectory3D)
{
  // This this is quite legacy behaviour - do something smarter.
  CDCTrajectory2D trajectory2D = trajectory3D.getTrajectory2D();
  CDCTrajectorySZ trajectorySZ = trajectory3D.getTrajectorySZ();

  return reconstruct(recoHit, trajectory2D, trajectorySZ);
}

CDCRecoHit3D CDCRecoHit3D::reconstruct(const CDCRecoHit2D& recoHit2D,
                                       const CDCTrajectory2D& trajectory2D,
                                       const CDCTrajectorySZ& trajectorySZ)
{
  EStereoKind stereoKind = recoHit2D.getStereoKind();

  double arcLength2D = 0;
  if (stereoKind == EStereoKind::c_StereoU or stereoKind == EStereoKind::c_StereoV) {
    //the closest approach of a wire line to a helix
    //( in this case representated by the two trajectories )
    //can not be solved as a closed expression
    //in the common case the z fit has been derived from the reconstructed points generated
    //with the reconstruct methode above in the other reconstruct method.
    //sticking to that method but using the average z from the sz fit
    Vector3D recoPos3D = recoHit2D.reconstruct3D(trajectory2D);
    arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());

  } else { /* if (stereoKind == EStereoKind::c_Axial)*/
    Vector2D recoPos2D = trajectory2D.getClosest(recoHit2D.getRecoPos2D());
    arcLength2D = trajectory2D.calcArcLength2D(recoPos2D);

  }

  const double z = trajectorySZ.mapSToZ(arcLength2D);

  // Reevaluating the z position eventually accounts for wire sag.
  const CDCWire& wire = recoHit2D.getWire();
  const Vector2D recoWirePos2D = wire.getWirePos2DAtZ(z);
  const Vector2D correctedRecoPos2D = trajectory2D.getClosest(recoWirePos2D);
  const double correctedPerpS = trajectory2D.calcArcLength2D(correctedRecoPos2D);
  const double correctedZ = trajectorySZ.mapSToZ(correctedPerpS);
  const Vector3D correctedRecoPos3D(correctedRecoPos2D, correctedZ);

  CDCRecoHit3D result(recoHit2D.getRLWireHit(), correctedRecoPos3D, correctedPerpS);
  result.snapToDriftCircle();
  return result;
}

CDCRecoHit3D CDCRecoHit3D::reconstructNearest(const CDCWireHit* axialWireHit,
                                              const CDCTrajectory2D& trajectory2D)
{
  B2ASSERT("This function can only be used with axial hits.", axialWireHit->isAxial());
  ERightLeft rlInfo = trajectory2D.isRightOrLeft(axialWireHit->getRefPos2D());
  CDCRLWireHit rlWireHit(axialWireHit, rlInfo);
  return CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
}

CDCRecoHit3D CDCRecoHit3D::average(const CDCRecoHit3D& first, const CDCRecoHit3D& second)
{
  if (first.getRLWireHit() == second.getRLWireHit()) {
    return CDCRecoHit3D(first.getRLWireHit(),
                        Vector3D::average(first.getRecoPos3D(), second.getRecoPos3D()),
                        (first.getArcLength2D() + second.getArcLength2D()) / 2);
  } else {
    B2ERROR("Averaging three dimensional hits which are on different oriented wire hits. Return "
            "first one unchanged");
    return first;
  }
}

Vector2D CDCRecoHit3D::getRecoDisp2D() const
{
  const CDCWire& wire = getWire();
  const double recoPosZ = getRecoPos3D().z();

  Vector2D wirePos = wire.getWirePos2DAtZ(recoPosZ);
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

void CDCRecoHit3D::snapToDriftCircle(bool switchSide)
{
  const CDCWire& wire = getWire();
  const double recoPosZ = getRecoPos3D().z();

  Vector2D wirePos = wire.getWirePos2DAtZ(recoPosZ);
  Vector2D disp2D = getRecoPos3D().xy() - wirePos;

  disp2D.normalizeTo(fabs(getSignedRecoDriftLength()));
  if (switchSide) {
    disp2D = -disp2D;
  }
  m_recoPos3D = Vector3D(wirePos + disp2D, recoPosZ);
}

void CDCRecoHit3D::setRecoDriftLength(double driftLength, bool snapRecoPos)
{
  double oldDriftLength = m_rlWireHit.getRefDriftLength();
  m_rlWireHit.setRefDriftLength(driftLength);
  if (snapRecoPos) {
    bool switchSide = sign(oldDriftLength) != sign(driftLength);
    snapToDriftCircle(switchSide);
  }
}

CDCRecoHit2D CDCRecoHit3D::getRecoHit2D() const
{
  return CDCRecoHit2D(m_rlWireHit, getRecoDisp2D());
}

CDCRecoHit2D CDCRecoHit3D::stereoProjectToRef() const
{
  return getRecoHit2D();
}

Vector2D CDCRecoHit3D::getRecoWirePos2D() const
{
  return getWire().getWirePos2DAtZ(getRecoZ());
}

bool CDCRecoHit3D::isInCellZBounds(const double factor) const
{
  return getWire().isInCellZBounds(getRecoPos3D(), factor);
}
