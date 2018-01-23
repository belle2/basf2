/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrackVarSet::extract(const BaseSegmentTrackFilter::Object* testPair)
{
  const CDCTrack* track = testPair->getFrom();
  const CDCSegment2D* segment = testPair->getTo();

  assert(segment);
  assert(track);

  double maxmimumTrajectoryDistanceFront = 0;
  double maxmimumTrajectoryDistanceBack = 0;
  double maxmimumHitDistanceFront = 0;
  double maxmimumHitDistanceBack = 0;
  double outOfCDC = 0; // 0 means no, 1 means yes
  double hitsInSameRegion = 0;
  double hitsInCommon = 0;

  const CDCRecoHit2D& front = segment->front();
  const CDCRecoHit2D& back = segment->back();

  // Calculate distances
  const CDCTrajectory3D& trajectoryTrack3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectoryTrack2D = trajectoryTrack3D.getTrajectory2D();
  const CDCTrajectorySZ& szTrajectoryTrack = trajectoryTrack3D.getTrajectorySZ();
  double radius = trajectoryTrack2D.getGlobalCircle().absRadius();

  maxmimumTrajectoryDistanceFront = trajectoryTrack2D.getDist2D(front.getWireHit().getRefPos2D());
  maxmimumTrajectoryDistanceBack = trajectoryTrack2D.getDist2D(back.getWireHit().getRefPos2D());

  var<named("z_distance")>() = 0;
  var<named("theta_distance")>() = 0;

  if (segment->getStereoKind() == EStereoKind::c_Axial) {
    CDCTrajectory2D& trajectorySegment = segment->getTrajectory2D();
    if (not trajectoryTrack2D.isFitted()) {
      const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
      fitter.update(trajectorySegment, *segment);
    }
  } else {
    CDCObservations2D observations;
    for (const CDCRecoHit2D& recoHit : *segment) {
      const CDCRLWireHit& rlWireHit = recoHit.getRLWireHit();
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectoryTrack2D);
      double s = recoHit3D.getArcLength2D();
      double z = recoHit3D.getRecoZ();
      observations.fill(s, z);
    }

    if (observations.size() > 3) {
      const CDCSZFitter& fitter = CDCSZFitter::getFitter();
      const CDCTrajectorySZ& szTrajectorySegments = fitter.fit(observations);

      double startZTrack = szTrajectoryTrack.getZ0();
      double startZSegments = szTrajectorySegments.getZ0();

      var<named("z_distance")>() = startZTrack - startZSegments;
      var<named("theta_distance")>() = szTrajectoryTrack.getTanLambda() - szTrajectorySegments.getTanLambda();
    }
  }

  // Calculate if it is out of the CDC
  Vector3D frontRecoPos3D = front.reconstruct3D(trajectoryTrack2D);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectoryTrack2D);

  if (segment->getStereoKind() != EStereoKind::c_Axial) {
    double forwardZ = front.getWire().getWireLine().forwardZ();
    double backwardZ = front.getWire().getWireLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ or backRecoPos3D.z() > forwardZ
        or backRecoPos3D.z() < backwardZ) {
      outOfCDC = 1.0;
    }
  }

  // Get perpS of track in the beginning and the end
  double perpSOfFront = trajectoryTrack2D.calcArcLength2D(segment->front().getRecoPos2D());
  double perpSOfBack = trajectoryTrack2D.calcArcLength2D(segment->back().getRecoPos2D());

  double perpSMinimum = std::min(perpSOfFront, perpSOfBack);
  double perpSMaximum = std::max(perpSOfFront, perpSOfBack);

  // Count number of hits in the same region
  for (const CDCRecoHit3D& recoHit : *track) {
    if (recoHit.getArcLength2D() < 0.8 * perpSMinimum or
        recoHit.getArcLength2D() > 1.2 * perpSMaximum) {
      continue;
    }
    if (recoHit.getISuperLayer() == segment->getISuperLayer()) {
      hitsInSameRegion++;
    } else if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) == 1) {
      double distanceFront = (front.getWireHit().getRefPos2D() - recoHit.getRecoPos2D()).norm();
      if (distanceFront > maxmimumHitDistanceFront) {
        maxmimumHitDistanceFront = distanceFront;
      }
      double distanceBack = (back.getWireHit().getRefPos2D() - recoHit.getRecoPos2D()).norm();
      if (distanceBack > maxmimumHitDistanceBack) {
        maxmimumHitDistanceBack = distanceBack;
      }
    }
  }

  // Count number of common hits
  for (const CDCRecoHit3D& trackHit : *track) {
    if (std::find_if(segment->begin(), segment->end(), [&trackHit](const CDCRecoHit2D & segmentHit) {
    return segmentHit.getWireHit().getHit() == trackHit.getWireHit().getHit();
    }) != segment->end()) {
      hitsInCommon += 1;
    }
  }

  // Make a fit with all the hits and one with only the hits in the near range
  CDCObservations2D observationsFull;
  CDCObservations2D observationsNeigh;

  // Collect the observations
  bool isAxialSegment = segment->getStereoKind() != EStereoKind::c_Axial;

  for (const CDCRecoHit3D& recoHit : *track) {
    if (isAxialSegment and recoHit.getStereoKind() == EStereoKind::c_Axial) {
      observationsFull.fill(recoHit.getWireHit().getRefPos2D());
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.fill(recoHit.getWireHit().getRefPos2D());
      }
    } else if (not isAxialSegment and recoHit.getStereoKind() != EStereoKind::c_Axial) {
      double s = recoHit.getArcLength2D();
      double z = recoHit.getRecoZ();
      observationsFull.fill(s, z);
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.fill(s, z);
      }
    }
  }

  const CDCTrajectorySZ& trajectorySZ = track->getStartTrajectory3D().getTrajectorySZ();
  double tanLambda = trajectorySZ.getTanLambda();

  bool hasZInformation = tanLambda != 0;
  double max_hit_z_distance = -1;
  double sum_hit_z_distance = 0;
  double stereo_quad_tree_distance = 0;

  if (hasZInformation) {
    double thetaFirstSegmentHit = -10;

    for (const CDCRecoHit2D& recoHit2D : *segment) {
      Vector3D reconstructedPosition = recoHit2D.reconstruct3D(trajectoryTrack2D);
      const Vector2D& recoPos2D = recoHit2D.getRecoPos2D();
      double perpS = trajectoryTrack2D.calcArcLength2D(recoPos2D);


      double current_z_distance = std::abs(trajectorySZ.getZDist(perpS, reconstructedPosition.z()));
      if (std::isnan(current_z_distance)) {
        continue;
      }

      if (thetaFirstSegmentHit == -10) {
        thetaFirstSegmentHit = reconstructedPosition.theta();
      }
      sum_hit_z_distance += current_z_distance;
      if (current_z_distance > max_hit_z_distance) {
        max_hit_z_distance = current_z_distance;
      }
    }

    double thetaTrack = trajectoryTrack3D.getFlightDirection3DAtSupport().theta();
    stereo_quad_tree_distance = thetaTrack - thetaFirstSegmentHit;
  }


  for (const CDCRecoHit2D& recoHit : *segment) {
    if (isAxialSegment) {
      observationsFull.fill(recoHit.getRecoPos2D());
      observationsNeigh.fill(recoHit.getRecoPos2D());
    } else {
      const CDCRLWireHit& rlWireHit = recoHit.getRLWireHit();
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectoryTrack2D);
      double s = recoHit3D.getArcLength2D();
      double z = recoHit3D.getRecoZ();
      observationsFull.fill(s, z);
      observationsNeigh.fill(s, z);
    }
  }

  // Do the fit
  var<named("fit_neigh")>() = 0;
  var<named("fit_full")>() = 0;
  if (segment->getStereoKind() == EStereoKind::c_Axial) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    var<named("fit_full")>() = fitter.fit(observationsFull).getPValue();
  } else {
    const CDCSZFitter& fitter = CDCSZFitter::getFitter();
    var<named("fit_full")>() = toFinite(fitter.fit(observationsFull).getPValue(), 0);

    if (observationsNeigh.size() > 3) {
      var<named("fit_neigh")>() = toFinite(fitter.fit(observationsNeigh).getPValue(), 0);
    } else {
      var<named("fit_neigh")>() = 0;
    }
  }

  if (observationsFull.size() == observationsNeigh.size()) {
    var<named("fit_neigh")>() = -1;
  }

  var<named("is_stereo")>() = segment->getStereoKind() != EStereoKind::c_Axial;
  var<named("segment_size")>() = segment->size();
  var<named("track_size")>() = track->size();
  var<named("mean_hit_z_distance")>() = sum_hit_z_distance;
  var<named("max_hit_z_distance")>() = max_hit_z_distance;
  var<named("stereo_quad_tree_distance")>() = toFinite(stereo_quad_tree_distance, 0);

  var<named("pt_of_track")>() = toFinite(std::isnan(trajectoryTrack2D.getAbsMom2D()) ? 0.0 : trajectoryTrack2D.getAbsMom2D(), 0);
  var<named("track_is_curler")>() = trajectoryTrack2D.getExit().hasNAN();

  var<named("superlayer_already_full")>() = not trajectoryTrack2D.getOuterExit().hasNAN() and hitsInSameRegion > 5;

  var<named("maxmimum_trajectory_distance_front")>() = toFinite(maxmimumTrajectoryDistanceFront, 999);
  var<named("maxmimum_trajectory_distance_back")>() = toFinite(maxmimumTrajectoryDistanceBack, 999);

  var<named("maxmimum_hit_distance_front")>() = maxmimumHitDistanceFront;
  var<named("maxmimum_hit_distance_back")>() = maxmimumHitDistanceBack;

  var<named("out_of_CDC")>() = outOfCDC;
  var<named("hits_in_same_region")>() = hitsInSameRegion;

  var<named("number_of_hits_in_common")>() = hitsInCommon;

  var<named("segment_super_layer")>() = segment->getISuperLayer();

  double phiBetweenTrackAndSegment = trajectoryTrack2D.getMom2DAtSupport().angleWith(segment->front().getRecoPos2D());

  var<named("phi_between_track_and_segment")>() = toFinite(phiBetweenTrackAndSegment, 0);
  var<named("perp_s_of_front")>() = toFinite(perpSOfFront / radius, 0);
  var<named("perp_s_of_back")>() = toFinite(perpSOfBack / radius, 0);

  return true;
}
