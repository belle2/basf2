/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/creators/TrackCreator.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void TrackProcessor::addCandidateWithHits(std::vector<const CDCWireHit*>& hits)
{
  if (hits.size() == 0) return;

  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  CDCTrack& track = m_cdcTrackList.createEmptyTrack();

  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  const CDCTrajectory2D& trackTrajectory2D = fitter.fit(observations);

  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = QuadTreeHitWrapperCreator::createRecoHit3D(trackTrajectory2D, item);
    track.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);

  postprocessTrack(track);
}

void TrackProcessor::postprocessTrack(CDCTrack& track)
{
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  //  B2INFO("update");
  trackQualityTools.normalizeTrack(track);
  //return;
  //  B2INFO("split");
  HitProcessor::splitBack2BackTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  //  B2INFO("update");
  trackQualityTools.normalizeTrack(track);

  removeBadSLayers(track);

  assignNewHitsUsingSegments(track);

  //  B2INFO("delete");
  deleteBadHitsOfOneTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  trackQualityTools.normalizeTrack(track);

  assignNewHits(track);
  assignNewHitsUsingSegments(track);
  removeBadSLayers(track);


  //  B2INFO("split");
  HitProcessor::splitBack2BackTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  //  B2INFO("update");
  trackQualityTools.normalizeTrack(track);

  //  B2INFO("delete");
  deleteBadHitsOfOneTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTrackList.getCDCTracks().remove(track);
    return;
  }



  //  B2INFO("update");
  trackQualityTools.normalizeTrack(track);

  assignNewHits(track);
  assignNewHitsUsingSegments(track);

  /*  ConformalExtension conformalExtension(this);

    conformalExtension.newRefPoint(track);

  */
  removeBadSLayers(track);
  //  B2INFO("update");
  trackQualityTools.normalizeTrack(track);

  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

}

void TrackProcessor::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }
}

void TrackProcessor::removeBadSLayers(CDCTrack& track)
{
  return;
  unmaskHitsInTrack(track);

  double apogeeArcLenght = fabs(track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 4.;

  for (CDCRecoHit3D& hit : track) {
    if (hit.getArcLength2D() < 0) {
      hit.setArcLength2D(hit.getArcLength2D() + apogeeArcLenght * 2.);
    }
  }

  track.sortByArcLength2D();

  int startSLayer = track.front().getISuperLayer();
  int currentSLayer(startSLayer);
  int nHitsInSL(0);

//  double startArcLength(track.front().getArcLength2D());
  double stopArcLength(track.back().getArcLength2D());

  std::vector<CDCRecoHit3D>::iterator lastHit;

  currentSLayer = track.front().getISuperLayer();
  nHitsInSL = 0;
  currentSLayer = 2;
  for (const CDCRecoHit3D& hit : track) {
    if (hit.getISuperLayer() < 2) continue;
    if (currentSLayer == hit.getISuperLayer()) nHitsInSL ++;
    else if (((currentSLayer == (hit.getISuperLayer() + 2)) || (currentSLayer == (hit.getISuperLayer() - 2))) && (nHitsInSL >= 4)) {
      nHitsInSL = 0;
      currentSLayer = hit.getISuperLayer();
    } else {
      stopArcLength = hit.getArcLength2D();
      break;
//      lastHit = hit;
    }
  }

  if ((stopArcLength > track.back().getArcLength2D())
      || (stopArcLength < track.front().getArcLength2D()))B2WARNING("wrong stop ArcLength");

  track.erase(std::remove_if(track.begin(), track.end(), [&stopArcLength](CDCRecoHit3D & hit) {
    if (hit.getArcLength2D() > stopArcLength) {
      hit.getWireHit()->getAutomatonCell().setTakenFlag(false);
      hit.getWireHit()->getAutomatonCell().setMaskedFlag(false);
      return true;
    } else return false;

//    return hit.getArcLength2D() > stopArcLength;
  }), track.end());

}

bool TrackProcessor::checkTrack(CDCTrack& track)
{
  if (track.size() < 5) return false;

  return true;
}

void TrackProcessor::deleteBadHitsOfOneTrack(CDCTrack& trackCandidate)
{
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  for (CDCRecoHit3D& recoHit : trackCandidate) {

    if (fabs(trackCandidate.getStartTrajectory3D().getTrajectory2D().getDist2D(recoHit.getRecoPos2D())) > 0.2)
      recoHit->getWireHit().getAutomatonCell().setMaskedFlag(true);
  }

  HitProcessor::deleteAllMarkedHits(trackCandidate);

  trackQualityTools.normalizeTrack(trackCandidate);
}

void TrackProcessor::assignNewHits()
{
  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  m_cdcTrackList.getCDCTracks().erase(std::remove_if(m_cdcTrackList.getCDCTracks().begin(), m_cdcTrackList.getCDCTracks().end(),
  [](const CDCTrack & track) {
    return track.size() == 0;
  }) , m_cdcTrackList.getCDCTracks().end());


//  for (CDCTrack& track : m_cdcTracks) {
  m_cdcTrackList.doForAllTracks([&](CDCTrack & track) {

    if (track.size() < 4) return;

    assignNewHits(track);

    std::vector<const CDCWireHit*> removedHits = HitProcessor::splitBack2BackTrack(track);

    addCandidateWithHits(removedHits);

    //  B2INFO("update");
    trackQualityTools.normalizeTrack(track);

    //  B2INFO("delete");
    deleteBadHitsOfOneTrack(track);

    trackQualityTools.normalizeTrack(track);
  });

  HitProcessor::reassignHitsFromOtherTracks(m_cdcTrackList.getCDCTracks());
  m_cdcTrackList.doForAllTracks([](CDCTrack & cand) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  });

}

