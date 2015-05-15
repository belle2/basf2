/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrackVarSet::extract(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>* testPair)
{
  extractNested(testPair);
  const CDCRecoSegment2D* segment = testPair->first;
  const CDCTrack* track = testPair->second;

  double maxmimumHitDistanceFront = 0;
  double maxmimumHitDistanceBack = 0;
  double outOfCDC = 0; // 0 means no, 1 means yes
  double hitsInSameRegion = 0;

  const CDCRecoHit2D& front = segment->front();
  const CDCRecoHit2D& back = segment->back();

  // Calculate distance
  const CDCTrajectory2D& trajectory = track->getStartTrajectory3D().getTrajectory2D();

  maxmimumHitDistanceFront = trajectory.getDist2D(front.getWireHit().getRefPos2D());
  maxmimumHitDistanceBack = trajectory.getDist2D(back.getWireHit().getRefPos2D());

  // Calculate if it is out of the CDC
  Vector3D frontRecoPos3D = front.reconstruct3D(trajectory);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectory);

  if (segment->getStereoType() != AXIAL) {
    double forwardZ = front.getWire().getSkewLine().forwardZ();
    double backwardZ = front.getWire().getSkewLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ or backRecoPos3D.z() > forwardZ
        or backRecoPos3D.z() < backwardZ) {
      outOfCDC = 1.0;
    }
  }

  // Count number of hits in the same region
  for (const CDCRecoHit3D& recoHit : track->items()) {
    if (recoHit.getISuperLayer() == segment->getISuperLayer()) {
      hitsInSameRegion++;
    }
  }

  // Make a fit with all the hits and one with only the hits in the near range
  CDCObservations2D observationsFull;
  CDCObservations2D observationsNeigh;

  // Collect the observations
  bool isAxialSegment = segment->getStereoType() != AXIAL;

  for (const CDCRecoHit3D& recoHit : track->items()) {
    if (isAxialSegment and recoHit.getStereoType() == AXIAL) {
      observationsFull.append(recoHit.getWireHit().getRefPos2D());
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.append(recoHit.getWireHit().getRefPos2D());
      }
    } else if (not isAxialSegment and recoHit.getStereoType() != AXIAL) {
      double s = recoHit.getPerpS();
      double z = recoHit.getRecoZ();
      observationsFull.append(s, z);
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.append(s, z);
      }
    }
  }


  for (const CDCRecoHit2D& recoHit : segment->items()) {
    if (isAxialSegment) {
      observationsFull.append(recoHit.getRecoPos2D());
      observationsNeigh.append(recoHit.getRecoPos2D());
    } else {
      CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory);
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observationsFull.append(s, z);
      observationsNeigh.append(s, z);
    }
  }

  if (observationsFull.size() == observationsNeigh.size()) {
    var<named("fit_full")>() = var<named("fit_neigh")>() = -1;
  } else {
    // Do the fit
    if (segment->getStereoType() == AXIAL) {
      const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
      var<named("fit_full")>() = fitter.fit(observationsFull).getProb();
      var<named("fit_neigh")>() = fitter.fit(observationsNeigh).getProb();
    } else {
      const CDCSZFitter& fitter = CDCSZFitter::getFitter();
      var<named("fit_full")>() = fitter.fit(observationsFull).getProb();
      var<named("fit_neigh")>() = fitter.fit(observationsNeigh).getProb();
    }
  }


  var<named("is_stereo")>() = segment->getStereoType() != AXIAL;
  var<named("segment_size")>() = segment->size();
  var<named("track_size")>() = track->size();

  var<named("pt_of_track")>() = std::isnan(trajectory.getAbsMom2D()) ? 0.0 : trajectory.getAbsMom2D();
  var<named("track_is_curler")>() = trajectory.getExit().hasNAN();

  var<named("superlayer_already_full")>() = not trajectory.getOuterExit().hasNAN() and hitsInSameRegion > 5;

  if (std::isnan(maxmimumHitDistanceFront)) {
    var<named("maxmimum_hit_distance_front")>() = 999;
  } else {
    var<named("maxmimum_hit_distance_front")>() = maxmimumHitDistanceFront;
  }

  if (std::isnan(maxmimumHitDistanceBack)) {
    var<named("maxmimum_hit_distance_back")>() = 999;
  } else {
    var<named("maxmimum_hit_distance_back")>() = maxmimumHitDistanceBack;
  }

  var<named("out_of_CDC")>() = outOfCDC;
  var<named("hits_in_same_region")>() = hitsInSameRegion;

  return true;
}
