/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <cdc/dataobjects/CDCSimHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;



CDCRLWireHit::CDCRLWireHit() :
  m_wirehit(nullptr),
  m_rlInfo(LEFT)
{;}


CDCRLWireHit::CDCRLWireHit(const CDCWireHit* wirehit,
                           RightLeftInfo rlInfo) :
  m_wirehit(wirehit),
  m_rlInfo(rlInfo)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}




CDCRLWireHit CDCRLWireHit::fromSimHit(const CDCWireHit* wirehit,
                                      const CDCSimHit& simhit)
{
  if (wirehit == nullptr) B2WARNING("Recohit with nullptr as wire hit");

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simhit.getPosWire() - simhit.getPosTrack()};
  Vector3D directionOfFlight{simhit.getMomentum()};

  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  CDCRLWireHit rlWireHit(wirehit, rlInfo);

  return rlWireHit;

}





const CDCRLWireHit* CDCRLWireHit::reversed() const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  return wireHitTopology.getReverseOf(*this);
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

    const Vector2D& refPos2D = getRefPos2D();
    Vector2D recoPos2D = trajectory2D.getClosest(refPos2D);

    const Vector2D& wirePos2D = getWire().getRefPos2D();
    const FloatType& driftLength = getRefDriftLength();

    Vector2D disp2D = recoPos2D - wirePos2D;
    // Fix the displacement to lie on the drift circle.
    disp2D.setCylindricalR(driftLength);

    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it the basic assumption.
    FloatType z = 0;
    return Vector3D(wirePos2D + disp2D, z);

  } else {
    B2ERROR("Reconstruction on invalid wire");
    return Vector3D();

  }

}
