/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
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
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>

#include <set>
#include <list>
#include <vector>

#include <algorithm>


namespace Belle2 {

  namespace TrackFindingCDC {

    class TrackProcessor;
    template<typename typeX, typename typeY, class typeData>
    class QuadTreeTemplate;
    class TrackFitter;
    class TrackMerger;

    class QuadTreeCandidateCreator {

    public:
      typedef QuadTreeTemplate<double, int, TrackHit> QuadTreeLegendre;

      QuadTreeCandidateCreator() {};

      ~QuadTreeCandidateCreator();

      static QuadTreeCandidateCreator& Instance();

      /** Set candidate fitter */
      static void setFitter(TrackFitter* cdcLegendreTrackFitter) {s_cdcLegendreTrackFitter = cdcLegendreTrackFitter;};

      /** Set hits */
      static void setHits(std::vector<TrackHit*>& axialHits) {s_axialHits = axialHits;};

      /** Set candidate fitter */
      static void setCandidateCreator(TrackProcessor* cdcLegendreTrackProcessor) {s_cdcLegendreTrackProcessor = cdcLegendreTrackProcessor;};

      /** Add node to the list of nodes with candidated */
      static inline void addNode(QuadTreeLegendre* node) {s_nodesWithCandidates.push_back(node);};

      /** Creating candidates using information from nodes */
      void createCandidates();

      /** Creating candidate using information from given node */
      bool createCandidate(QuadTreeLegendre* node);

      /** Creating candidate using information from given node */
      bool createCandidateDirect(QuadTreeLegendre* node);

      /** Creating candidates using information from nodes */
      std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > >& getCandidates()
      {return s_candidates;};

      /** Clear list of candidates */
      void clearCandidates();

      /** Creating candidates using information from nodes */
      void clearNodes();

      /** Find leaf node with given parameters r and theta */
      QuadTreeLegendre* findNode(QuadTreeLegendre*, double, double);


      /** Sort nodes according to number of hits */
      static bool sort_nodes(const QuadTreeLegendre* node_one, const QuadTreeLegendre* node_two);

      static void setAppendHitsWhileFinding(bool appendHitsWhileFinding) {
        m_appendHitsWhileFinding = appendHitsWhileFinding;
      }

      static void setMergeTracksWhileFinding(bool mergeTracksWhileFinding) {
        m_mergeTracksWhileFinding = mergeTracksWhileFinding;
      }

      static void setDeleteHitsWhileFinding(bool deleteHitsWhileFinding) {
        m_deleteHitsWhileFinding = deleteHitsWhileFinding;
      }

    private:
      /**
       * After havin find the trackCandidate, we try to postprocess only this single trackCandidate.
       * @param trackCandidate
       * @return a flag if everything was correct
       */
      bool postprocessTrackCandidate(TrackCandidate* trackCandidate);

      static std::vector< std::pair<std::vector<TrackHit*>, std::pair<double, double> > > s_candidates; /**< Holds list of track candidates */
      static std::list<QuadTreeLegendre*> s_nodesWithCandidates; /**< List of nodes with possible track candidates */
      static TrackFitter* s_cdcLegendreTrackFitter; /**< Track fitter object */
      static TrackProcessor* s_cdcLegendreTrackProcessor; /**< Track creator object */
      static std::vector<TrackHit*> s_axialHits; /**< Holds list of acial hits */

      static bool m_appendHitsWhileFinding;
      static bool m_mergeTracksWhileFinding;
      static bool m_deleteHitsWhileFinding;

      static QuadTreeCandidateCreator* s_cdcLegendreQuadTreeCandidateCreator; /**< Instance of the class */


    };
  }
}


