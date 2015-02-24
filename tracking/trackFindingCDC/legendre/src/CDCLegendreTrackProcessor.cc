/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidate* TrackProcessor::createLegendreTrackCandidateFromQuadNode(QuadTreeLegendre* node)
{
  // Remove used or bad hits
  node->cleanUpItems<QuadTreeProcessor>();

  // Clean hits in the neighborhood
  std::vector<QuadTreeLegendre*> nodeList;
  nodeList.push_back(node);

  node->findNeighbors();
  for (QuadTreeLegendre * nodeNeighbor : node->getNeighborsVector()) {
    if (nodeNeighbor->getNItems() == 0) nodeNeighbor->fillChildrenForced<QuadTreeProcessor>();
    nodeNeighbor->cleanUpItems<QuadTreeProcessor>();
  }

  // Create the legendre Track Candidate from the nodes
  TrackCandidate* trackCandidate = createLegendreTrackCandidateFromQuadNodeList(nodeList);

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

void TrackProcessor::processTrack(TrackCandidate* trackCandidate)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    m_trackList.push_back(trackCandidate);

    m_cdcLegendreTrackDrawer->drawTrackCand(trackCandidate);

    for (TrackHit * hit : trackCandidate->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    m_cdcLegendreTrackDrawer->showPicture();
    m_cdcLegendreTrackDrawer->drawListOfTrackCands(m_trackList);

  }

  else {
    for (TrackHit * hit : trackCandidate->getTrackHits()) {
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

void TrackProcessor::createGFTrackCandidates(string& m_gfTrackCandsColName)
{
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int createdTrackCandidatesCounter = 0;

  for (TrackCandidate * trackCand : m_trackList) {
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
    for (TrackHit * trackHit : trackHitVector) {
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
