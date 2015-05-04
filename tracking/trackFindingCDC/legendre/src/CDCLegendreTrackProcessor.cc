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
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackingSortHit.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidate* TrackProcessor::createLegendreTrackCandidateFromQuadNode(QuadTreeLegendre* node)
{
  QuadTreeProcessor qtProcessor(13);
  // Remove used or bad hits
  node->cleanUpItems(qtProcessor);

  // Clean hits in the neighborhood
  std::vector<QuadTreeLegendre*> nodeList;
  nodeList.push_back(node);

  /*  node->findNeighbors();
    for (QuadTreeLegendre * nodeNeighbor : node->getNeighborsVector()) {
      if (nodeNeighbor->getNItems() == 0) nodeNeighbor->fillChildrenForced<QuadTreeProcessor>();
      nodeNeighbor->cleanUpItems<QuadTreeProcessor>();
    }*/

  // Create the legendre Track Candidate from the nodes
  TrackCandidate* trackCandidate = createLegendreTrackCandidateFromQuadNodeList(nodeList);

  B2DEBUG(90, "Found new track with " << trackCandidate->getNHits() << " hits.")

  return trackCandidate;
}

TrackCandidate* TrackProcessor::createLegendreTrackCandidateFromQuadNodeList(const std::vector<QuadTreeLegendre*>& nodeList)
{
  TrackCandidate* trackCandidate = new TrackCandidate(nodeList);
  TrackFitter cdcLegendreTrackFitter;
  cdcLegendreTrackFitter.fitTrackCandidateFast(trackCandidate);
  trackCandidate->setCandidateType(TrackCandidate::goodTrack);

  processTrack(trackCandidate);
  return trackCandidate;
}

TrackCandidate* TrackProcessor::createTracklet(std::vector<TrackHit*>& hits)
{
  std::pair<double, double> track_par;
  std::pair<double, double> ref_point;
  TrackFitter cdcLegendreTrackFitter;
  cdcLegendreTrackFitter.fitTrackCandidateFast(hits, track_par, ref_point);
  int m_charge = TrackCandidate::getChargeAssumption(track_par.first, track_par.second, hits);

  TrackCandidate* trackCandidate = new TrackCandidate(track_par.first, track_par.second, m_charge, hits);
  processTrack(trackCandidate);

  trackCandidate->setCandidateType(TrackCandidate::tracklet);
  return trackCandidate;
}

void TrackProcessor::processTrack(TrackCandidate* trackCandidate)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    m_trackList.push_back(trackCandidate);

    m_cdcLegendreTrackDrawer->drawTrackCand(trackCandidate);

    for (TrackHit* hit : trackCandidate->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    m_cdcLegendreTrackDrawer->showPicture();
    m_cdcLegendreTrackDrawer->drawListOfTrackCands(m_trackList);

  }

  else {
    for (TrackHit* hit : trackCandidate->getTrackHits()) {
      hit->setHitUsage(TrackHit::bad);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}

bool TrackProcessor::fullfillsQualityCriteria(TrackCandidate* /*trackCandidate*/)
{
  return true;
}

void TrackProcessor::createCDCTrackCandidates(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  tracks.clear();
  tracks.reserve(m_trackList.size());
  int createdTrackCandidatesCounter = 0;

  for (TrackCandidate* trackCand : m_trackList) {
    if (trackCand->getTrackHits().size() < 5) continue;
    tracks.emplace_back();
    CDCTrack& newTrackCandidate = tracks.back();

    TVector3 position = trackCand->getReferencePoint();
    TVector3 momentum = trackCand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = trackCand->getChargeSign() * (211);

    //set the start parameters
    CDCTrajectory2D trajectory2D(Vector2D(position.x(), position.y()), Vector2D(momentum.x(), momentum.y()),
                                 trackCand->getChargeSign());
    CDCTrajectory3D trajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption());
    newTrackCandidate.setStartTrajectory3D(trajectory3D);

    B2DEBUG(100, "Create genfit::TrackCandidate " << createdTrackCandidatesCounter << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")");

    //find indices of the Hits
    std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());

    unsigned int sortingParameter = 0;
    for (TrackHit* trackHit : trackHitVector) {
      // TODO: Can we determine the plane?
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(trackHit->getOriginalCDCHit(), 0);
      rlWireHit->getWireHit().getAutomatonCell().setTakenFlag();

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trajectory2D);
      newTrackCandidate.push_back(std::move(cdcRecoHit3D));
      sortingParameter++;
    }
    ++createdTrackCandidatesCounter;
  }
}

