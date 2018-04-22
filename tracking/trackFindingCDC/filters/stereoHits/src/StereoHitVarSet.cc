/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <cdc/dataobjects/CDCHit.h>

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoHitVarSet::extract(const BaseStereoHitFilter::Object* testPair)
{
  const CDCRLWireHit* rlWireHit = testPair->getTo();
  const CDCTrack* track = testPair->getFrom();

  if (not testPair or not rlWireHit or not track) return false;


  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

  const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trajectory2D);

  const Vector2D& startMomentum = trajectory2D.getMom2DAtSupport();
  const double radius = trajectory2D.getLocalCircle()->radius();
  const double size = track->size();
  const Vector3D& reconstructedPosition = recoHit3D.getRecoPos3D();
  const double reconstructedDriftLength = recoHit3D.getSignedRecoDriftLength();
  const double reconstructedS = recoHit3D.getArcLength2D();
  const unsigned short int adcCount = rlWireHit->getWireHit().getHit()->getADCCount();
  const ERightLeft rlInformation = rlWireHit->getRLInfo();
  const double backArcLength2D = track->back().getArcLength2D();
  const double frontArcLength2D = track->front().getArcLength2D();
  const double arcLength2DSum = std::accumulate(track->begin(), track->end(), 0.0, [](const double sum,
  const CDCRecoHit3D & listRecoHit) { return sum + listRecoHit.getArcLength2D();});

  const CDCWire& wire = rlWireHit->getWire();
  Vector2D wirePos = wire.getWirePos2DAtZ(reconstructedPosition.z());
  Vector2D disp2D = reconstructedPosition.xy() - wirePos;
  const double xyDistance = disp2D.norm();

  const auto nearestAxialHit = std::min_element(track->begin(), track->end(), [&reconstructedS](const CDCRecoHit3D & lhs,
  const CDCRecoHit3D & rhs) {
    return std::abs(lhs.getArcLength2D() - reconstructedS) < std::abs(rhs.getArcLength2D() - reconstructedS);
  });

  var<named("track_size")>() = size;
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);
  var<named("reco_s")>() = toFinite(reconstructedS, 0);
  var<named("reco_z")>() = toFinite(reconstructedPosition.z(), 0);
  var<named("phi_track")>() = toFinite(startMomentum.phi(), 0);
  var<named("phi_hit")>() = reconstructedPosition.phi();
  var<named("theta_hit")>() = reconstructedPosition.theta();
  var<named("drift_length")>() = reconstructedDriftLength;
  var<named("adc_count")>() = adcCount;
  var<named("xy_distance_zero_z")>() = toFinite(xyDistance, 0);
  var<named("right_hit")>() = rlInformation == ERightLeft::c_Right;
  var<named("track_back_s")>() = toFinite(backArcLength2D, 0);
  var<named("track_front_s")>() = toFinite(frontArcLength2D, 0);
  var<named("track_mean_s")>() = toFinite(arcLength2DSum / size, 0);
  var<named("s_distance")>() = toFinite(nearestAxialHit->getArcLength2D() - reconstructedS, 0);
  var<named("track_radius")>() = toFinite(radius, 0);
  var<named("superlayer_id")>() = rlWireHit->getISuperLayer();
  return true;
}
