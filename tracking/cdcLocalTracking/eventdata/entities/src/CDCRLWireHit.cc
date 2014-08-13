/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRLWireHit.h"

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRLWireHit)





CDCRLWireHit::CDCRLWireHit() :
  m_wirehit(nullptr),
  m_rlInfo(LEFT)
{;}





CDCRLWireHit::CDCRLWireHit(
  const CDCWireHit* wirehit,
  RightLeftInfo rlInfo
) :
  m_wirehit(wirehit),
  m_rlInfo(rlInfo)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}





CDCRLWireHit::~CDCRLWireHit()
{
}





CDCRLWireHit CDCRLWireHit::fromSimHit(
  const CDCWireHit* wirehit,
  const CDCSimHit& simhit
)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simhit.getPosWire();
  trackPosToWire.subtract(simhit.getPosTrack());

  Vector3D directionOfFlight = simhit.getMomentum();

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  CDCRLWireHit rlWireHit(wirehit, rlInfo);

  return rlWireHit;

}





const CDCRLWireHit* CDCRLWireHit::reversed() const
{
  return CDCWireHitTopology::getInstance().getReverseOf(*this);
}





Vector3D CDCRLWireHit::reconstruct3D(const CDCTrajectory2D& trajectory2D) const
{
  const StereoType stereoType = getStereoType();

  if (stereoType == STEREO_V or stereoType == STEREO_U) {
    const BoundSkewLine& skewLine = getWire().getSkewLine();
    const FloatType& signedDriftLength = getSignedRefDriftLength();

    Vector3D recoWirePos3D = trajectory2D.reconstruct3D(skewLine, signedDriftLength);

    Vector2D recoPos2D = trajectory2D.getClosest(recoWirePos3D.xy());
    return Vector3D(recoPos2D, recoWirePos3D.z());

  } else if (stereoType == AXIAL) {
    Vector2D recoPos2D = getRecoPos2D(trajectory2D);

    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it to a not a number
    FloatType z        = std::numeric_limits<FloatType>::quiet_NaN();

    return Vector3D(recoPos2D, z);

  } else {
    B2ERROR("Reconstruction on invalid wire");
    return Vector3D();

  }

}