void TrackProcessor::createGFTrackCandidates(const string& m_gfTrackCandsColName)
{
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int createdTrackCandidatesCounter = 0;

  for (TrackCandidate* trackCand : m_trackList) {
    if (trackCand->getTrackHits().size() < 5) continue;
    genfit::TrackCand* newTrackCandidate = gfTrackCandidates.appendNew();

    TVector3 position = trackCand->getReferencePoint();
    TVector3 momentum = trackCand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = trackCand->getChargeSign() * (211);

    //set the start parameters
    newTrackCandidate->setPosMomSeedAndPdgCode(position, momentum, pdg);

    B2DEBUG(100, "Create genfit::TrackCandidate " << createdTrackCandidatesCounter << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")");

    //find indices of the Hits
    std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());

    unsigned int sortingParameter = 0;
    for (TrackHit* trackHit : trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      // TODO: Can we determine the plane?
      newTrackCandidate->addHit(Const::CDC, hitID, -1, sortingParameter);
      sortingParameter++;
    }
    ++createdTrackCandidatesCounter;
  }
}

void TrackProcessor::sortHits(std::vector<TrackHit*>& hits, int charge)
{
  SortHits sorter(charge);
  stable_sort(hits.begin(), hits.end(), sorter);
}

std::set<TrackHit*> TrackProcessor::createHitSet()
{
  for (TrackCandidate* cand : m_trackList) {
    for (TrackHit* hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }
  std::sort(m_axialHitList.begin(), m_axialHitList.end());
  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit* trackHit : m_axialHitList) {
    if ((trackHit->getHitUsage() != TrackHit::used_in_track)
        && (trackHit->getHitUsage() != TrackHit::background))
      it = hits_set.insert(it, trackHit);
  }
  B2DEBUG(90, "In hit set are " << hits_set.size() << " hits.")
  return hits_set;
}

void TrackProcessor::deleteTracksWithASmallNumberOfHits()
{
  // Delete a track if we have to few hits left
  m_trackList.erase(std::remove_if(m_trackList.begin(), m_trackList.end(), [](TrackCandidate * trackCandidate) {
    if (trackCandidate->getNHits() < 3) {
      for (TrackHit* trackHit : trackCandidate->getTrackHits()) {
        trackHit->setHitUsage(TrackHit::not_used);
      }
      return true;
    } else {
      return false;
    }

  }), m_trackList.end());
}

void TrackProcessor::fitAllTracks()
{
  for (TrackCandidate* cand : m_trackList) {
    fitOneTrack(cand);
  }
}


void TrackProcessor::initializeHitList(const StoreArray<CDCHit>& cdcHits)
{
  B2DEBUG(90, "Number of digitized hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }
  m_axialHitList.reserve(2048);
  for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
    TrackHit* trackHit = new TrackHit(cdcHits[iHit], iHit);
    if (trackHit->checkHitDriftLength() && trackHit->getIsAxial()) {
      m_axialHitList.push_back(trackHit);
    } else {
      delete trackHit;
    }
  }
  B2DEBUG(90,
          "Number of hits to be used by track finder: " << m_axialHitList.size());
}

void TrackProcessor::initializeHitListFromWireHitTopology()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  const SortableVector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }
  m_axialHitList.reserve(2048);
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    const CDCHit* cdcHit = cdcWireHit.getHit();
    TrackHit* trackHit = new TrackHit(cdcHit, cdcWireHit.getStoreIHit());
    if (trackHit->checkHitDriftLength() && trackHit->getIsAxial()) {
      m_axialHitList.push_back(trackHit);
    } else {
      delete trackHit;
    }
  }
  B2DEBUG(90,
          "Number of hits to be used by track finder: " << m_axialHitList.size());
}

void TrackProcessor::deleteHitsOfAllBadTracks()
{
  SimpleFilter::appendUnusedHits(m_trackList, m_axialHitList, 0.8);
  fitAllTracks();
  for (TrackCandidate* trackCandidate : m_trackList) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  }
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

  for (TrackCandidate* cand : m_trackList) {
    SimpleFilter::deleteWrongHitsOfTrack(cand, 0.8);
    m_cdcLegendreTrackFitter.fitTrackCandidateFast(cand);
    cand->reestimateCharge();
  }
}

void TrackProcessor::mergeAllTracks()
{
  TrackMerger::doTracksMerging(m_trackList);
  for (TrackCandidate* trackCandidate : m_trackList) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  }
  fitAllTracks();
}

void TrackProcessor::appendHitsOfAllTracks()
{
  SimpleFilter::reassignHitsFromOtherTracks(m_trackList);
  fitAllTracks();
  SimpleFilter::appendUnusedHits(m_trackList, m_axialHitList, 0.90);
  fitAllTracks();
  for (TrackCandidate* trackCandidate : m_trackList) {
    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
    fitOneTrack(trackCandidate);
  }
}

