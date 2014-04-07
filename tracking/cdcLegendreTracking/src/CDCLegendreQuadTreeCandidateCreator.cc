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

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <algorithm>

using namespace std;
using namespace Belle2;

CDCLegendreQuadTreeCandidateCreator* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreQuadTreeCandidateCreator = 0;
std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > > CDCLegendreQuadTreeCandidateCreator::s_candidates; /**< Holds list of track candidates */
std::list<CDCLegendreQuadTree*> CDCLegendreQuadTreeCandidateCreator::s_nodesWithCandidates;
CDCLegendreTrackFitter* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreTrackFitter;
CDCLegendreTrackCreator* CDCLegendreQuadTreeCandidateCreator::s_cdcLegendreTrackCreator;

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
      hit->setUsed(CDCLegendreTrackHit::used_in_cand);
    }

    candidate_temp.first = node->getHits();
    candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
    //B2INFO("Pushing new candidate");

    s_candidates.push_back(candidate_temp);

  }

  B2INFO("Created " << s_candidates.size() << "candidates");
}


void CDCLegendreQuadTreeCandidateCreator::createCandidate(CDCLegendreQuadTree* node)
{
  //B2INFO("Cleaning hits");
  node->cleanHitsInNode();
  if (not node->checkNode()) return;

  //B2INFO("Good node");


  std::vector<CDCLegendreTrackHit*> c_list;
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
    std::make_pair(c_list, std::make_pair(-999, -999));




  for (CDCLegendreTrackHit * hit : node->getHits()) {
    hit->setUsed(CDCLegendreTrackHit::used_in_cand);
  }

  candidate_temp.first = node->getHits();
  candidate_temp.second = std::make_pair(node->getThetaMean(), node->getRMean());
  //B2INFO("Pushing new candidate");

  s_candidates.push_back(candidate_temp);

  B2INFO("Created candidate");
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
