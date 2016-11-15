/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <numeric>

using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoSegmentVarSet::extract(const std::pair<const CDCSegment3D*, const CDCTrack*>* testPair)
{
  if (not testPair) return false;
  const CDCSegment3D& segment3D = *testPair->first;
  const CDCTrack& track = *testPair->second;

  if (track.size() == 0) {
    return false;
  }

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = track.getStartTrajectory3D().getTrajectorySZ();
  const bool isCurler = trajectory2D.isCurler();

  const Vector2D& startMomentum = trajectory2D.getMom2DAtSupport();
  const double radius = trajectory2D.getLocalCircle()->radius();
  const double size = track.size();

  const double backArcLength2D = track.back().getArcLength2D();
  const double frontArcLength2D = track.front().getArcLength2D();
  const double arcLength2DSum =
    std::accumulate(track.begin(),
                    track.end(),
                    0.0,
  [](const double sum, const CDCRecoHit3D & listRecoHit) {
    return sum + listRecoHit.getArcLength2D();
  });

  // Count the number of hits with reconstruction position out of the CDC
  unsigned int numberOfHitsOutOfCDC = 0;
  unsigned int numberOfHitsOnWrongSide = 0;
  double sumDistanceZReconstructed2D = 0;
  double sumDistance2DReconstructedZ = 0;

  std::vector<double> arcLength2DList(segment3D.size());

  for (const CDCRecoHit3D& recoHit3D : segment3D.items()) {
    const CDCWire& wire = recoHit3D.getWire();
    const Vector3D& reconstructedPositionTo2D = recoHit3D.getRecoPos3D();

    if (not wire.isInCellZBounds(reconstructedPositionTo2D)) {
      numberOfHitsOutOfCDC++;
    }

    double arcLength2D = recoHit3D.getArcLength2D();
    if (arcLength2D < 0 and not isCurler) {
      numberOfHitsOnWrongSide++;
      arcLength2D += 2 * TMath::Pi() * radius;
    }

    sumDistanceZReconstructed2D += std::fabs(trajectorySZ.getZDist(arcLength2D, reconstructedPositionTo2D.z()));
    arcLength2DList.push_back(arcLength2D);

    const double reconstructedZ = trajectorySZ.mapSToZ(arcLength2D);
    const Vector2D& reconstructedPositionToZ = recoHit3D.getWire().getWirePos2DAtZ(reconstructedZ);
    sumDistance2DReconstructedZ += (reconstructedPositionToZ - reconstructedPositionTo2D.xy()).norm();
  }

  std::sort(arcLength2DList.begin(), arcLength2DList.end());

  const double minimumArcLength2D = arcLength2DList.front();
  const double maximumArcLength2D = arcLength2DList.back();

  size_t numberOfHitsInSameRegion =
  std::count_if(track.begin(), track.end(), [&](const CDCRecoHit3D & recoHit) -> bool {
    return recoHit.getArcLength2D() < maximumArcLength2D and
    recoHit.getArcLength2D() > minimumArcLength2D;
  });

  ////////

  var<named("track_size")>() = size;
  var<named("segment_size")>() = segment3D.size();
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);
  var<named("phi_track")>() = toFinite(startMomentum.phi(), 0);

  var<named("segment_back_s")>() = toFinite(maximumArcLength2D, 0);
  var<named("segment_front_s")>() = toFinite(minimumArcLength2D, 0);
  var<named("track_mean_s")>() = toFinite(arcLength2DSum / size, 0);
  var<named("track_radius")>() = toFinite(radius, 0);

  var<named("number_of_hits_in_same_region")>() = numberOfHitsInSameRegion;
  var<named("number_of_hits_out_of_cdc")>() = numberOfHitsOutOfCDC;
  var<named("number_of_hits_on_wrong_side")>() = numberOfHitsOnWrongSide;
  //var<named("number_of_hits_in_common")>() = numberOfHitsInCommon;

  var<named("sum_distance_using_2d")>() = toFinite(sumDistanceZReconstructed2D, 0);
  var<named("sum_distance_using_z")>() = toFinite(sumDistance2DReconstructedZ, 0);

  var<named("superlayer_id")>() = segment3D.getISuperLayer();

  var<named("track_front_s")>() = toFinite(frontArcLength2D, 0);
  var<named("track_back_s")>() = toFinite(backArcLength2D, 0);
  return true;
}
