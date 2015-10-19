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
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/legendre/ConformalExtension.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>
#include "../include/TrackHitsProcessor.h"

#include "TCanvas.h"
#include "TH1F.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

/*CDCTrack&*/void TrackProcessorNew::createCDCTrackCandidates(std::vector<QuadTreeHitWrapper*>& trackHits)
{
//  B2INFO("create");
  if (trackHits.size() == 0) return;

  m_cdcTracks.emplace_back();
  CDCTrack& newTrackCandidate = m_cdcTracks.back();


  CDCObservations2D observations2DLegendre;
  for (const QuadTreeHitWrapper* item : trackHits) {
    observations2DLegendre.append(*item->getCDCWireHit());
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations2DLegendre);


  for (QuadTreeHitWrapper* trackHit : trackHits) {
    if (trackHit->getUsedFlag() || trackHit->getMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitProcessor::createRecoHit3D(trackTrajectory2D, trackHit);
    newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  newTrackCandidate.setStartTrajectory3D(trajectory3D);

  postprocessTrack(newTrackCandidate);

}



void TrackProcessorNew::createCandidate(std::vector<const CDCWireHit*>& hits)
{
  if (hits.size() == 0) return;
  //  B2INFO("create");
  m_cdcTracks.emplace_front();
  CDCTrack& newTrackCandidate = m_cdcTracks.front();


  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations);


  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitProcessor::createRecoHit3D(trackTrajectory2D, item);
    newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  newTrackCandidate.setStartTrajectory3D(trajectory3D);

  postprocessTrack(newTrackCandidate);

}

void TrackProcessorNew::postprocessTrack(CDCTrack& track)
{
  //  B2INFO("update");
  updateTrack(track);
  //return;
  //  B2INFO("split");
  TrackHitsProcessor::splitBack2BackTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTracks.remove(track);
    return;
  }

  //  B2INFO("update");
  updateTrack(track);

  removeBadSLayers(track);

  assignNewHitsUsingSegments(track);

  //  B2INFO("delete");
  deleteBadHitsOfOneTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTracks.remove(track);
    return;
  }

  updateTrack(track);

  assignNewHits(track);
  assignNewHitsUsingSegments(track);
  removeBadSLayers(track);


  //  B2INFO("split");
  TrackHitsProcessor::splitBack2BackTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTracks.remove(track);
    return;
  }

  //  B2INFO("update");
  updateTrack(track);

  //  B2INFO("delete");
  deleteBadHitsOfOneTrack(track);
  if (not checkTrack(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    m_cdcTracks.remove(track);
    return;
  }



  //  B2INFO("update");
  updateTrack(track);

  assignNewHits(track);
  assignNewHitsUsingSegments(track);

  /*  ConformalExtension conformalExtension(this);

    conformalExtension.newRefPoint(track);

  */
  removeBadSLayers(track);
  //  B2INFO("update");
  updateTrack(track);

  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

}

void TrackProcessorNew::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
}


void TrackProcessorNew::updateTrack(CDCTrack& track)
{
  // Set the start point of the trajectory to the first hit
//  track.sort();
  if (track.size() < 5) return;

  unmaskHitsInTrack(track);


  CDCTrajectory2D trackTrajectory2D = /*m_trackFitter.*/fit(track);

  //B2INFO("orientation: " << trackTrajectory2D.getLocalCircle().orientation() << "; charge: " << trackTrajectory2D.getChargeSign() << "; charge2: " << TrackMergerNew::getChargeSign(track));

  if (trackTrajectory2D.getChargeSign() != TrackHitsProcessor::getChargeSign(track)) trackTrajectory2D.reverse();

  trackTrajectory2D.setLocalOrigin(trackTrajectory2D.getGlobalPerigee());

//  B2INFO("updated reco position:");
  for (CDCRecoHit3D& recoHit : track) {
    HitProcessor::updateRecoHit3D(trackTrajectory2D, recoHit);
  }

  track.sortByArcLength2D();

//  trackTrajectory2D.setLocalOrigin(track.front().getRecoPos2D());

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);

  for (CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }

}

