/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>


using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


double SimpleFilter::getAssigmentProbability(TrackHit* hit, TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());

  return 1.0 - exp(-1 / dist);
}



void SimpleFilter::processTracks(std::list<TrackCandidate*>& m_trackList)
{

  /*
  double total_prob = 1.;
  for (CDCLegendreFilterCandidate* filterCandidate : m_cdcLegendreFilterCandidateList) {

    for(auto& hitMap: filterCandidate->getHitsMap()){
      CDCLegendreTrackHit* hit = hitMap.first;
      double prob = hitMap.second;

  //     if(prob<m_minProb){

        double bestHitProb = prob;
        TrackCandidate* BestCandidate;

        for (TrackCandidate * cand : m_trackList) {
          double probTemp = getAssigmentProbability(hit, cand);

          int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

          if(probTemp > bestHitProb && curve_sign == cand->getCharge()) {
            BestCandidate = cand;
            bestHitProb = probTemp;
          }
        }

        if(bestHitProb > prob) {
          filterCandidate->getLegendreCandidate()->removeHit(hit);
          BestCandidate->addHit(hit);
  //         filterCandidate->removeHit(hit);
          B2INFO("Hit has been reassigned.");
        }


  //     }

    }

  }

  */

  int ii = 0;

  B2DEBUG(100, "NCands = " << m_trackList.size());

  for (TrackCandidate * cand : m_trackList) {
    ii++;
    B2DEBUG(100, "ii = " << ii);
    B2DEBUG(100, "Processing: Cand hits vector size = " << cand->getTrackHits().size());
    B2DEBUG(100, "Processing: Cand R = " << cand->getR());

    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    if (cand->getTrackHits().size() == 0) continue;

    for (TrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);

      double bestHitProb = prob;
      TrackCandidate* BestCandidate = NULL;

      for (TrackCandidate * candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

        if (probTemp > bestHitProb) {
          BestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
        BestCandidate->addHit(hit);
        hit->setHitUsage(TrackHit::bad);
      }
    }

    deleteAllMarkedHits(cand);
  }

  for (TrackCandidate * cand : m_trackList) {
    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }
}

void SimpleFilter::deleteAllMarkedHits(TrackCandidate* trackCandidate)
{
  trackCandidate->getTrackHits().erase(
    std::remove_if(trackCandidate->getTrackHits().begin(), trackCandidate->getTrackHits().end(),
  [&](TrackHit * hit) {return hit->getHitUsage() == TrackHit::bad;}),
  trackCandidate->getTrackHits().end());

}

void SimpleFilter::appendUnusedHits(std::list<TrackCandidate*>& trackList, std::vector<TrackHit*>& axialHitList, double minimal_assignment_probability)
{

  for (TrackHit * hit : axialHitList) {
    if (hit->getHitUsage() == TrackHit::used_in_track or hit->getHitUsage() == TrackHit::used_in_cand) continue;

    // Search for best candidate to assign to
    double bestHitProb = 0;
    TrackCandidate* BestCandidate = nullptr;

    for (TrackCandidate * cand : trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      if (probTemp > bestHitProb) {
        BestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestHitProb > minimal_assignment_probability) {
      BestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);
    }

  }
}

void SimpleFilter::deleteWrongHitsOfTrack(TrackCandidate* trackCandidate, double minimal_assignment_probability, TrackFitter* trackFitter)
{

  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackHits.size() == 0) return;

  for (TrackHit * hit : trackHits) {
    hit->setHitUsage(TrackHit::used_in_track);
  }

  int ndf = trackHits.size() - 4;

  if (ndf <= 0) return;

  // If the trackCandidate goes more or less through the IP, we have a problem with back-to-back tracks. These can be assigned to only on track.
  // If this is the case, we delete the smaller fraction here and let the track-finder find the remaining track again

  if (trackCandidate->getCharge() == TrackCandidate::charge_two_tracks and trackCandidate->getReferencePoint().Mag() < 0.5) {

    unsigned int number_of_hits_in_one_half = 0;
    unsigned int number_of_hits_in_other_half = 0;

    double phiOfTrack = trackCandidate->getMomentumEstimation().Phi();

    for (TrackHit * hit : trackHits) {
      double phiOfHit = hit->getWirePosition().Phi();
      if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
        number_of_hits_in_one_half++;
      } else {
        number_of_hits_in_other_half++;
      }
    }

    if (number_of_hits_in_one_half > 2 * number_of_hits_in_other_half) {
      for (TrackHit * hit : trackHits) {
        double phiOfHit = hit->getWirePosition().Phi();
        if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) >= TMath::PiOver2()) {
          hit->setHitUsage(TrackHit::bad);
        }
      }
    } else if (number_of_hits_in_other_half > 2 * number_of_hits_in_one_half) {
      for (TrackHit * hit : trackHits) {
        double phiOfHit = hit->getWirePosition().Phi();
        if (std::abs(TVector2::Phi_mpi_pi(phiOfTrack - phiOfHit)) < TMath::PiOver2()) {
          hit->setHitUsage(TrackHit::bad);
        }
      }
    }
  }

  deleteAllMarkedHits(trackCandidate);

  for (auto hitIterator = trackHits.begin(); hitIterator != trackHits.end(); hitIterator++) {
    double assignment_probability = getAssigmentProbability(*hitIterator, trackCandidate);

    if (assignment_probability < minimal_assignment_probability) {
      (*hitIterator)->setHitUsage(TrackHit::bad);
    }
  }

  deleteAllMarkedHits(trackCandidate);
}

// UNUSED AT THE MOMENT

/*
void SimpleFilter::trackCore()
{

  for (TrackCandidate * cand : m_trackList) {

//    if(cand->getTrackHits().size() < 10)

    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      bool removeHit = false;
      double prob = getAssigmentProbability(hit, cand);;

      if (prob < 0.9) {
        removeHit = true;
      } else {

        double otherProbs = 0;

        for (TrackCandidate * candInner : m_trackList) {
          if (candInner == cand) continue;
          double probTemp = getAssigmentProbability(hit, candInner);

          if (probTemp > otherProbs && curve_sign == candInner->getCharge()) {

            otherProbs = probTemp;
          }
        }

        if (otherProbs > prob || otherProbs > 0.6) {
          removeHit = true;
        }
      }

      if (removeHit) {
        cand->removeHit(hit);
//        B2INFO("Hit has been removed!");
      }


    }

  }



}

*/


