/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    AutomatonCell& automatonCell = wireHit->getAutomatonCell();
    if (automatonCell.hasTakenFlag()) continue;
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trajectory2D);
    track.push_back(std::move(recoHit3D));

    automatonCell.setTakenFlag(true);
  }
  track.sortByArcLength2D();

  // Change everything again in the postprocessing, if desired
  bool success = withPostprocessing ? postprocessTrack(track, allAxialWireHits) : true;
  if (success) {
    /// Mark hits as taken and add the new track to the track list
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
    }
    axialTracks.emplace_back(std::move(track));
  } else {
    /// Masked bad hits
    for (const CDCRecoHit3D& recoHit3D : track) {
      recoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(true);
      recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
  }
}

bool LowHitsAxialTrackUtil::postprocessTrack([[maybe_unused]] CDCTrack& track, [[maybe_unused]] const
                                             std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // No postprocessing yet
  return true;
}
