/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreQuadTreeCandidateCreator.h>

using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

QuadTreeCandidateCreator* QuadTreeCandidateCreator::s_cdcLegendreQuadTreeCandidateCreator = 0;
std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > > QuadTreeCandidateCreator::s_candidates; /**< Holds list of track candidates */
std::list<QuadTree*> QuadTreeCandidateCreator::s_nodesWithCandidates;
TrackFitter* QuadTreeCandidateCreator::s_cdcLegendreTrackFitter;
TrackCreator* QuadTreeCandidateCreator::s_cdcLegendreTrackCreator;
TrackMerger* QuadTreeCandidateCreator::s_cdcLegendreTrackMerger;
std::vector<TrackHit*> QuadTreeCandidateCreator::s_axialHits;


QuadTreeCandidateCreator& QuadTreeCandidateCreator::Instance()
{
  if (!s_cdcLegendreQuadTreeCandidateCreator) s_cdcLegendreQuadTreeCandidateCreator = new QuadTreeCandidateCreator();
  return *s_cdcLegendreQuadTreeCandidateCreator;
}

bool QuadTreeCandidateCreator::sort_nodes(const QuadTree* node_one, const QuadTree* node_two)
{
  return (node_one->getNHits() > node_two->getNHits());
}


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
}


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
}

bool QuadTreeCandidateCreator::createCandidateDirect(QuadTree* node)
{


  //B2INFO("Cleaning hits");
  node->cleanHitsInNode();
  if (not node->checkNode()) return false;

  int AxialVsStereo = 0;

  for (TrackHit * hit : node->getHits()) {
    hit->setHitUsage(TrackHit::used_in_cand);
    if (hit->getIsAxial()) AxialVsStereo++;
    else AxialVsStereo--;
  }

  std::vector<QuadTree*> nodeList;
  nodeList.push_back(node);

  node->findNeighbors();
  for (QuadTree * nodeNeighbor : node->getNeighborsVector()) {
    if (nodeNeighbor->getNHits() == 0) nodeNeighbor->fillChildrenForced();
    nodeNeighbor->cleanHitsInNode();
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
    trackCandidate = s_cdcLegendreTrackCreator->createLegendreTrackCandidate(nodeList);
    PatternChecker cdcLegendrePatternChecker(s_cdcLegendreTrackCreator);
    cdcLegendrePatternChecker.checkCandidate(trackCandidate);

    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    trackCandidate->clearBadHits();
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, true);

  } else trackCandidate = s_cdcLegendreTrackCreator->createLegendreStereoTracklet(nodeList);


  int neighborsOrder = 5;

  std::vector<QuadTree*> nodesInitial;
  std::vector<QuadTree*> nodesNeighbors;
  std::vector<QuadTree*> nodesCluster;

  for (QuadTree * node_temp : nodeList) {
    nodesInitial.push_back(node_temp);
    nodesCluster.push_back(node_temp);
  }
  for (int order = 0; order < neighborsOrder; order++) {
    nodesNeighbors.clear();
    for (QuadTree * node_temp : nodesInitial) {
      for (QuadTree * nodeNeighbor : node_temp->getNeighborsVector()) {
        bool nodeToAdd = true;
        for (QuadTree * nodeInVector : nodesCluster) {
          if (nodeInVector == nodeNeighbor) nodeToAdd = false;
        }
        if (nodeToAdd) {
          nodesCluster.push_back(nodeNeighbor);
          nodesNeighbors.push_back(nodeNeighbor);
        }
      }
    }


    nodesInitial.clear();
    for (QuadTree * node_temp : nodesNeighbors) {
      nodesInitial.push_back(node_temp);
    }
  }

  std::vector<TrackHit*> hitsToProcess;

  for (QuadTree * node_temp : nodesCluster) {
    for (TrackHit * hit : node_temp->getHits()) {
      bool hitToAdd = true;
      for (TrackHit * hitInVector : hitsToProcess) {
        if (hitInVector == hit) hitToAdd = false;
      }
      if (hitToAdd) hitsToProcess.push_back(hit);
    }
  }

  s_cdcLegendreTrackMerger->extendTracklet(trackCandidate, hitsToProcess/*s_axialHits*/);



  return true;

}


QuadTree* QuadTreeCandidateCreator::findNode(QuadTree* tree, double r, double theta)
{
  if (((tree->getRMin() < r) && (tree->getRMax() > r)) && ((tree->getThetaMin() < theta) && (tree->getThetaMax() > theta))) {
    if (tree->isLeaf())
      return tree;
    else {
      QuadTree* returnNode = 0;

      for (int rBin = 0; rBin < tree->getRNbins(); rBin++) {
        for (int thetaBin = 0; thetaBin < tree->getThetaNbins(); thetaBin++) {
          returnNode = findNode(tree->getChildren(thetaBin, rBin), r, theta);
          if (returnNode != 0) return returnNode;
        }
      }
    }
  }

  return 0;
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
