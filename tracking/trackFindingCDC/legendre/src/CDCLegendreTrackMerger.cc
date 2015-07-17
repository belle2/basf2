/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void TrackMerger::mergeTracks(TrackCandidate* cand1, TrackCandidate* cand2)
{
  if (cand1 == cand2) return;

  std::vector<TrackHit*>& commonHitListOfTwoTracks = cand1->getTrackHits();
  for (TrackHit* hit : cand2->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }

  // Do delete hits in the old track cands
  cand2->getTrackHits().clear();

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it does the job.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  // Only use hits once
  commonHitListOfTwoTracks.erase(
    std::unique(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end()),
    commonHitListOfTwoTracks.end());
}

void TrackMerger::resetHits(TrackCandidate* otherTrackCandidate)
{
  for (TrackHit* hit : otherTrackCandidate->getTrackHits()) {
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }
}

void TrackMerger::doTracksMerging(std::list<TrackCandidate*>& trackList)
{
  TrackFitter trackFitter;

  // Search for best matches
  unsigned int outerCounter = 0;
  for (TrackCandidate* trackCandidate : trackList) {
    unsigned int innerCounter = 0;
    double prob = 0;
    TrackCandidate* bestCandidate = nullptr;
    for (TrackCandidate* otherTrackCandidate : trackList) {
      if (innerCounter <= outerCounter) {
        innerCounter++;
        continue;
      }

      double probTemp = doTracksFitTogether(trackCandidate, otherTrackCandidate);

      resetHits(otherTrackCandidate);
      resetHits(trackCandidate);

      if (probTemp > prob) {
        prob = probTemp;
        bestCandidate = otherTrackCandidate;
      }

      innerCounter++;
    }

    if (prob > m_minimum_probability_to_be_merged) {
      mergeTracks(bestCandidate, trackCandidate);
      trackFitter.fitTrackCandidateFast(bestCandidate);
    }

    outerCounter++;
  }

  trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
  [&](TrackCandidate * cand) { return (cand->getTrackHits().size() == 0); }),
  trackList.end());

  for (TrackCandidate* trackCandidate : trackList) {
    trackFitter.fitTrackCandidateFast(trackCandidate);
  }
}

TrackMerger::BestMergePartner TrackMerger::calculateBestTrackToMerge(TrackCandidate* trackCandidateToBeMerged,
    std::list<TrackCandidate*>::iterator start_iterator, std::list<TrackCandidate*>::iterator end_iterator)
{
  double probabilityToBeMerged = 0;
  TrackCandidate* candidateToMergeBest = nullptr;

  for (auto iterator = start_iterator; iterator != end_iterator; iterator++) {
    TrackCandidate* cand2 = *iterator;
    if (trackCandidateToBeMerged == cand2) {
      continue;
    }


    if (cand2->getTrackHits().size() < 3) {
      continue;
    }

    double probabilityTemp = doTracksFitTogether(trackCandidateToBeMerged, cand2);

    // Reset hits, because we do not want to throw them away if this is not be best candidate to merge
    for (TrackHit* hit : trackCandidateToBeMerged->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }
    for (TrackHit* hit : cand2->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

    if (probabilityToBeMerged < probabilityTemp) {
      probabilityToBeMerged = probabilityTemp;
      candidateToMergeBest = cand2;
    }
  }

  return std::make_pair(candidateToMergeBest, probabilityToBeMerged);
}


void TrackMerger::tryToMergeTrackWithOtherTracks(TrackCandidate* cand1, std::list<TrackCandidate*>& trackList)
{

  TrackFitter trackFitter;
  B2DEBUG(100, "Merger: Initial nCands = " << trackList.size());

  bool have_merged_something;

  do {
    have_merged_something = false;
    BestMergePartner candidateToMergeBest = calculateBestTrackToMerge(cand1, trackList.begin(), trackList.end());
    Probability& probabilityWithCandidate = candidateToMergeBest.second;

    if (probabilityWithCandidate > m_minimum_probability_to_be_merged) {
      TrackCandidate* bestFitTrackCandidate = candidateToMergeBest.first;
      mergeTracks(cand1, bestFitTrackCandidate);
      have_merged_something = true;
    }

    B2DEBUG(100, "Cand hits vector size = " << cand1->getTrackHits().size());
    B2DEBUG(100, "Cand R = " << cand1->getR());

    trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
    [&](TrackCandidate * cand) { return (cand->getTrackHits().size() < 3); }),
    trackList.end());

  } while (have_merged_something);

  B2DEBUG(100, "Merger: Resulting nCands = " << trackList.size());

  for (TrackCandidate* cand : trackList) {
    for (TrackHit* hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

    trackFitter.fitTrackCandidateFast(cand);
    cand->reestimateCharge();
  }

}


