/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <cdc/dataobjects/CDCSimHit.h>

#include <framework/logging/Logger.h>

#include <TVector3.h>

#include <ostream>
#include <type_traits>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace Belle2 {
  namespace TrackFindingCDC {
    class WireLine;
  }
}
CDCRLWireHit::CDCRLWireHit(const CDCWireHit* wireHit, ERightLeft rlInfo)
  : CDCRLWireHit(wireHit, rlInfo, wireHit->getRefDriftLength(), wireHit->getRefDriftLengthVariance())
{
}

CDCRLWireHit::CDCRLWireHit(const CDCWireHit* wireHit,
                           ERightLeft rlInfo,
                           double driftLength,
                           double driftLengthVariance)
  : m_wireHit(wireHit)
  , m_rlInfo(rlInfo)
  , m_refDriftLength(driftLength)
  , m_refDriftLengthVariance(driftLengthVariance)
{
}

CDCRLWireHit CDCRLWireHit::average(const CDCRLWireHit& rlWireHit1,
                                   const CDCRLWireHit& rlWireHit2)
{
  B2ASSERT("Average of two CDCRLWireHits with different wire hits requested.",
           rlWireHit1.getWireHit() == rlWireHit2.getWireHit());

  B2ASSERT("Average of two CDCRLWireHits with different right left passage information requested.",
           rlWireHit1.getRLInfo() == rlWireHit2.getRLInfo());

  ERightLeft rlInfo = rlWireHit1.getRLInfo();
  const CDCWireHit& wireHit = rlWireHit1.getWireHit();

  double driftLength = (rlWireHit1.getRefDriftLength() +
                        rlWireHit2.getRefDriftLength()) / 2.0;

  double driftLengthVariance = (rlWireHit1.getRefDriftLengthVariance() +
                                rlWireHit2.getRefDriftLengthVariance()) / 2.0;

  CDCRLWireHit result(&wireHit, rlInfo, driftLength, driftLengthVariance);
  return result;
}

CDCRLWireHit CDCRLWireHit::average(const CDCRLWireHit& rlWireHit1,
                                   const CDCRLWireHit& rlWireHit2,
                                   const CDCRLWireHit& rlWireHit3)
{
  B2ASSERT("Average of three CDCRLWireHits with different wire hits requested.",
           rlWireHit1.getWireHit() == rlWireHit2.getWireHit() and
           rlWireHit2.getWireHit() == rlWireHit3.getWireHit());

  B2ASSERT("Average of three CDCRLWireHits with different right left passage information requested.",
           rlWireHit1.getRLInfo() == rlWireHit2.getRLInfo() and
           rlWireHit2.getRLInfo() == rlWireHit3.getRLInfo());


  ERightLeft rlInfo = rlWireHit1.getRLInfo();
  const CDCWireHit& wireHit = rlWireHit1.getWireHit();

  double driftLength = (rlWireHit1.getRefDriftLength() +
                        rlWireHit2.getRefDriftLength() +
                        rlWireHit3.getRefDriftLength()) / 3.0;

  double driftLengthVariance = (rlWireHit1.getRefDriftLengthVariance() +
                                rlWireHit2.getRefDriftLengthVariance() +
                                rlWireHit3.getRefDriftLengthVariance()) / 3.0;

  CDCRLWireHit result(&wireHit, rlInfo, driftLength, driftLengthVariance);
  return result;
}


CDCRLWireHit CDCRLWireHit::fromSimHit(const CDCWireHit* wirehit,
                                      const CDCSimHit& simhit)
{
  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire{simhit.getPosWire() - simhit.getPosTrack()};
  Vector3D directionOfFlight{simhit.getMomentum()};

  ERightLeft rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  CDCRLWireHit rlWireHit(wirehit, rlInfo, simhit.getDriftLength(), CDCWireHit::c_simpleDriftLengthVariance);

  return rlWireHit;
}

const CDCHit* CDCRLWireHit::getHit() const
{
  return getWireHit().getHit();
}

const CDCWire& CDCRLWireHit::getWire() const
{
  return getWireHit().getWire();
}

const WireID& CDCRLWireHit::getWireID() const
{
  return getWire().getWireID();
}

ISuperLayer CDCRLWireHit::getISuperLayer() const
{
  return getWire().getISuperLayer();
}

EStereoKind CDCRLWireHit::getStereoKind() const
{
  return getWire().getStereoKind();
}

bool CDCRLWireHit::isAxial() const
{
  return getWire().isAxial();
}

const Vector2D& CDCRLWireHit::getRefPos2D() const
{
  return getWire().getRefPos2D();
}

double CDCRLWireHit::getRefCylindricalR() const
{
  return getWire().getRefCylindricalR();
}

Vector2D CDCRLWireHit::reconstruct2D(const CDCTrajectory2D& trajectory2D) const
{
  const Vector2D& refPos2D = getRefPos2D();
  Vector2D recoPos2D = trajectory2D.getClosest(refPos2D);

  const Vector2D& wirePos2D = getWire().getRefPos2D();
  const double driftLength = getRefDriftLength();

  Vector2D disp2D = recoPos2D - wirePos2D;

  // Fix the displacement to lie on the drift circle.
  disp2D.normalizeTo(driftLength);
  return wirePos2D + disp2D;
}

Vector3D CDCRLWireHit::reconstruct3D(const CDCTrajectory2D& trajectory2D, const double z) const
{
  const EStereoKind stereoType = getStereoKind();
  const ERightLeft rlInfo = getRLInfo();

  if (stereoType == EStereoKind::c_StereoV or stereoType == EStereoKind::c_StereoU) {
    const WireLine& wireLine = getWire().getWireLine();
    const double signedDriftLength = isValid(rlInfo) ? rlInfo * getRefDriftLength() : 0.0;
    return trajectory2D.reconstruct3D(wireLine, signedDriftLength, z);

  } else { /*if (stereoType == EStereoKind::c_Axial)*/
    const Vector2D recoPos2D = reconstruct2D(trajectory2D);
    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it the basic assumption.
    return Vector3D(recoPos2D, z);
  }
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const CDCRLWireHit& rlWireHit)
{
  output << "CDCRLWireHit(" << rlWireHit.getWireHit() << ","
         << static_cast<typename std::underlying_type<ERightLeft>::type>(rlWireHit.getRLInfo()) << ")" ;
  return output;
}
