/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


double SimpleFilter::getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getWirePosition().X();
  double y0_hit = hit->getWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) *
                                   (y0_track - y0_hit))) - hit->getDriftLength());

  return 1.0 - exp(-1 / dist);
}



void SimpleFilter::reassignHitsFromOtherTracks(const std::list<TrackCandidate*>& m_trackList)
{

  B2DEBUG(100, "NCands = " << m_trackList.size());

  for (TrackCandidate* cand : m_trackList) {
    for (TrackHit* hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

    if (cand->getTrackHits().size() == 0) continue;

    for (TrackHit* hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);

      double bestHitProb = prob;
      TrackCandidate* bestCandidate = NULL;

      for (TrackCandidate* candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

        if (probTemp > bestHitProb) {
          bestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
        bestCandidate->addHit(hit);
        hit->setHitUsage(TrackHit::c_bad);
      }
    }

    deleteAllMarkedHits(cand);
  }

  for (TrackCandidate* cand : m_trackList) {
    for (TrackHit* hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }
  }
}

void SimpleFilter::deleteAllMarkedHits(TrackCandidate* trackCandidate)
{
  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  trackHits.erase(
  std::remove_if(trackHits.begin(), trackHits.end(), [](TrackHit * hit) {
    return hit->getHitUsage() == TrackHit::c_bad;
  }),
  trackHits.end());

}

void SimpleFilter::appendUnusedHits(const std::list<TrackCandidate*>& trackList, const std::vector<TrackHit*>& axialHitList,
                                    double minimal_assignment_probability)
{

  for (TrackHit* hit : axialHitList) {
    if (hit->getHitUsage() == TrackHit::c_usedInTrack or
        hit->getHitUsage() == TrackHit::c_bad) continue;

    // Search for best candidate to assign to
    double bestHitProb = 0;
    TrackCandidate* bestCandidate = nullptr;

    for (TrackCandidate* cand : trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      if (probTemp > bestHitProb) {
        bestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestCandidate != nullptr and bestHitProb > minimal_assignment_probability) {
      bestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

  }
}

void SimpleFilter::deleteWrongHitsOfTrack(TrackCandidate* trackCandidate, double minimal_assignment_probability)
{
  assert(trackCandidate);

  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackHits.size() == 0) return;

  for (TrackHit* hit : trackHits) {
    assert(hit);
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  int ndf = trackHits.size() - 4;

  if (ndf <= 0) return;

  for (TrackHit* trackHit : trackHits) {
    assert(trackHit);
    double assignment_probability = getAssigmentProbability(trackHit, trackCandidate);

    if (assignment_probability < minimal_assignment_probability) {
      trackHit->setHitUsage(TrackHit::c_bad);
    }
  }

  deleteAllMarkedHits(trackCandidate);
}