void TrackMerger::removeStrangeHits(double factor, std::vector<TrackHit*>& trackHits, std::pair<double, double>& track_par,
                                    std::pair<double , double>& ref_point)
{

  // Maybe it is better to use the assignment probability here also? -> SimpleFilter
  for (TrackHit* hit : trackHits) {
    double x0_hit = hit->getWirePosition().X();
    double y0_hit = hit->getWirePosition().Y();
    double x0_track = cos(track_par.first) / fabs(track_par.second) + ref_point.first;
    double y0_track = sin(track_par.first) / fabs(track_par.second) + ref_point.second;
    double dist = fabs(fabs(1 / fabs(track_par.second) - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) *
                                                              (y0_track - y0_hit))) - hit->getDriftLength());
    if (dist > hit->getDriftLength() * factor) {
      hit->setHitUsage(TrackHit::c_bad);
    }
  }
  trackHits.erase(std::remove_if(trackHits.begin(), trackHits.end(),
  [&](TrackHit * hit) {
    return hit->getHitUsage() == TrackHit::c_bad;
  }), trackHits.end());
}

double TrackMerger::doTracksFitTogether(TrackCandidate* cand1, TrackCandidate* cand2)
{
  TrackFitter trackFitter;

  // Check if the two tracks do have something in common!
  Vector2D deltaMomentum = cand1->getMomentumEstimation() - cand2->getMomentumEstimation();
  Vector3D deltaPosition = cand1->getReferencePoint() - cand2->getReferencePoint();

  // Quick processing: if we have two tracks with approximately the same parameters, they can be merged!
  if (deltaMomentum.norm() / cand1->getMomentumEstimation().norm() < 0.1 and
      std::abs(TVector2::Phi_mpi_pi(cand1->getMomentumEstimation().phi() - cand2->getMomentumEstimation().phi())) < TMath::Pi() / 10 and
      deltaPosition.norm() < 10 and cand1->getChargeSign() == cand2->getChargeSign()) {
    return 1;
  }

  // Build common hit list
  std::vector<TrackHit*> commonHitListOfTwoTracks;
  for (TrackHit* hit : cand1->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }
  for (TrackHit* hit : cand2->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it does the job.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  commonHitListOfTwoTracks.erase(
    std::unique(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end()),
    commonHitListOfTwoTracks.end());

  for (TrackHit* hit : commonHitListOfTwoTracks) {
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  // Calculate track parameters
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999); // theta; R

  // Approach the best fit
  trackFitter.fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(5, commonHitListOfTwoTracks, track_par, ref_point);
  trackFitter.fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(3, commonHitListOfTwoTracks, track_par, ref_point);
  trackFitter.fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(1, commonHitListOfTwoTracks, track_par, ref_point);
  trackFitter.fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(1, commonHitListOfTwoTracks, track_par, ref_point);
  double chi2_temp = trackFitter.fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);

  int charge = TrackCandidate::getChargeAssumption(track_par.first, track_par.second, commonHitListOfTwoTracks);

  if (charge == TrackCandidate::charge_two_tracks) {
    return 0;
  }

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller than the two lists before or if the list is too small
  if (commonHitListOfTwoTracks.size() <= max(cand2->getTrackHits().size(), cand1->getTrackHits().size())
      or commonHitListOfTwoTracks.size() < 15) {
    return 0;
  }

  unsigned int ndf = commonHitListOfTwoTracks.size() - 4;
  return TMath::Prob(chi2_temp * ndf, ndf);
}

TrackCandidate* TrackMerger::splitBack2BackTrack(TrackCandidate* trackCandidate)
{
  assert(trackCandidate);

  // If the trackCandidate goes more or less through the IP, we have a problem with back-to-back tracks. These can be assigned to only on track.
  // If this is the case, we delete the smaller fraction here and let the track-finder find the remaining track again
  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackCandidate->getCharge() == TrackCandidate::charge_two_tracks) {

    // TODO
    std::sort(trackHits.begin(), trackHits.end(), [](TrackHit * hit1, TrackHit * hit2) {
      return hit1->getWirePosition().Mag2() < hit2->getWirePosition().Mag2();
    });

    unsigned int number_of_hits_in_one_half = 0;
    unsigned int number_of_hits_in_other_half = 0;

    Vector2D momentum = trackCandidate->getMomentumEstimation();
    double phiOfTrack = momentum.phi();

    for (TrackHit* hit : trackHits) {
      double phiOfHit = hit->getWirePosition().Phi();
      if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
        number_of_hits_in_one_half++;
      } else {
        number_of_hits_in_other_half++;
      }
    }

    if (number_of_hits_in_one_half > 0 and number_of_hits_in_other_half > 0) {

      TrackCandidate* secondTrackCandidate = new TrackCandidate(*trackCandidate);
      std::vector<TrackHit*>& secondTrackHits = secondTrackCandidate->getTrackHits();
      secondTrackHits.clear();

      // Small trick: mark the hits which should belong to the other track candidate as bad,
      // add them to the other track candidate and delete all marked hits from the first. Then unmark the hits again.

      for (TrackHit* hit : trackHits) {
        double phiOfHit = hit->getWirePosition().Phi();
        B2INFO(phiOfTrack << " " << phiOfHit)
        if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
          secondTrackHits.push_back(hit);
          hit->setHitUsage(TrackHit::c_bad);
        }
      }

      SimpleFilter::deleteAllMarkedHits(trackCandidate);

      for (TrackHit* hit : secondTrackHits) {
        hit->setHitUsage(TrackHit::c_usedInTrack);
      }

      return secondTrackCandidate;
    }
  }

  return nullptr;
}
