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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/legendre/TrackMerger.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>

#include "TCanvas.h"
#include "TH1F.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCTrack& TrackProcessorNew::createLegendreTrackCandidateFromHits(std::vector<LegendreHit*>& trackHits)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  m_cdcTracks.emplace_back();
  CDCTrack& newTrackCandidate = m_cdcTracks.back();


  CDCObservations2D observations2DLegendre;
  for (const LegendreHit* item : trackHits) {
    observations2DLegendre.append(*item->getCDCWireHit());
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations2DLegendre);


  for (LegendreHit* trackHit : trackHits) {
    RightLeftInfo rlInfo = RIGHT;
    if (trackTrajectory2D.getDist2D(trackHit->getCDCWireHit()->getRefPos2D()) < 0)
      rlInfo = LEFT;
    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(trackHit->getCDCWireHit()->getHit(), rlInfo);
    if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
      continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);
    newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
  /*
    static int count(0);

    TCanvas canv1(Form("canv1_%i", count), "canv", 600, 800);
    TH1F hist1(Form("hist1_%i", count), "hist", 100, 0, 0.5);

    for (CDCRecoHit3D& recoHit : newTrackCandidate) {
      hist1.Fill(trackTrajectory2D.getDist2D(recoHit.getRecoPos2D()));
    }


    hist1.Draw();
    canv1.SaveAs(Form("tmp/distToHitBefore_%i.root", count));
  */

  updateTrack(newTrackCandidate);
  /*
    TCanvas canv2(Form("canv2_%i", count), "canv", 600, 800);
    TH1F hist2(Form("hist2_%i", count), "hist", 100, 0, 0.5);

    for (CDCRecoHit3D& recoHit : newTrackCandidate) {
      hist2.Fill(newTrackCandidate.getStartTrajectory3D().getTrajectory2D().getDist2D(recoHit.getRecoPos2D()));
    }


    hist2.Draw();
    canv2.SaveAs(Form("tmp/distToHitAfter_%i.root", count));

    count++;
  */
  TrackMergerNew::splitBack2BackTrack(newTrackCandidate);

  updateTrack(newTrackCandidate);

  deleteBadHitsOfOneTrack(newTrackCandidate);

  assignNewHits(newTrackCandidate);

  TrackMergerNew::splitBack2BackTrack(newTrackCandidate);

  updateTrack(newTrackCandidate);

  deleteBadHitsOfOneTrack(newTrackCandidate);


  for (LegendreHit* hit : trackHits) {
    hit->setUsedFlag(true);
  }

  return newTrackCandidate;
}

void TrackProcessorNew::updateTrack(CDCTrack& track)
{
  // Set the start point of the trajectory to the first hit
  track.sort();

  CDCTrajectory2D trackTrajectory2D = m_trackFitter.fit(track);

  for (CDCRecoHit3D& recoHit : track) {
    recoHit.setRecoPos3D(recoHit.getRecoHit2D().getRLWireHit().reconstruct3D(trackTrajectory2D));
  }

  // Recalculate the perpS of the hits
  for (CDCRecoHit3D& recoHit : track) {
    recoHit.setPerpS(trackTrajectory2D.calcPerpS(recoHit.getRecoPos2D()));
  }

  track.sortByPerpS();

  trackTrajectory2D.setLocalOrigin(track.front().getRecoPos2D());

  // Maybe we should reverse the trajectory here, is this right?

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);


  /*
  static int count(0);

    TCanvas canv(Form("canv_%i", count), "canv", 600, 800);
    TH1F hist(Form("hist_%i", count), "hist", 100, 0, 0.5);

    for (CDCRecoHit3D& recoHit : newTrackCandidate) {
      hist.Fill(recoHit.getSquaredDist2D(trackTrajectory2D));
    }


    hist.Draw();
    canv.SaveAs(Form("tmp/distToHit_%i.root", count));
    count++;
  */


}



void TrackProcessorNew::createCDCTrackCandidates(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{

  tracks.clear();


//  std::copy(m_tracksVector.begin(), m_tracksVector.end(), tracks.begin());

  for (CDCTrack& track : m_cdcTracks) {
    if (track.size() > 5) tracks.push_back(std::move(track));
  }


}

void TrackProcessorNew::initializeLegendreHits()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  const std::vector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }

  m_legendreHits.reserve(cdcWireHits.size());
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    if (cdcWireHit.getAutomatonCell().hasTakenFlag()) continue;
    LegendreHit legendreHit(cdcWireHit);
    if (legendreHit.checkHitDriftLength() and legendreHit.getCDCWireHit()->isAxial()) {
      m_legendreHits.push_back(std::move(legendreHit));
    }
  }
  B2DEBUG(90, "Number of hits to be used by legendre track finder: " << m_legendreHits.size() << " axial.");

}

std::vector<LegendreHit*> TrackProcessorNew::createLegendreHitsForQT()
{
  doForAllTracks([](CDCTrack & cand) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  });

  std::vector<LegendreHit*> legendreHits;
  doForAllHits([&legendreHits](LegendreHit & trackHit) {
    if (trackHit.getUsedFlag() || trackHit.getMaskedFlag()) return;
    legendreHits.push_back(&trackHit);
  });
  B2DEBUG(90, "In hit set are " << legendreHits.size() << " hits.")
  return legendreHits;
}

void TrackProcessorNew::deleteBadHitsOfOneTrack(CDCTrack& trackCandidate)
{

  for (CDCRecoHit3D& recoHit : trackCandidate) {

    if (trackCandidate.getStartTrajectory3D().getTrajectory2D().getDist2D(recoHit.getRecoPos2D()) > 0.1)
      recoHit->getWireHit().getAutomatonCell().setMaskedFlag(true);
  }

  TrackMergerNew::deleteAllMarkedHits(trackCandidate);

  updateTrack(trackCandidate);
}

void TrackProcessorNew::assignNewHits(CDCTrack& track)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  SignType trackCharge = TrackMergerNew::getChargeSign(track);
  CDCTrajectory2D trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();


  for (LegendreHit& hit : m_legendreHits) {
    if (hit.getUsedFlag() || hit.getMaskedFlag()) continue;

    RightLeftInfo rlInfo = RIGHT;
    if (trackTrajectory2D.getDist2D(hit.getCDCWireHit()->getRefPos2D()) < 0)
      rlInfo = LEFT;
    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit.getCDCWireHit()->getHit(), rlInfo);
    if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
      continue;

//        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
//          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

    if (trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D()) < 0.2) {
      track.push_back(std::move(cdcRecoHit3D));
      cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
    }


  }

  updateTrack(track);
}















int TrackProcessorNew::estimateCharge(double theta, double r, std::vector<TrackHit*>& trackHits)
{
  double yc = TMath::Sin(theta) / r;
  double xc = TMath::Cos(theta) / r;
  double rc = fabs(1. / r);

  int vote_pos = 0;
  int vote_neg = 0;

  for (TrackHit* Hit : trackHits) {
    int curve_sign = Hit->getCurvatureSignWrt(xc, yc);

    if (curve_sign == TrackCandidate::charge_positive)
      ++vote_pos;
    else if (curve_sign == TrackCandidate::charge_negative)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if (vote_pos > vote_neg)
    return TrackCandidate::charge_positive;

  else
    return TrackCandidate::charge_negative;

}

