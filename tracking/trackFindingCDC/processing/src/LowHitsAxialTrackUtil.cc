/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/processing/LowHitsAxialTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void LowHitsAxialTrackUtil::addCandidateFromHits(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                 std::vector<CDCTrack>& axialTracks,
                                                 bool fromOrigin,
                                                 bool straight,
                                                 bool withPostprocessing)
{
  if (foundAxialWireHits.empty()) return;

  // New track
  CDCTrack track;

  // Fit trajectory
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter(fromOrigin, straight);
  CDCTrajectory2D trajectory2D = fitter.fit(foundAxialWireHits);
  track.setStartTrajectory3D(CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption()));

  // Reconstruct and add hits
  for (const CDCWireHit* wireHit : foundAxialWireHits) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trajectory2D);
    track.push_back(std::move(recoHit3D));
  }
  track.sortByArcLength2D();

  // Change everything again in the postprocessing, if desired
  bool success = withPostprocessing ? postprocessTrack(track, allAxialWireHits) : true;
  if (success) {
    axialTracks.emplace_back(std::move(track));
  }
}

bool LowHitsAxialTrackUtil::postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // No postprocessing yet
  return true;
}
