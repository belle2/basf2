/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/processing/TrackMerger.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <TMath.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackMerger::mergeTracks(CDCTrack& track1,
                              CDCTrack& track2,
                              const std::vector<const CDCWireHit*>& allAxialWireHits,
                              std::list<CDCTrack>& cdcTrackList)
{
  if (track1 == track2) return;

  for (const CDCRecoHit3D& hit : track2) {
    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(hit.getRLWireHit(), track1.getStartTrajectory3D().getTrajectory2D());

    track1.push_back(std::move(cdcRecoHit3D));
  }
  track2.clear();

  TrackQualityTools::normalizeTrack(track1);

  std::vector<const CDCWireHit*> splittedHits = HitProcessor::splitBack2BackTrack(track1);

  TrackQualityTools::normalizeTrack(track1);

  TrackProcessor::addCandidateFromHitsWithPostprocessing(splittedHits, allAxialWireHits, cdcTrackList);

}


void TrackMerger::doTracksMerging(std::list<CDCTrack>& cdcTrackList,
                                  const std::vector<const CDCWireHit*>& allAxialWireHits,
                                  double minimum_probability_to_be_merged)
{
  // Search for best matches
  for (std::list<CDCTrack>::iterator it1 = cdcTrackList.begin(); it1 !=  cdcTrackList.end(); ++it1) {
    std::list<CDCTrack>::iterator it2 = it1;
    ++it2;
    CDCTrack& track1 = *it1;
    double prob = 0;
    CDCTrack* bestCandidate = nullptr;

    for (; it2 !=  cdcTrackList.end(); ++it2) {

      CDCTrack& track2 = *it2;

      double probTemp = doTracksFitTogether(track1, track2);

      if (probTemp > prob) {
        prob = probTemp;
        bestCandidate = &track2;
      }

    }

    if (bestCandidate and prob > minimum_probability_to_be_merged) {
      mergeTracks(track1, *bestCandidate, allAxialWireHits, cdcTrackList);
      TrackQualityTools::normalizeTrack(*bestCandidate);
//      const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getFitter();
//      trackFitter.fitTrackCandidateFast(bestCandidate); <----- TODO
    }
  }

  cdcTrackList.erase(std::remove_if(cdcTrackList.begin(),
                                    cdcTrackList.end(),
  [&](CDCTrack & track) { return (track.size() == 0); }),
  cdcTrackList.end());
}

TrackMerger::BestMergePartner TrackMerger::calculateBestTrackToMerge(CDCTrack& trackToBeMerged,
    std::list<CDCTrack>::iterator start_iterator, std::list<CDCTrack>::iterator end_iterator)
{
  double probabilityToBeMerged = 0;
  CDCTrack* candidateToMergeBest = nullptr;

  for (auto iterator = start_iterator; iterator != end_iterator; iterator++) {
    CDCTrack& track2 = *iterator;
    if (trackToBeMerged == track2) {
      continue;
    }


    if (track2.size() < 3) {
      continue;
    }

    double probabilityTemp = doTracksFitTogether(trackToBeMerged, track2);

    HitProcessor::unmaskHitsInTrack(trackToBeMerged);
    HitProcessor::unmaskHitsInTrack(track2);

    if (probabilityToBeMerged < probabilityTemp) {
      probabilityToBeMerged = probabilityTemp;
      candidateToMergeBest = &track2;
    }
  }

  return std::make_pair(candidateToMergeBest, probabilityToBeMerged);
}


void TrackMerger::tryToMergeTrackWithOtherTracks(CDCTrack& track,
                                                 std::list<CDCTrack>& cdcTrackList,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits,

                                                 double minimum_probability_to_be_merged)
{
  B2DEBUG(100, "Merger: Initial nCands = " << cdcTrackList.size());

  bool have_merged_something;

  do {
    have_merged_something = false;
    BestMergePartner candidateToMergeBest = calculateBestTrackToMerge(track, cdcTrackList.begin(),
                                            cdcTrackList.end());
    CDCTrack* bestFitTrackCandidate = candidateToMergeBest.first;
    double probabilityWithCandidate = candidateToMergeBest.second;

    if (bestFitTrackCandidate and probabilityWithCandidate > minimum_probability_to_be_merged) {
      mergeTracks(track, *bestFitTrackCandidate, allAxialWireHits, cdcTrackList);
      have_merged_something = true;
    }

    B2DEBUG(100, "Cand hits vector size = " << track.size());

    cdcTrackList.erase(std::remove_if(cdcTrackList.begin(),
                                      cdcTrackList.end(),
    [](CDCTrack & otherTrack) { return (otherTrack.size() < 3); }),
    cdcTrackList.end());

  } while (have_merged_something);

  B2DEBUG(100, "Merger: Resulting nCands = " << cdcTrackList.size());

  for (CDCTrack& otherTrack : cdcTrackList) {
    HitProcessor::unmaskHitsInTrack(otherTrack);
    TrackQualityTools::normalizeTrack(otherTrack);
// const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getFitter();
//    trackFitter.fitTrackCandidateFast(cand); <<------ TODO
//    cand->reestimateCharge();
  }
}

void TrackMerger::removeStrangeHits(double factor, std::vector<const CDCWireHit*>& wireHits, CDCTrajectory2D& trajectory)
{
  for (const CDCWireHit* hit : wireHits) {

    double dist = fabs(fabs(trajectory.getDist2D(hit->getRefPos2D())) - fabs(hit->getRefDriftLength()));

    if (dist > hit->getRefDriftLength() * factor) {
      hit->getAutomatonCell().setMaskedFlag(true);
    }
  }

  HitProcessor::deleteAllMarkedHits(wireHits);
}

double TrackMerger::doTracksFitTogether(CDCTrack& track1, CDCTrack& track2)
{
  HitProcessor::unmaskHitsInTrack(track1);
  HitProcessor::unmaskHitsInTrack(track2);

  // Build common hit list by copying the wire hits into one large list (we use the wire hits here as we do not want hem to bring
  // their "old" reconstructed position when fitting later.
  std::vector<const CDCWireHit*> commonHitListOfTwoTracks;
  for (const CDCRecoHit3D& hit : track1) {
    commonHitListOfTwoTracks.push_back(&(hit.getWireHit()));
  }
  for (const CDCRecoHit3D& hit : track2) {
    commonHitListOfTwoTracks.push_back(&(hit.getWireHit()));
  }

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it does the job for later unique.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  commonHitListOfTwoTracks.erase(std::unique(commonHitListOfTwoTracks.begin(),
                                             commonHitListOfTwoTracks.end()),
                                 commonHitListOfTwoTracks.end());

  // Calculate track parameters
  CDCTrajectory2D commonTrajectory;

  const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();

  // Approach the best fit
  commonTrajectory = fitter.fit(commonHitListOfTwoTracks);
  removeStrangeHits(5, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fit(commonHitListOfTwoTracks);
  removeStrangeHits(3, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fit(commonHitListOfTwoTracks);
  removeStrangeHits(1, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fit(commonHitListOfTwoTracks);
  removeStrangeHits(1, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fit(commonHitListOfTwoTracks);

  HitProcessor::unmaskHitsInTrack(track1);
  HitProcessor::unmaskHitsInTrack(track2);

//TODO: perform B2B tracks check
//if B2B return 0;

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller than the two lists before or if the list is too small
  if (commonHitListOfTwoTracks.size() <= std::max(track1.size(), track2.size())
      or commonHitListOfTwoTracks.size() < 15) {
    return 0;
  }

  return TMath::Prob(commonTrajectory.getChi2(), commonTrajectory.getNDF());
}
