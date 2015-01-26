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

#include <tracking/trackFindingCDC/legendre/CDCLegendrePatternChecker.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>

#include <set>
#include <list>
#include <vector>

#include <algorithm>


namespace Belle2 {

  namespace TrackFindingCDC {

    class TrackProcessor;
    class QuadTree;
    class TrackFitter;
    class TrackMerger;

    class QuadTreeCandidateCreator {

    public:
      QuadTreeCandidateCreator() {};

      ~QuadTreeCandidateCreator();

      static QuadTreeCandidateCreator& Instance();

      /** Set candidate fitter */
      static void setFitter(TrackFitter* cdcLegendreTrackFitter) {s_cdcLegendreTrackFitter = cdcLegendreTrackFitter;};

      /** Set candidate merger */
      static void setMerger(TrackMerger* cdcLegendreTrackMerger) {s_cdcLegendreTrackMerger = cdcLegendreTrackMerger;};

      /** Set hits */
      static void setHits(std::vector<TrackHit*>& axialHits) {s_axialHits = axialHits;};

      /** Set candidate fitter */
      static void setCandidateCreator(TrackProcessor* cdcLegendreTrackProcessor) {s_cdcLegendreTrackProcessor = cdcLegendreTrackProcessor;};

      /** Add node to the list of nodes with candidated */
      static inline void addNode(QuadTree* node) {s_nodesWithCandidates.push_back(node);};

      /** Creating candidates using information from nodes */
      void createCandidates();

      /** Creating candidate using information from given node */
      bool createCandidate(QuadTree* node);

      /** Creating candidate using information from given node */
      bool createCandidateDirect(QuadTree* node);

      /** Creating candidates using information from nodes */
      std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > >& getCandidates()
      {return s_candidates;};

      /** Clear list of candidates */
      void clearCandidates();

      /** Creating candidates using information from nodes */
      void clearNodes();

      /** Find leaf node with given parameters r and theta */
      QuadTree* findNode(QuadTree*, double, double);


      /** Sort nodes according to number of hits */
      static bool sort_nodes(const QuadTree* node_one, const QuadTree* node_two);


    private:
      static std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > > s_candidates; /**< Holds list of track candidates */
      static std::list<QuadTree*> s_nodesWithCandidates; /**< List of nodes with possible track candidates */
      static TrackFitter* s_cdcLegendreTrackFitter; /**< Track fitter object */
      static TrackProcessor* s_cdcLegendreTrackProcessor; /**< Track creator object */
      static TrackMerger* s_cdcLegendreTrackMerger; /**< Track merger object */
      static std::vector<TrackHit*> s_axialHits; /**< Holds list of acial hits */

      static QuadTreeCandidateCreator* s_cdcLegendreQuadTreeCandidateCreator; /**< Instance of the class */


    };
  }
}