void TrackProcessorNew::removeBadSLayers(CDCTrack& track)
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

bool TrackProcessorNew::checkTrack(CDCTrack& track)
{
  if (track.size() < 5) return false;

  return true;
}



void TrackProcessorNew::createCDCTracks(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
//  tracks.clear();

//  std::copy(m_tracksVector.begin(), m_tracksVector.end(), tracks.begin());

  const TrackQualityTools& trackQualityTools = TrackQualityTools::getInstance();
  tracks.reserve(tracks.size() + m_cdcTracks.size());

  for (CDCTrack& track : m_cdcTracks) {
    trackQualityTools.normalizeHitsAndResetTrajectory(track);
    track.sortByRadius();
    if (track.size() > 5) tracks.push_back(std::move(track));
  }
}

void TrackProcessorNew::initializeQuadTreeHitWrappers()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  const std::vector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }

  m_QuadTreeHitWrappers.reserve(cdcWireHits.size());
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    if (cdcWireHit.getAutomatonCell().hasTakenFlag()) continue;
    QuadTreeHitWrapper QuadTreeHitWrapper(cdcWireHit);
    if (QuadTreeHitWrapper.checkHitDriftLength() and QuadTreeHitWrapper.getCDCWireHit()->isAxial()) {
      m_QuadTreeHitWrappers.push_back(std::move(QuadTreeHitWrapper));
    }
  }
  B2DEBUG(90, "Number of hits to be used by legendre track finder: " << m_QuadTreeHitWrappers.size() << " axial.");

}


std::vector<QuadTreeHitWrapper*> TrackProcessorNew::createQuadTreeHitWrappersForQT(bool useSegmentsOnly)
{

  doForAllTracks([](CDCTrack & cand) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  });

  std::vector<QuadTreeHitWrapper*> QuadTreeHitWrappers;
  doForAllHits([&QuadTreeHitWrappers, &useSegmentsOnly](QuadTreeHitWrapper & trackHit) {
    if (trackHit.getUsedFlag() || trackHit.getMaskedFlag()) return;
    if ((not trackHit.getSegment().isAxial()) and useSegmentsOnly) return;
    QuadTreeHitWrappers.push_back(&trackHit);
  });
  B2DEBUG(90, "In hit set are " << QuadTreeHitWrappers.size() << " hits.")
  return QuadTreeHitWrappers;
}

void TrackProcessorNew::deleteBadHitsOfOneTrack(CDCTrack& trackCandidate)
{

  for (CDCRecoHit3D& recoHit : trackCandidate) {

    if (fabs(trackCandidate.getStartTrajectory3D().getTrajectory2D().getDist2D(recoHit.getRecoPos2D())) > 0.3)
      recoHit->getWireHit().getAutomatonCell().setMaskedFlag(true);
  }

  TrackHitsProcessor::deleteAllMarkedHits(trackCandidate);

  updateTrack(trackCandidate);
}

void TrackProcessorNew::assignNewHits()
{
  m_cdcTracks.erase(std::remove_if(m_cdcTracks.begin(), m_cdcTracks.end(),
  [](const CDCTrack & track) {
    return track.size() == 0;
  }) , m_cdcTracks.end());


//  for (CDCTrack& track : m_cdcTracks) {
  doForAllTracks([&](CDCTrack & track) {

    if (track.size() < 4) return;

    assignNewHits(track);

    std::vector<const CDCWireHit*> removedHits = TrackHitsProcessor::splitBack2BackTrack(track);

    createCandidate(removedHits);

    //  B2INFO("update");
    updateTrack(track);

    //  B2INFO("delete");
    deleteBadHitsOfOneTrack(track);

    updateTrack(track);
  });

  HitProcessor::reassignHitsFromOtherTracks(m_cdcTracks);
  doForAllTracks([](CDCTrack & cand) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  });

}

