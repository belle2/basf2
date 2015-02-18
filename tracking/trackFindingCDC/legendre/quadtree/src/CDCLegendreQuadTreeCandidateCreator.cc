/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeCandidateCreator.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

QuadTreeCandidateCreator* QuadTreeCandidateCreator::s_cdcLegendreQuadTreeCandidateCreator = 0;
std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > > QuadTreeCandidateCreator::s_candidates; /**< Holds list of track candidates */
std::list<QuadTreeLegendre*> QuadTreeCandidateCreator::s_nodesWithCandidates;
TrackFitter* QuadTreeCandidateCreator::s_cdcLegendreTrackFitter;
TrackProcessor* QuadTreeCandidateCreator::s_cdcLegendreTrackProcessor;
std::vector<TrackHit*> QuadTreeCandidateCreator::s_axialHits;
bool QuadTreeCandidateCreator::m_appendHitsWhileFinding;
bool QuadTreeCandidateCreator::m_mergeTracksWhileFinding;
bool QuadTreeCandidateCreator::m_deleteHitsWhileFinding;


QuadTreeCandidateCreator& QuadTreeCandidateCreator::Instance()
{
  if (!s_cdcLegendreQuadTreeCandidateCreator) s_cdcLegendreQuadTreeCandidateCreator = new QuadTreeCandidateCreator();
  return *s_cdcLegendreQuadTreeCandidateCreator;
}

bool QuadTreeCandidateCreator::createCandidateDirect(QuadTreeLegendre* node)
{
  node->cleanUpItems<QuadTreeProcessor>();

  if (not node->checkNode()) return false;

  int AxialVsStereo = 0;

  for (TrackHit * hit : node->getItemsVector()) {
    hit->setHitUsage(TrackHit::used_in_cand);
    if (hit->getIsAxial()) AxialVsStereo++;
    else AxialVsStereo--;
  }

  std::vector<QuadTreeLegendre*> nodeList;
  nodeList.push_back(node);

  node->findNeighbors();
  for (QuadTreeLegendre * nodeNeighbor : node->getNeighborsVector()) {
    if (nodeNeighbor->getNItems() == 0) nodeNeighbor->fillChildrenForced<QuadTreeProcessor>();
    nodeNeighbor->cleanUpItems<QuadTreeProcessor>();
  }

  /*
    for(CDCLegendreQuadTree* neighbors: node->getNeighborsVector()){
      if(neighbors->checkNode()){
        nodeList.push_back(neighbors);
      }
    }
    */

  TrackCandidate* trackCandidate;
  if (AxialVsStereo >= 0) {
    trackCandidate = s_cdcLegendreTrackProcessor->createLegendreTrackCandidate(nodeList);

//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    trackCandidate->clearBadHits();
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, true);

  } else trackCandidate = s_cdcLegendreTrackProcessor->createLegendreStereoTracklet(nodeList);


  return postprocessTrackCandidate(trackCandidate);

  /*
  UNUSED
  int neighborsOrder = 5;

  std::vector<QuadTreeLegendre*> nodesInitial;
  std::vector<QuadTreeLegendre*> nodesNeighbors;
  std::vector<QuadTreeLegendre*> nodesCluster;

  for (QuadTreeLegendre * node_temp : nodeList) {
    nodesInitial.push_back(node_temp);
    nodesCluster.push_back(node_temp);
  }
  for (int order = 0; order < neighborsOrder; order++) {
    nodesNeighbors.clear();
    for (QuadTreeLegendre * node_temp : nodesInitial) {
      for (QuadTreeLegendre * nodeNeighbor : node_temp->getNeighborsVector()) {
        bool nodeToAdd = true;
        for (QuadTreeLegendre * nodeInVector : nodesCluster) {
          if (nodeInVector == nodeNeighbor) nodeToAdd = false;
        }
        if (nodeToAdd) {
          nodesCluster.push_back(nodeNeighbor);
          nodesNeighbors.push_back(nodeNeighbor);
        }
      }
    }


    nodesInitial.clear();
    for (QuadTreeLegendre * node_temp : nodesNeighbors) {
      nodesInitial.push_back(node_temp);
    }
  }

  std::vector<TrackHit*> hitsToProcess;

  for (QuadTreeLegendre * node_temp : nodesCluster) {
    for (TrackHit * hit : node_temp->getItemsVector()) {
      bool hitToAdd = true;
      for (TrackHit * hitInVector : hitsToProcess) {
        if (hitInVector == hit) hitToAdd = false;
      }
      if (hitToAdd) hitsToProcess.push_back(hit);
    }
  }*/

//  s_cdcLegendreTrackMerger->extendTracklet(trackCandidate, /*hitsToProcess*/ s_axialHits);
}

