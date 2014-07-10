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
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackMerger.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <algorithm>

using namespace std;
using namespace Belle2;

CDCLegendreQuadTreeCandidateCreator* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreQuadTreeCandidateCreator = 0;
std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > > CDCLegendreQuadTreeCandidateCreator::s_candidates; /**< Holds list of track candidates */
std::list<CDCLegendreQuadTree*> CDCLegendreQuadTreeCandidateCreator::s_nodesWithCandidates;
CDCLegendreTrackFitter* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreTrackFitter;
CDCLegendreTrackCreator* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreTrackCreator;
CDCLegendreTrackMerger* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreTrackMerger;
std::vector<CDCLegendreTrackHit*> CDCLegendreQuadTreeCandidateCreator::s_axialHits;


CDCLegendreQuadTreeCandidateCreator& CDCLegendreQuadTreeCandidateCreator::Instance()
{
  if (!s_cdcLegendreQuadTreeCandidateCreator) s_cdcLegendreQuadTreeCandidateCreator = new CDCLegendreQuadTreeCandidateCreator();
  return *s_cdcLegendreQuadTreeCandidateCreator;
}

bool CDCLegendreQuadTreeCandidateCreator::sort_nodes(const CDCLegendreQuadTree* node_one, const CDCLegendreQuadTree* node_two)
{
  return (node_one->getNHits() > node_two->getNHits());
}


void CDCLegendreQuadTreeCandidateCreator::createCandidates()
{
  //B2INFO("Sorting nodes");
  s_nodesWithCandidates.sort(CDCLegendreQuadTreeCandidateCreator::sort_nodes);
  //B2INFO("Done sorting nodes");

  for (CDCLegendreQuadTree * node : s_nodesWithCandidates) {
    //B2INFO("Cleaning hits");
    node->cleanHitsInNode();
    if (not node->checkNode()) continue;

    //B2INFO("Good node");


    std::vector<CDCLegendreTrackHit*> c_list;
    std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
      std::make_pair(c_list, std::make_pair(-999, -999));




    for (CDCLegendreTrackHit * hit : node->getHits()) {
      hit->setHitUsage(CDCLegendreTrackHit::used_in_cand);
    }

    candidate_temp.first = node->getHits();
    candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
    //B2INFO("Pushing new candidate");

    s_candidates.push_back(candidate_temp);

  }

  B2INFO("Created " << s_candidates.size() << "candidates");
}


bool CDCLegendreQuadTreeCandidateCreator::createCandidate(CDCLegendreQuadTree* node)
{
  //B2INFO("Cleaning hits");
  node->cleanHitsInNode();
  if (not node->checkNode()) return false;

  //B2INFO("Good node");


  std::vector<CDCLegendreTrackHit*> c_list;
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
    std::make_pair(c_list, std::make_pair(-999, -999));




  for (CDCLegendreTrackHit * hit : node->getHits()) {
    hit->setHitUsage(CDCLegendreTrackHit::used_in_cand);
  }

  candidate_temp.first = node->getHits();
  candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
  //B2INFO("Pushing new candidate");

  s_candidates.push_back(candidate_temp);

  return true;
//  B2INFO("Created candidate");
}

bool CDCLegendreQuadTreeCandidateCreator::createCandidateDirect(CDCLegendreQuadTree* node)
{


  //B2INFO("Cleaning hits");
  node->cleanHitsInNode();
  if (not node->checkNode()) return false;

  int AxialVsStereo = 0;

  for (CDCLegendreTrackHit * hit : node->getHits()) {
    hit->setHitUsage(CDCLegendreTrackHit::used_in_cand);
    if (hit->getIsAxial()) AxialVsStereo++;
    else AxialVsStereo--;
  }

  std::vector<CDCLegendreQuadTree*> nodeList;
  nodeList.push_back(node);

  node->findNeighbors();
  for (CDCLegendreQuadTree * nodeNeighbor : node->getNeighborsVector()) {
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




  CDCLegendreTrackCandidate* trackCandidate;
  if (AxialVsStereo >= 0) {
    trackCandidate = s_cdcLegendreTrackCreator->createLegendreTrackCandidate(nodeList);
    CDCLegendrePatternChecker cdcLegendrePatternChecker(s_cdcLegendreTrackCreator);
    cdcLegendrePatternChecker.checkCandidate(trackCandidate);

    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    trackCandidate->clearBadHits();
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate);
//    s_cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, true);

  } else trackCandidate = s_cdcLegendreTrackCreator->createLegendreStereoTracklet(nodeList);


  int neighborsOrder = 5;

  std::vector<CDCLegendreQuadTree*> nodesInitial;
  std::vector<CDCLegendreQuadTree*> nodesNeighbors;
  std::vector<CDCLegendreQuadTree*> nodesCluster;

  for (CDCLegendreQuadTree * node_temp : nodeList) {
    nodesInitial.push_back(node_temp);
    nodesCluster.push_back(node_temp);
  }
  for (int order = 0; order < neighborsOrder; order++) {
    nodesNeighbors.clear();
    for (CDCLegendreQuadTree * node_temp : nodesInitial) {
      for (CDCLegendreQuadTree * nodeNeighbor : node_temp->getNeighborsVector()) {
        bool nodeToAdd = true;
        for (CDCLegendreQuadTree * nodeInVector : nodesCluster) {
          if (nodeInVector == nodeNeighbor) nodeToAdd = false;
        }
        if (nodeToAdd) {
          nodesCluster.push_back(nodeNeighbor);
          nodesNeighbors.push_back(nodeNeighbor);
        }
      }
    }


    nodesInitial.clear();
    for (CDCLegendreQuadTree * node_temp : nodesNeighbors) {
      nodesInitial.push_back(node_temp);
    }
  }

  std::vector<CDCLegendreTrackHit*> hitsToProcess;

  for (CDCLegendreQuadTree * node_temp : nodesCluster) {
    for (CDCLegendreTrackHit * hit : node_temp->getHits()) {
      bool hitToAdd = true;
      for (CDCLegendreTrackHit * hitInVector : hitsToProcess) {
        if (hitInVector == hit) hitToAdd = false;
      }
      if (hitToAdd) hitsToProcess.push_back(hit);
    }
  }

  s_cdcLegendreTrackMerger->extendTracklet(trackCandidate, hitsToProcess/*s_axialHits*/);



  return true;

}


CDCLegendreQuadTree* CDCLegendreQuadTreeCandidateCreator::findNode(CDCLegendreQuadTree* tree, double r, double theta)
{
  if (((tree->getRMin() < r) && (tree->getRMax() > r)) && ((tree->getThetaMin() < theta) && (tree->getThetaMax() > theta))) {
    if (tree->isLeaf())
      return tree;
    else {
      CDCLegendreQuadTree* returnNode = 0;

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


void CDCLegendreQuadTreeCandidateCreator::clearCandidates()
{
  s_candidates.clear();
}

void CDCLegendreQuadTreeCandidateCreator::clearNodes()
{
  s_nodesWithCandidates.clear();
}


CDCLegendreQuadTreeCandidateCreator::~CDCLegendreQuadTreeCandidateCreator()
{
  if ((s_cdcLegendreQuadTreeCandidateCreator) && (this != s_cdcLegendreQuadTreeCandidateCreator)) {
    delete s_cdcLegendreQuadTreeCandidateCreator;
    s_cdcLegendreQuadTreeCandidateCreator = NULL;
  }

}
