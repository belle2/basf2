/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/legendre/TrackMerger.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include "../include/QuadTreeHitWrapper.h"

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCRecoHit3D HitProcessor::createRecoHit3D(CDCTrajectory2D& trackTrajectory2D, QuadTreeHitWrapper* hit)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  RightLeftInfo rlInfo = RIGHT;
  if (trackTrajectory2D.getDist2D(hit->getCDCWireHit()->getRefPos2D()) < 0)
    rlInfo = LEFT;
  const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit->getCDCWireHit()->getHit(), rlInfo);

  return CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

}

void HitProcessor::updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit)
{
  hit.setRecoPos3D(hit.getRecoHit2D().getRLWireHit().reconstruct3D(trackTrajectory2D));

  FloatType perpS = trackTrajectory2D.calcPerpS(hit.getRecoPos2D());
  if (perpS < 0.) {
    FloatType perimeter = fabs(trackTrajectory2D.getGlobalCircle().perimeter()) / 2.;
    perpS += perimeter;
  }
  // Recalculate the perpS of the hits
  hit.setPerpS(perpS);

//  B2INFO("    perpS: " << hit.getPerpS());

}


void HitProcessor::appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<QuadTreeHitWrapper*>& axialHitList)
{
  for (CDCTrack& trackCandidate : trackCandidates) {
    if (trackCandidate.size() < 5) continue;

    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//    SignType trackCharge = TrackMergerNew::getChargeSign(trackCandidate);
    CDCTrajectory2D trackTrajectory2D = trackCandidate.getStartTrajectory3D().getTrajectory2D();


    for (const QuadTreeHitWrapper* hit : axialHitList) {
      if (hit->getUsedFlag() || hit->getMaskedFlag()) continue;

      RightLeftInfo rlInfo = RIGHT;
      if (trackTrajectory2D.getDist2D(hit->getCDCWireHit()->getRefPos2D()) < 0)
        rlInfo = LEFT;
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit->getCDCWireHit()->getHit(), rlInfo);
      if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
        continue;

      //        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
      //          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

      if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.1) {
        trackCandidate.push_back(std::move(cdcRecoHit3D));
        cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
      }
    }

  }
  /*
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
  */
}

void HitProcessor::deleteAllMarkedHits(CDCTrack& trackCandidate)
{

  trackCandidate.erase(
  std::remove_if(trackCandidate.begin(), trackCandidate.end(), [](const CDCRecoHit3D & hit) {
    if (hit.getWireHit().getAutomatonCell().hasMaskedFlag()) {
      return true;
    }
    return false;
//    return hit.getWireHit().getAutomatonCell().hasMaskedFlag();
  }),
  trackCandidate.end());

}

void HitProcessor::deleteWrongHitsOfTrack(CDCTrack& trackCandidate)
{
  /*
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
  */
  deleteAllMarkedHits(trackCandidate);

}


/*
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



*/