bool QuadTreeCandidateCreator::postprocessTrackCandidate(TrackCandidate* trackCandidate)
{
  list<TrackCandidate*>& trackList = s_cdcLegendreTrackProcessor->getTrackList();

  // Before postprocessing, we need a fitted trackCandidate
  s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
  trackCandidate->reestimateCharge();

  if (m_deleteHitsWhileFinding) {
    TrackCandidate* resultSplittedTrack = TrackMerger::splitBack2BackTrack(trackCandidate);
    if (resultSplittedTrack != nullptr) {
      trackList.push_back(resultSplittedTrack);
      s_cdcLegendreTrackFitter->fitTrackCandidateFast(resultSplittedTrack);
      resultSplittedTrack->reestimateCharge();

      s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
      trackCandidate->reestimateCharge();
    }

    SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
    trackCandidate->reestimateCharge();
  }

  if (m_mergeTracksWhileFinding) {
    TrackMerger::tryToMergeTrackWithOtherTracks(trackCandidate, trackList, s_cdcLegendreTrackFitter);

    // Has merging deleted the candidate?
    if (trackCandidate == nullptr) return false;

    for (TrackCandidate * cand : trackList) {
      SimpleFilter::deleteWrongHitsOfTrack(cand, 0.8);
      s_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
      cand->reestimateCharge();
    }
  }

  if (m_appendHitsWhileFinding) {
    SimpleFilter::reassignHitsFromOtherTracks(trackList);

    for (TrackCandidate * cand : trackList) {
      s_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
      cand->reestimateCharge();
    }

    SimpleFilter::appendUnusedHits(trackList, s_axialHits, 0.8);

    for (TrackCandidate * cand : trackList) {
      s_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
      cand->reestimateCharge();
    }

    for (TrackCandidate * cand : trackList) {
      SimpleFilter::deleteWrongHitsOfTrack(cand, 0.8);
      s_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);
      cand->reestimateCharge();
    }
  }

  // Delete a track if we have to few hits left
  trackList.erase(std::remove_if(trackList.begin(), trackList.end(), [](TrackCandidate * trackCandidate) {
    return trackCandidate->getNHits() < 3;
  }), trackList.end());

  // UNUSED
  //PatternChecker cdcLegendrePatternChecker(s_cdcLegendreTrackProcessor);
  //cdcLegendrePatternChecker.checkCandidate(trackCandidate);

  return true;
}

void QuadTreeCandidateCreator::clearCandidates()
{
  s_candidates.clear();
}

void QuadTreeCandidateCreator::clearNodes()
{
  s_nodesWithCandidates.clear();
}


QuadTreeCandidateCreator::~QuadTreeCandidateCreator()
{
  if ((s_cdcLegendreQuadTreeCandidateCreator) && (this != s_cdcLegendreQuadTreeCandidateCreator)) {
    delete s_cdcLegendreQuadTreeCandidateCreator;
    s_cdcLegendreQuadTreeCandidateCreator = NULL;
  }

}

// UNUSED
/*
QuadTreeLegendre* QuadTreeCandidateCreator::findNode(QuadTreeLegendre* tree, double r, double theta)
{
  if (((tree->getXMin() < r) && (tree->getXMax() > r)) && ((tree->getYMin() < theta) && (tree->getYMax() > theta))) {
    if (tree->isLeaf())
      return tree;
    else {
      QuadTreeLegendre* returnNode = 0;

      for (int rBin = 0; rBin < tree->getXNbins(); rBin++) {
        for (int thetaBin = 0; thetaBin < tree->getYNbins(); thetaBin++) {
          if (tree->getChildren() == nullptr)
            tree->initializeChildren<QuadTreeProcessor>();
          returnNode = findNode(tree->getChildren()->get(thetaBin, rBin), r, theta);
          if (returnNode != 0) return returnNode;
        }
      }
    }
  }

  return 0;
}*/
/*
bool QuadTreeCandidateCreator::sort_nodes(const QuadTree* node_one, const QuadTree* node_two)
{
  return (node_one->getNHits() > node_two->getNHits());
}
 */
/*
bool QuadTreeCandidateCreator::createCandidate(QuadTree* node)
{
  //B2INFO("Cleaning hits");
  node->cleanHitsInNode();
  if (not node->checkNode()) return false;

  //B2INFO("Good node");


  std::vector<TrackHit*> c_list;
  std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate_temp =
    std::make_pair(c_list, std::make_pair(-999, -999));




  for (TrackHit * hit : node->getHits()) {
    hit->setHitUsage(TrackHit::used_in_cand);
  }

  candidate_temp.first = node->getHits();
  candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
  //B2INFO("Pushing new candidate");

  s_candidates.push_back(candidate_temp);

  return true;
//  B2INFO("Created candidate");
}*/

/*
void QuadTreeCandidateCreator::createCandidates()
{
  //B2INFO("Sorting nodes");
  s_nodesWithCandidates.sort(QuadTreeCandidateCreator::sort_nodes);
  //B2INFO("Done sorting nodes");

  for (QuadTree * node : s_nodesWithCandidates) {
    //B2INFO("Cleaning hits");
    node->cleanHitsInNode();
    if (not node->checkNode()) continue;

    //B2INFO("Good node");


    std::vector<TrackHit*> c_list;
    std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate_temp =
      std::make_pair(c_list, std::make_pair(-999, -999));




    for (TrackHit * hit : node->getHits()) {
      hit->setHitUsage(TrackHit::used_in_cand);
    }

    candidate_temp.first = node->getHits();
    candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
    //B2INFO("Pushing new candidate");

    s_candidates.push_back(candidate_temp);

  }

  B2INFO("Created " << s_candidates.size() << "candidates");
}*/
