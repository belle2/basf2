/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/processing/TrackProcessor.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackProcessor::addCandidateFromHitsWithPostprocessing(const std::vector<const CDCWireHit*>& foundAxialWireHits,
                                                            const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                            std::vector<CDCTrack>& axialTracks)
{
  if (foundAxialWireHits.empty()) return;

  // New track
  CDCTrack track;

  // Fit trajectory
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
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

  // Change everything again in the postprocessing
  bool success = postprocessTrack(track, allAxialWireHits);
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

bool TrackProcessor::postprocessTrack(CDCTrack& track, const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::deleteHitsFarAwayFromTrajectory(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track)) {
    return false;
  }

  HitProcessor::assignNewHitsToTrack(track, allAxialWireHits);
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::splitBack2BackTrack(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track)) {
    return false;
  }
  return true;
}

bool TrackProcessor::checkTrackQuality(const CDCTrack& track)
{
  return not(track.size() < 5);
}

void TrackProcessor::deleteTracksWithLowFitProbability(std::vector<CDCTrack>& axialTracks,
                                                       double minimal_probability_for_good_fit)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
  const auto lowPValue = [&](const CDCTrack & track) {
    CDCTrajectory2D fittedTrajectory = trackFitter.fit(track);
    // Keep good fits - p-value is not really a probability,
    // but what can you do if the original author did not mind...
    if (not(fittedTrajectory.getPValue() >= minimal_probability_for_good_fit)) {
      // Release hits
      track.forwardTakenFlag(false);
      return true;
    }
    return false;
  };
  erase_remove_if(axialTracks, lowPValue);
}

void TrackProcessor::deleteShortTracks(std::vector<CDCTrack>& axialTracks, double minimal_size)
{
  const auto isShort = [&](const CDCTrack & track) {
    if (track.size() < minimal_size) {
      // Release hits
      track.forwardTakenFlag(false);
      return true;
    }
    return false;
  };
  erase_remove_if(axialTracks, isShort);
}
