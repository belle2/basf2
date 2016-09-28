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

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoHitVarSet::extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair)
{
  const CDCRecoHit3D* recoHit = testPair->first;
  const CDCTrack* track = testPair->second;

  bool extracted = extractNested(testPair);
  if (not extracted or not testPair or not recoHit or not track) return false;

  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();
  const Vector2D& startMomentum = trajectory2D.getMom2DAtSupport();
  const double radius = trajectory2D.getLocalCircle()->radius();
  const double size = track->size();
  const Vector3D& reconstructedPosition = recoHit->getRecoPos3D();
  const double reconstructedDriftLength = recoHit->getSignedRecoDriftLength();
  const double reconstructedS = recoHit->getArcLength2D();
  const unsigned short int adcCount = recoHit->getWireHit().getHit()->getADCCount();
  const ERightLeft rlInformation = recoHit->getRLInfo();
  const double backArcLength2D = track->back().getArcLength2D();
  const double frontArcLength2D = track->front().getArcLength2D();
  const double arcLength2DSum = std::accumulate(track->begin(), track->end(), 0.0, [](const double sum,
  const CDCRecoHit3D & listRecoHit) { return sum + listRecoHit.getArcLength2D();});

  const CDCWire& wire = recoHit->getWire();
  Vector2D wirePos = wire.getWirePos2DAtZ(reconstructedPosition.z());
  Vector2D disp2D = reconstructedPosition.xy() - wirePos;
  const double xyDistance = disp2D.norm();

  const auto nearestAxialHit = std::min_element(track->begin(), track->end(), [&reconstructedS](const CDCRecoHit3D & lhs,
  const CDCRecoHit3D & rhs) {
    return std::abs(lhs.getArcLength2D() - reconstructedS) < std::abs(rhs.getArcLength2D() - reconstructedS);
  });

  var<named("track_size")>() = size;
  setVariableIfNotNaN<named("pt")>(trajectory2D.getAbsMom2D());
  setVariableIfNotNaN<named("reco_s")>(reconstructedS);
  setVariableIfNotNaN<named("reco_z")>(reconstructedPosition.z());
  setVariableIfNotNaN<named("phi_track")>(startMomentum.phi());
  var<named("phi_hit")>() = reconstructedPosition.phi();
  var<named("theta_hit")>() = reconstructedPosition.theta();
  var<named("drift_length")>() = reconstructedDriftLength;
  var<named("adc_count")>() = adcCount;
  setVariableIfNotNaN<named("xy_distance_zero_z")>(xyDistance);
  var<named("right_hit")>() = rlInformation == ERightLeft::c_Right;
  setVariableIfNotNaN<named("track_back_s")>(backArcLength2D);
  setVariableIfNotNaN<named("track_front_s")>(frontArcLength2D);
  setVariableIfNotNaN<named("track_mean_s")>(arcLength2DSum / size);
  setVariableIfNotNaN<named("s_distance")>(nearestAxialHit->getArcLength2D() - reconstructedS);
  setVariableIfNotNaN<named("track_radius")>(radius);
  var<named("superlayer_id")>() = recoHit->getISuperLayer();
  return true;
}