void TrackProcessorNew::assignNewHits(CDCTrack& track)
{
  if (track.size() < 10) return;

  unmaskHitsInTrack(track);

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//  ESign trackCharge = TrackMergerNew::getChargeSign(track);
  CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();


  for (QuadTreeHitWrapper& hit : m_QuadTreeHitWrappers) {
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

  updateTrack(track);
}


void TrackProcessorNew::assignNewHitsUsingSegments(CDCTrack& track, float fraction)
{
  return;
  if (track.size() < 5) return;

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//  ESign trackCharge = TrackMergerNew::getChargeSign(track);
  CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  std::vector<CDCRecoSegment2D> segments;
  for (CDCRecoHit3D& recoHit : track) {
    for (QuadTreeHitWrapper& qtHit : m_QuadTreeHitWrappers) {
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

  std::vector<QuadTreeHitWrapper*> hitsToAdd;

  for (CDCRecoSegment2D& segment : segments) {
    for (QuadTreeHitWrapper& qtHitGlobal : m_QuadTreeHitWrappers) {
      bool addHit(true);
      for (CDCRecoHit3D& recoHit : track) {
        if (qtHitGlobal.getCDCWireHit()->getWire() == recoHit.getWire()) addHit = false;
      }

      if (addHit && (qtHitGlobal.getSegment() == segment)) {
        hitsToAdd.push_back(&qtHitGlobal);
      }

    }

  }

  for (QuadTreeHitWrapper* hit : hitsToAdd) {

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


  updateTrack(track);


}


void TrackProcessorNew::mergeTracks()
{
  doForAllTracks([](CDCTrack & track) {
    std::vector<std::pair<std::pair<double, double>, double>> hits;

    for (const CDCRecoHit3D& hit : track) {

      hits.push_back(std::make_pair(std::make_pair(hit.getRecoWirePos2D().x(), hit.getRecoWirePos2D().y()),
                                    fabs(hit.getSignedRecoDriftLength())));
    }


//    B2DEBUG(100, "chi2: new = " << TMath::Prob(TrackHitsProcessor::fitWhithoutRecoPos(track), track.size() - 4) << "; old = " << TMath::Prob(TrackFitter::fitTrackCandidateFast(hits), track.size() - 4));

    if (TMath::Prob((TrackHitsProcessor::fitWhithoutRecoPos(track)).getChi2(), track.size() - 4) < 0.4) {
      for (const CDCRecoHit3D& hit : track) {
        hit->getWireHit().getAutomatonCell().setMaskedFlag(true);
      }
      TrackHitsProcessor::deleteAllMarkedHits(track);
    }
  });

  return ;

  m_cdcTracks.sort([](const CDCTrack & track1, const CDCTrack & track2) {
    return track1.size() < track2.size();
  });


  for (std::list<CDCTrack>::iterator it1 = m_cdcTracks.begin(); it1 !=  m_cdcTracks.end(); ++it1) {
    std::list<CDCTrack>::iterator it2 = it1;
    ++it2;

    for (; it2 !=  m_cdcTracks.end(); ++it2) {

      CDCTrack& track1 = *it1;
      CDCTrack& track2 = *it2;

      unmaskHitsInTrack(track1);
      unmaskHitsInTrack(track2);

//  for (CDCTrack& track1 : m_cdcTracks) {
//    unsigned int innerCounter = 0;
//    if (track1.size() == 0) continue;
//    removeBadSLayers(track1);
//    CDCTrack& bestCandidate = nullptr;
//    for (CDCTrack& track2 : m_cdcTracks) {
//      if (innerCounter <= outerCounter) {
//        innerCounter++;
//        continue;
//      }
//      if (track2.size() == 0) continue;
//      removeBadSLayers(track2);

      if (TrackHitsProcessor::mergeTwoTracks(track1, track2)) {
//        B2INFO("MERGE TRACKS!");

        for (const CDCRecoHit3D& hit : track2) {
          const CDCRecoHit3D& cdcRecoHit3D  =  CDCRecoHit3D::reconstruct(hit.getRLWireHit(), track1.getStartTrajectory3D().getTrajectory2D());

          track1.push_back(std::move(cdcRecoHit3D));
        }
        track2.clear();

        updateTrack(track1);

        std::vector<const CDCWireHit*> removedHits = TrackHitsProcessor::splitBack2BackTrack(track1);

        updateTrack(track1);

        createCandidate(removedHits);
      }

    }
  }

  m_cdcTracks.erase(std::remove_if(m_cdcTracks.begin(), m_cdcTracks.end(),
  [](const CDCTrack & track) {
    return track.size() == 0;
  }) , m_cdcTracks.end());

}

CDCTrajectory2D TrackProcessorNew::fit(CDCTrack& track)
{
  bool m_usePosition(true);
//  bool m_useOrientation(false);

  CDCTrajectory2D result;
  CDCObservations2D observations2D;
  observations2D.setUseRecoPos(m_usePosition);
  size_t nAppendedHits = 0;
  for (const CDCRecoHit3D& item : track.items()) {
    nAppendedHits += observations2D.append(item, m_usePosition);
  }

  m_trackFitter.update(result, observations2D);

  return result;
}



void TrackProcessorNew::fillHist(CDCTrack& track)
{
  m_histChi2.Fill(track.getStartTrajectory3D().getChi2());
  m_histChi2NDF.Fill(track.getStartTrajectory3D().getChi2() / track.getStartTrajectory3D().getNDF());

  for (const CDCRecoHit3D& hit : track) {

    m_histDist.Fill(fabs(track.getStartTrajectory3D().getTrajectory2D().getDist2D(hit.getRecoPos2D())));
  }
}

void TrackProcessorNew::saveHist()
{
  TCanvas canv1("canv1", "canv1", 600, 800);
  m_histChi2.Draw();
  canv1.SaveAs("tmp/chi2.root");

  TCanvas canv2("canv2", "canv2", 600, 800);
  m_histDist.Draw();
  canv2.SaveAs("tmp/dist.root");

  TCanvas canv3("canv3", "canv3", 600, 800);
  m_histChi2NDF.Draw();
  canv3.SaveAs("tmp/chi2NDF.root");

  TCanvas canv4("canv4", "canv4", 600, 800);
  m_histADC.Draw();
  canv4.SaveAs("tmp/meanADC.root");

  TCanvas canv5("canv5", "canv5", 600, 800);
  m_histADCpt.Draw();
  canv5.SaveAs("tmp/meanADCpt.root");

}



bool TrackProcessorNew::checkChi2(CDCTrack& track)
{
  CDCTrajectory2D trackTrajectory2D = /*m_trackFitter.*/fit(track);


  double minChi2 = getQuantile(0.025, trackTrajectory2D.getNDF());
  double maxChi2 = getQuantile(0.975, trackTrajectory2D.getNDF());
  double Chi2 = trackTrajectory2D.getChi2();

//  B2INFO("Chi2: " << Chi2 << "; min: " << minChi2 << "; max: " << maxChi2 << "; PVal: " << trackTrajectory2D.getPValue());

  if ((Chi2 < minChi2) || (Chi2 > maxChi2)) return false;

  return true;

}


double TrackProcessorNew::getQuantile(double alpha, double n)
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


void TrackProcessorNew::checkADC()
{
  for (CDCTrack& track : m_cdcTracks) {

    double meanADC(0.);

    for (CDCRecoHit3D& hit : track) {
      meanADC += hit.getWireHit().getHit()->getADCCount();
    }

    meanADC = meanADC / track.size();

    m_histADC.Fill(meanADC);

    m_histADCpt.Fill(meanADC, track.getStartTrajectory3D().getTrajectory2D().getAbsMom2D());


  }
}