void TrackProcessor::assignNewHits(CDCTrack& track)
{
  if (track.size() < 10) return;

  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  unmaskHitsInTrack(track);

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//  ESign trackCharge = TrackMergerNew::getChargeSign(track);
  CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();


  for (ConformalCDCWireHit& hit : m_hitFactory.getQuadTreeHitWrappers()) {
    if (hit.getUsedFlag() || hit.getMaskedFlag()) continue;

    ERightLeft rlInfo = ERightLeft::c_Right;
    if (trackTrajectory2D.getDist2D(hit.getCDCWireHit()->getRefPos2D()) < 0)
      rlInfo = ERightLeft::c_Left;
    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit.getCDCWireHit()->getHit(), rlInfo);
    if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
      continue;

//        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
//          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

    if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.15) {
      track.push_back(std::move(cdcRecoHit3D));
      cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
    }


  }

  trackQualityTools.normalizeTrack(track);
}

void TrackProcessor::assignNewHitsUsingSegments(CDCTrack& track, float fraction)
{
  return;
  if (track.size() < 5) return;

  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//  ESign trackCharge = TrackMergerNew::getChargeSign(track);
  CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  std::vector<CDCRecoSegment2D> segments;
  for (CDCRecoHit3D& recoHit : track) {
    for (ConformalCDCWireHit& qtHit : m_hitFactory.getQuadTreeHitWrappers()) {
      if (qtHit.getCDCWireHit()->getWire() == recoHit.getWire()) {

        CDCRecoSegment2D& segment = qtHit.getSegment();
        if (segment.size() == 0) continue;
        bool addSegment(true);
        if (segments.size() != 0) {
          for (CDCRecoSegment2D& segmentTmp : segments) {
            if (segmentTmp == segment) {
              addSegment = false;
            }
          }
        }

        int nHits(0);
        for (CDCRecoHit2D& segmentHit : segment) {
//          for (CDCRecoHit3D& recoHitTmp : track) {
          if (recoHit.getWire() == segmentHit.getWire()) nHits++;
//          }
        }

        if (static_cast<float>(nHits / segment.items().size()) < fraction) addSegment = false;

        if (addSegment)segments.push_back(segment);

        break;
      }
    }
  }

  std::vector<ConformalCDCWireHit*> hitsToAdd;

  for (CDCRecoSegment2D& segment : segments) {
    for (ConformalCDCWireHit& qtHitGlobal : m_hitFactory.getQuadTreeHitWrappers()) {
      bool addHit(true);
      for (CDCRecoHit3D& recoHit : track) {
        if (qtHitGlobal.getCDCWireHit()->getWire() == recoHit.getWire()) addHit = false;
      }

      if (addHit && (qtHitGlobal.getSegment() == segment)) {
        hitsToAdd.push_back(&qtHitGlobal);
      }

    }

  }

  for (ConformalCDCWireHit* hit : hitsToAdd) {

    ERightLeft rlInfo = ERightLeft::c_Right;
    if (trackTrajectory2D.getDist2D(hit->getCDCWireHit()->getRefPos2D()) < 0)
      rlInfo = ERightLeft::c_Left;
    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit->getCDCWireHit()->getHit(), rlInfo);
    if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
      continue;

//        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
//          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

    if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.2) {
      track.push_back(std::move(cdcRecoHit3D));
      cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
    }


  }


  trackQualityTools.normalizeTrack(track);


}

void TrackProcessor::checkTrackProb()
{
  m_cdcTrackList.doForAllTracks([](CDCTrack & track) {
    std::vector<std::pair<std::pair<double, double>, double>> hits;

    for (const CDCRecoHit3D& hit : track) {

      hits.push_back(std::make_pair(std::make_pair(hit.getRecoWirePos2D().x(), hit.getRecoWirePos2D().y()),
                                    fabs(hit.getSignedRecoDriftLength())));
    }


//    B2DEBUG(100, "chi2: new = " << TMath::Prob(HitProcessor::fitWhithoutRecoPos(track), track.size() - 4) << "; old = " << TMath::Prob(TrackFitter::fitTrackCandidateFast(hits), track.size() - 4));

    if (TMath::Prob((HitProcessor::fitWhithoutRecoPos(track)).getChi2(), track.size() - 4) < 0.4) {
      for (const CDCRecoHit3D& hit : track) {
        hit->getWireHit().getAutomatonCell().setMaskedFlag(true);
      }
      HitProcessor::deleteAllMarkedHits(track);
    }
  });

  return ;

}


bool TrackProcessor::checkChi2(CDCTrack& track)
{
  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  double minChi2 = calculateChi2ForQuantile(0.025, trackTrajectory2D.getNDF());
  double maxChi2 = calculateChi2ForQuantile(0.975, trackTrajectory2D.getNDF());
  double Chi2 = trackTrajectory2D.getChi2();

  if ((Chi2 < minChi2) || (Chi2 > maxChi2)) return false;

  return true;
}

double TrackProcessor::calculateChi2ForQuantile(double alpha, double n)
{
  double d;
  if (alpha > 0.5) {
    d = 2.0637 * pow(log(1. / (1. - alpha)) - 0.16, 0.4274) - 1.5774;
  } else {
    d = -2.0637 * pow(log(1. / alpha) - 0.16, 0.4274) + 1.5774;
  }

  double A = d * pow(2., 0.5);
  double B = 2.*(d * d - 1.) / 3.;
  double C = d * (d * d - 7) / (9. * pow(2., 0.5));
  double D = (6.*d * d * d * d + 14 * d * d - 32) / 405.;
  double E = d * (9 * d * d * d * d + 256 * d * d - 433) / (4860. * pow(2., 0.5));

  return n + A * pow(n, 0.5) + B + C / pow(n, 0.5) + D / n + E / (n * pow(n, 0.5));
}

