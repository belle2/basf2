/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <set>
#include <list>
#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;
  class CDCLegendreQuadTree;
  class CDCLegendreTrackFitter;
  class CDCLegendreTrackCreator;
  class CDCLegendreTrackMerger;


  class CDCLegendreQuadTreeCandidateCreator {

  public:
    CDCLegendreQuadTreeCandidateCreator() {};

    ~CDCLegendreQuadTreeCandidateCreator();

    static CDCLegendreQuadTreeCandidateCreator& Instance();

    /**
     * Set candidate fitter
     */
    static void setFitter(CDCLegendreTrackFitter* cdcLegendreTrackFitter) {s_cdcLegendreTrackFitter = cdcLegendreTrackFitter;};

    /**
     * Set candidate merger
     */
    static void setMerger(CDCLegendreTrackMerger* cdcLegendreTrackMerger) {s_cdcLegendreTrackMerger = cdcLegendreTrackMerger;};

    /**
     * Set hits
     */
    static void setHits(std::vector<CDCLegendreTrackHit*>& axialHits) {s_axialHits = axialHits;};

    /**
     * Set candidate fitter
     */
    static void setCandidateCreator(CDCLegendreTrackCreator* cdcLegendreTrackCreator) {s_cdcLegendreTrackCreator = cdcLegendreTrackCreator;};

    /**
     * Add node to the list of nodes with candidated
     */
    static inline void addNode(CDCLegendreQuadTree* node) {s_nodesWithCandidates.push_back(node);};

    /**
     * Creating candidates using information from nodes
     */
    void createCandidates();

    /**
     * Creating candidate using information from given node
     */
    bool createCandidate(CDCLegendreQuadTree* node);

    /**
     * Creating candidate using information from given node
     */
    bool createCandidateDirect(CDCLegendreQuadTree* node);

    /**
     * Creating candidates using information from nodes
     */
    std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > >& getCandidates()
    {return s_candidates;};

    /**
     * Clear list of candidates
     */
    void clearCandidates();

    /**
     * Creating candidates using information from nodes
     */
    void clearNodes();

    /**
     * Find leaf node with given parameters r and theta
     */
    CDCLegendreQuadTree* findNode(CDCLegendreQuadTree*, double, double);


    /**
     * Sort nodes according to number of hits
     */
    static bool sort_nodes(const CDCLegendreQuadTree* node_one, const CDCLegendreQuadTree* node_two);


  private:
    static std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > > s_candidates; /**< Holds list of track candidates */
    static std::list<CDCLegendreQuadTree*> s_nodesWithCandidates; /**< List of nodes with possible track candidates */
    static CDCLegendreTrackFitter* s_cdcLegendreTrackFitter; /**< Track fitter object */
    static CDCLegendreTrackCreator* s_cdcLegendreTrackCreator; /**< Track creator object */
    static CDCLegendreTrackMerger* s_cdcLegendreTrackMerger; /**< Track merger object */
    static std::vector<CDCLegendreTrackHit*> s_axialHits; /**< Holds list of acial hits */

    static CDCLegendreQuadTreeCandidateCreator* s_cdcLegendreQuadTreeCandidateCreator; /**< Instance of the class */


  };
}


