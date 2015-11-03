/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackMerger.h>
#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void TrackMerger::mergeTracks(CDCTrack& track1, CDCTrack& track2, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  if (track1 == track2) return;

  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  for (const CDCRecoHit3D& hit : track2) {
    const CDCRecoHit3D& cdcRecoHit3D  =  CDCRecoHit3D::reconstruct(hit.getRLWireHit(), track1.getStartTrajectory3D().getTrajectory2D());

    track1.push_back(std::move(cdcRecoHit3D));
  }
  track2.clear();

  trackQualityTools.normalizeTrack(track1);

  std::vector<const CDCWireHit*> removedHits = HitProcessor::splitBack2BackTrack(track1);

  trackQualityTools.normalizeTrack(track1);

  m_trackProcessor.addCandidateWithHits(removedHits, conformalCDCWireHitList);

}

void TrackMerger::resetHits(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track.items()) {
    hit->getWireHit().getAutomatonCell().setTakenFlag(true);
    hit->getWireHit().getAutomatonCell().setMaskedFlag(false);
  }
}


void TrackMerger::doTracksMerging(std::list<CDCTrack>& trackList, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getFitter();
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  // Search for best matches
  for (std::list<CDCTrack>::iterator it1 = trackList.begin(); it1 !=  trackList.end(); ++it1) {
    std::list<CDCTrack>::iterator it2 = it1;
    ++it2;
    CDCTrack& track1 = *it1;
    double prob = 0;
    CDCTrack* bestCandidate = nullptr;

    for (; it2 !=  trackList.end(); ++it2) {

      CDCTrack& track2 = *it2;

      resetHits(track1);
      resetHits(track2);

      double probTemp = doTracksFitTogether(track1, track2);

      resetHits(track1);
      resetHits(track2);

      if (probTemp > prob) {
        prob = probTemp;
        bestCandidate = &track2;
      }

    }

    if (prob > m_minimum_probability_to_be_merged) {
      mergeTracks(track1, *bestCandidate, conformalCDCWireHitList);
      trackQualityTools.normalizeTrack(*bestCandidate);
//      trackFitter.fitTrackCandidateFast(bestCandidate); <----- TODO
    }
  }

  trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
  [&](CDCTrack & track) { return (track.size() == 0); }),
  trackList.end());

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

    resetHits(trackToBeMerged);
    resetHits(track2);

    if (probabilityToBeMerged < probabilityTemp) {
      probabilityToBeMerged = probabilityTemp;
      candidateToMergeBest = &track2;
    }
  }

  return std::make_pair(candidateToMergeBest, probabilityToBeMerged);
}


void TrackMerger::tryToMergeTrackWithOtherTracks(CDCTrack& track, std::list<CDCTrack>& trackList,
                                                 const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getFitter();
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  B2DEBUG(100, "Merger: Initial nCands = " << trackList.size());

  bool have_merged_something;

  do {
    have_merged_something = false;
    BestMergePartner candidateToMergeBest = calculateBestTrackToMerge(track, trackList.begin(), trackList.end());
    Probability& probabilityWithCandidate = candidateToMergeBest.second;

    if (probabilityWithCandidate > m_minimum_probability_to_be_merged) {
      CDCTrack* bestFitTrackCandidate = candidateToMergeBest.first;
      mergeTracks(track, *bestFitTrackCandidate, conformalCDCWireHitList);
      have_merged_something = true;
    }

    B2DEBUG(100, "Cand hits vector size = " << track.size());

    trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
    [&](CDCTrack & track) { return (track.size() < 3); }),
    trackList.end());

  } while (have_merged_something);

  B2DEBUG(100, "Merger: Resulting nCands = " << trackList.size());

  for (CDCTrack& track : trackList) {

    resetHits(track);
    trackQualityTools.normalizeTrack(track);

//    trackFitter.fitTrackCandidateFast(cand); <<------ TODO
//    cand->reestimateCharge();
  }

}


