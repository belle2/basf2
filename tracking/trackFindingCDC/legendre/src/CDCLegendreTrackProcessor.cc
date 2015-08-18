/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackDrawer.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <cdc/dataobjects/CDCHit.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidate* TrackProcessor::createLegendreTrackCandidateFromQuadNodeList(const std::vector<QuadTreeLegendre*>& nodeList)
{
  TrackCandidate* trackCandidate = new TrackCandidate(nodeList);
  TrackFitter cdcLegendreTrackFitter;
  cdcLegendreTrackFitter.fitTrackCandidateFast(trackCandidate);

  processTrack(trackCandidate);
  return trackCandidate;
}

TrackCandidate* TrackProcessor::createLegendreTrackCandidateFromHits(const std::vector<TrackHit*>& trackHits)
{
  TrackCandidate* trackCandidate = new TrackCandidate(0, 0, 0, trackHits);
  TrackFitter cdcLegendreTrackFitter;
  cdcLegendreTrackFitter.fitTrackCandidateFast(trackCandidate);

  processTrack(trackCandidate);
  return trackCandidate;
}

void TrackProcessor::processTrack(TrackCandidate* trackCandidate)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  m_trackList.push_back(trackCandidate);

  m_cdcLegendreTrackDrawer->drawTrackCand(trackCandidate);

  for (TrackHit* hit : trackCandidate->getTrackHits()) {
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  m_cdcLegendreTrackDrawer->showPicture();
  m_cdcLegendreTrackDrawer->drawListOfTrackCands(m_trackList);
}

void TrackProcessor::createCDCTrackCandidates(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  tracks.clear();

  if (m_trackList.empty()) return;
  tracks.reserve(m_trackList.size());

  for (TrackCandidate* trackCand : m_trackList) {
    if (trackCand->getTrackHits().size() < 5) continue;
    tracks.emplace_back();
    CDCTrack& newTrackCandidate = tracks.back();

    //find indices of the Hits
    std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

    Vector3D position{trackCand->getReferencePoint()};
    Vector2D momentum{trackCand->getMomentumEstimation()};

    //set the start parameters
    CDCTrajectory2D trajectory2D(position.xy(), momentum,
                                 trackCand->getChargeSign());

    for (TrackHit* trackHit : trackHitVector) {
      RightLeftInfo rlInfo = RIGHT;
      if (trajectory2D.getDist2D(trackHit->getUnderlayingCDCWireHit()->getRefPos2D()) < 0)
        rlInfo = LEFT;
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(trackHit->getOriginalCDCHit(), rlInfo);
      if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
        continue;
      rlWireHit->getWireHit().getAutomatonCell().setTakenFlag();

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trajectory2D);
      newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    }

    // Set the start point of the trajectory to the first hit
    newTrackCandidate.sort();
    if (newTrackCandidate.size() < 5) {
      tracks.pop_back();
      continue;
    }
    trajectory2D.setLocalOrigin(newTrackCandidate.front().getRecoPos2D());

    // Maybe we should reverse the trajectory here, is this right?

    CDCTrajectory3D trajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption());
    newTrackCandidate.setStartTrajectory3D(trajectory3D);

    // Recalculate the perpS of the hits
    for (CDCRecoHit3D& recoHit : newTrackCandidate) {
      recoHit.setPerpS(recoHit.getPerpS(trajectory2D));
    }

  }
}

std::set<TrackHit*> TrackProcessor::createHitSet() const
{
  doForAllTracks([](TrackCandidate * cand) {
    for (TrackHit* hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }
  });
  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  doForAllHits([&hits_set, &it](TrackHit * trackHit) {
    if (trackHit->getHitUsage() != TrackHit::c_usedInTrack)
      it = hits_set.insert(it, trackHit);
  });
  B2DEBUG(90, "In hit set are " << hits_set.size() << " hits.");
  return hits_set;
}

void TrackProcessor::deleteTracksWithASmallNumberOfHits()
{
  // Delete a track if we have to few hits left
  m_trackList.erase(std::remove_if(m_trackList.begin(), m_trackList.end(), [](TrackCandidate * trackCandidate) {
    if (trackCandidate->getNHits() < 3) {
      for (TrackHit* trackHit : trackCandidate->getTrackHits()) {
        trackHit->setHitUsage(TrackHit::c_notUsed);
      }
      return true;
    } else {
      return false;
    }

  }), m_trackList.end());
}

void TrackProcessor::fitAllTracks() const
{
  doForAllTracks([this](TrackCandidate * cand) {
    fitOneTrack(cand);
  });
}

void TrackProcessor::initializeHitListFromWireHitTopology()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  const SortableVector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }
  m_axialHitList.reserve(cdcWireHits.size());
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    if (cdcWireHit.getAutomatonCell().hasTakenFlag()) continue;
    TrackHit* trackHit = new TrackHit(cdcWireHit);
    if (trackHit->checkHitDriftLength() and trackHit->getIsAxial()) {
      m_axialHitList.push_back(trackHit);
    } else {
      delete trackHit;
    }
  }
  B2DEBUG(90,
          "Number of hits to be used by track finder: " << getAxialHitsList().size() << " axial.");
}

void TrackProcessor::deleteHitsOfAllBadTracks() const
{
  SimpleFilter::appendUnusedHits(getTrackList(), getAxialHitsList(), 0.8);
  fitAllTracks();
  doForAllTracks([](TrackCandidate * trackCandidate) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  });
  fitAllTracks();
}

void TrackProcessor::deleteBadHitsOfOneTrack(
  TrackCandidate* trackCandidate)
{
  TrackCandidate* resultSplittedTrack = TrackMerger::splitBack2BackTrack(
                                          trackCandidate);
  if (resultSplittedTrack != nullptr) {
    m_trackList.push_back(resultSplittedTrack);
    fitOneTrack(resultSplittedTrack);
    fitOneTrack(trackCandidate);
  }
  SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  fitOneTrack(trackCandidate);
}

void TrackProcessor::mergeOneTrack(TrackCandidate* trackCandidate)
{
  TrackMerger::tryToMergeTrackWithOtherTracks(trackCandidate, m_trackList);
  // Has merging deleted the candidate?
  if (trackCandidate == nullptr)
    return;

  doForAllTracks([this](TrackCandidate * cand) {
    SimpleFilter::deleteWrongHitsOfTrack(cand, 0.8);
    m_cdcLegendreTrackFitter.fitTrackCandidateFast(cand);
  });
}

void TrackProcessor::mergeAllTracks()
{
  TrackMerger::doTracksMerging(m_trackList);
  doForAllTracks([](TrackCandidate * trackCandidate) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  });
  fitAllTracks();
}

void TrackProcessor::appendHitsOfAllTracks() const
{
  SimpleFilter::reassignHitsFromOtherTracks(getTrackList());
  fitAllTracks();
  SimpleFilter::appendUnusedHits(getTrackList(), getAxialHitsList(), 0.90);
  fitAllTracks();
  doForAllTracks([this](TrackCandidate * trackCandidate) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
    fitOneTrack(trackCandidate);
  });
}

