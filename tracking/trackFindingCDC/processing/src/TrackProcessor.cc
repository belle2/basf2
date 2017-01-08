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
#include <tracking/trackFindingCDC/processing/TrackMerger.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackProcessor::addCandidateFromHitsWithPostprocessing(std::vector<const CDCWireHit*>& foundAxialWireHits,
                                                            const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                            std::list<CDCTrack>& cdcTrackList)
{
  if (foundAxialWireHits.size() == 0) return;

  // New track
  CDCTrack track;

  // Fit trajectory
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trajectory2D = fitter.fit(foundAxialWireHits);
  track.setStartTrajectory3D(CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption()));

  // Reconstruct and add hits
  for (const CDCWireHit* wireHit : foundAxialWireHits) {
    AutomatonCell& automatonCell = wireHit->getAutomatonCell();
    if (automatonCell.hasTakenFlag() or automatonCell.hasMaskedFlag()) continue;

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
    cdcTrackList.emplace_back(std::move(track));
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

  for (int iPass = 0; iPass < 2; ++iPass) {
    HitProcessor::splitBack2BackTrack(track);
    TrackQualityTools::normalizeTrack(track);
    if (not checkTrackQuality(track)) {
      return false;
    }

    HitProcessor::deleteHitsFarAwayFromTrajectory(track);
    TrackQualityTools::normalizeTrack(track);
    if (not checkTrackQuality(track)) {
      return false;
    }

    HitProcessor::assignNewHitsToTrack(track, allAxialWireHits);
    TrackQualityTools::normalizeTrack(track);
  }

  HitProcessor::unmaskHitsInTrack(track);
  return true;
}

bool TrackProcessor::checkTrackQuality(const CDCTrack& track)
{
  return not(track.size() < 5);
}

void TrackProcessor::assignNewHits(const std::vector<const CDCWireHit*>& allAxialWireHits,
                                   std::list<CDCTrack>& cdcTrackList)
{
  cdcTrackList.erase(std::remove_if(cdcTrackList.begin(),
                                    cdcTrackList.end(),
  [](const CDCTrack & track) { return track.size() == 0; }),
  cdcTrackList.end());
  for (CDCTrack& track : cdcTrackList) {
    if (track.size() < 4) continue;

    HitProcessor::assignNewHitsToTrack(track, allAxialWireHits);
    TrackQualityTools::normalizeTrack(track);

    std::vector<const CDCWireHit*> splittedHits = HitProcessor::splitBack2BackTrack(track);
    TrackQualityTools::normalizeTrack(track);

    addCandidateFromHitsWithPostprocessing(splittedHits, allAxialWireHits, cdcTrackList);

    HitProcessor::deleteHitsFarAwayFromTrajectory(track);
    TrackQualityTools::normalizeTrack(track);
  }


  // TODO: HitProcessor::reassignHitsFromOtherTracks(cdcTrackList);
  for (CDCTrack& cand : cdcTrackList) {
    cand.forwardTakenFlag();
  }
}

void TrackProcessor::mergeAndFinalizeTracks(std::list<CDCTrack>& cdcTrackList,
                                            const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // Check quality of the track basing on holes on the trajectory;
  // if holes exsist then track is splitted
  for (CDCTrack& track : cdcTrackList) {
    if (track.size() > 3) {
      HitProcessor::maskHitsWithPoorQuality(track);
      HitProcessor::splitBack2BackTrack(track);

      TrackQualityTools::normalizeTrack(track);
      std::vector<const CDCWireHit*> hitsToSplit;

      for (CDCRecoHit3D& hit : track) {
        if (hit.getWireHit().getAutomatonCell().hasMaskedFlag()) {
          hitsToSplit.push_back(&(hit.getWireHit()));
        }
      }

      HitProcessor::deleteAllMarkedHits(track);

      for (const CDCWireHit* hit : hitsToSplit) {
        hit->getAutomatonCell().setMaskedFlag(false);
        hit->getAutomatonCell().setTakenFlag(false);
      }

      TrackProcessor::addCandidateFromHitsWithPostprocessing(hitsToSplit, allAxialWireHits, cdcTrackList);

    }
  }

  // Update tracks before storing to DataStore
  for (CDCTrack& track : cdcTrackList) {
    TrackQualityTools::normalizeTrack(track);
  };

  // Remove bad tracks
  TrackProcessor::deleteTracksWithLowFitProbability(cdcTrackList);

  // Perform tracks merging
  TrackMerger::doTracksMerging(cdcTrackList, allAxialWireHits);

  // Assign new hits
  TrackProcessor::assignNewHits(allAxialWireHits, cdcTrackList);
}

void TrackProcessor::deleteTracksWithLowFitProbability(std::list<CDCTrack>& cdcTrackList, double minimal_probability_for_good_fit)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
  const auto lowPValue = [&](const CDCTrack & track) {
    CDCTrajectory2D fittedTrajectory = trackFitter.fit(track);
    return not(fittedTrajectory.getPValue() >= minimal_probability_for_good_fit);
  };
  erase_remove_if(cdcTrackList, lowPValue);
}

bool TrackProcessor::isChi2InQuantiles(CDCTrack& track, double lower_quantile, double upper_quantile)
{
  double pValue = track.getStartTrajectory3D().getPValue();
  return (lower_quantile <= pValue) and (pValue <= upper_quantile);
}

double TrackProcessor::calculateChi2ForQuantile(double alpha, double n)
{
  return TMath::ChisquareQuantile(alpha, n);
  /*
  double d;
  if (alpha > 0.5) {
    d = 2.0637 * pow(log(1. / (1. - alpha)) - 0.16, 0.4274) - 1.5774;
  } else {
    d = -2.0637 * pow(log(1. / alpha) - 0.16, 0.4274) + 1.5774;
  }

  const double A = d * pow(2., 0.5);
  const double B = 2.*(d * d - 1.) / 3.;
  const double C = d * (d * d - 7) / (9. * pow(2., 0.5));
  const double D = (6.*d * d * d * d + 14 * d * d - 32) / 405.;
  const double E = d * (9 * d * d * d * d + 256 * d * d - 433) / (4860. * pow(2., 0.5));

  return n + A * pow(n, 0.5) + B + C / pow(n, 0.5) + D / n + E / (n * pow(n, 0.5));
  */
}