void TrackMerger::removeStrangeHits(double factor, std::vector<const CDCWireHit*>& wireHits, CDCTrajectory2D& trajectory)
{

  // Maybe it is better to use the assignment probability here also? -> SimpleFilter
  for (const CDCWireHit* hit : wireHits) {

    double dist = fabs(fabs(trajectory.getDist2D(hit->getRefPos2D())) - fabs(hit->getRefDriftLength()));

    if (dist > hit->getRefDriftLength() * factor) {
      hit->getAutomatonCell().setMaskedFlag(true);
    }
  }
  wireHits.erase(std::remove_if(wireHits.begin(), wireHits.end(),
  [&](const CDCWireHit * hit) {
    return hit->getAutomatonCell().hasMaskedFlag();
  }), wireHits.end());

}

double TrackMerger::doTracksFitTogether(CDCTrack& track1, CDCTrack& track2)
{
  CDCKarimakiFitter trackFitter;
  /*
    // Check if the two tracks do have something in common!
    Vector2D deltaMomentum{track1.getStartTrajectory3D().getTrajectory2D().getStartMom2D() - track2.getStartTrajectory3D().getTrajectory2D().getStartMom2D()};
    Vector3D deltaPosition{track1.getStartTrajectory3D().getTrajectory2D().getGlobalPerigee() - track2.getStartTrajectory3D().getTrajectory2D().getGlobalPerigee()};

    // Quick processing: if we have two tracks with approximately the same parameters, they can be merged!
    if (deltaMomentum.norm() / track1.getStartTrajectory3D().getTrajectory2D().getStartMom2D().norm() < 0.1 and
        std::abs(TVector2::Phi_mpi_pi(track1.getStartTrajectory3D().getTrajectory2D().getStartMom2D().phi() - track2.getStartTrajectory3D().getTrajectory2D().getStartMom2D().phi())) < TMath::Pi() / 10 and
        deltaPosition.norm() < 10 and track1.getStartChargeSign() == track2.getStartChargeSign()) {
      return 1;
    }
  */
  // Build common hit list
  std::vector<const CDCWireHit*> commonHitListOfTwoTracks;
  for (const CDCRecoHit3D& hit : track1) {
    commonHitListOfTwoTracks.push_back(&(hit.getWireHit()));
  }
  for (const CDCRecoHit3D& hit : track2) {
    commonHitListOfTwoTracks.push_back(&(hit.getWireHit()));
  }

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it does the job.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  commonHitListOfTwoTracks.erase(
    std::unique(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end()),
    commonHitListOfTwoTracks.end());

  for (const CDCWireHit* hit : commonHitListOfTwoTracks) {
    hit->getAutomatonCell().setTakenFlag(true);
    hit->getAutomatonCell().setMaskedFlag(false);
  }

  // Calculate track parameters
  CDCTrajectory2D commonTrajectory;

  const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getFitter();

  // Approach the best fit
  commonTrajectory = fitter.fitWhithoutDriftLengthVariance(commonHitListOfTwoTracks);
  removeStrangeHits(5, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fitWhithoutDriftLengthVariance(commonHitListOfTwoTracks);
  removeStrangeHits(3, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fitWhithoutDriftLengthVariance(commonHitListOfTwoTracks);
  removeStrangeHits(1, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fitWhithoutDriftLengthVariance(commonHitListOfTwoTracks);
  removeStrangeHits(1, commonHitListOfTwoTracks, commonTrajectory);
  commonTrajectory = fitter.fitWhithoutDriftLengthVariance(commonHitListOfTwoTracks);

//TODO: perform B2B tracks check
//if B2B return 0;

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller than the two lists before or if the list is too small
  if (commonHitListOfTwoTracks.size() <= max(track1.size(), track2.size())
      or commonHitListOfTwoTracks.size() < 15) {
    return 0;
  }

  return TMath::Prob(commonTrajectory.getChi2(), commonTrajectory.getNDF());
}
