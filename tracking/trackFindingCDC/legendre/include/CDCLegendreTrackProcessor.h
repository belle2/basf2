/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>

#include <list>
#include <vector>
#include <string>
#include <set>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class TrackDrawer;
    class TrackHit;

    class TrackProcessor {
    public:

      /**
       * We use the fitter and the drawer as a pointer to have the possibility to use different classes.
       */
      TrackProcessor() :
        m_axialHitList(), m_trackList(), m_cdcLegendreTrackDrawer(nullptr) { }

      /**
       * Do not copy this class
       */
      TrackProcessor(const TrackProcessor& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /**
       * Using a node from the quad tree we create a new track candidate with all hits.
       */
      TrackCandidate* createLegendreTrackCandidateFromQuadNode(QuadTreeLegendre* node);

      /**
       * Create track candidate using CDCLegendreQuadTree nodes and return pointer to created candidate
       */
      TrackCandidate* createLegendreTrackCandidateFromQuadNodeList(const std::vector<QuadTreeLegendre*>& nodeList);

      /** Create tracklet using vector of hits and store it
       * UNUSED AS THE CLASS PATTERN_CHECKER IS UNUSED*/
      TrackCandidate* createTracklet(std::vector<TrackHit*>& hits);

      /** Creates GeantFit Track Candidates from the stored CDCLegendreTrackCandidates */
      void createGFTrackCandidates(const string& m_gfTrackCandsColName);

      /** Sort hits for fitting.
       * This method sorts hits to bring them in a correct order, which is needed for the fitting
       */
      static void sortHits(std::vector<TrackHit*>& hits, int charge);

      /**
       * Get the list with currently processed tracks.
       */
      list<TrackCandidate*>& getTrackList()
      {
        return m_trackList;
      }

      /**
       * Get the list with currently stored axial hits.
       */
      std::vector<TrackHit*>& getAxialHitsList()
      {
        return m_axialHitList;
      }

      /**
       * Compile the hitList from the StoreArray with CDCHits.
       * @param cdcHits
       */
      void initializeHitList(const StoreArray<CDCHit>& cdcHits);

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        for (TrackHit* hit : m_axialHitList) {
          delete hit;
        }
        m_axialHitList.clear();

        for (TrackCandidate* track : m_trackList) {
          delete track;
        }
        m_trackList.clear();
      }

      /**
       * Postprocessing: Delete hits that do not "match" to the tracks.
       */
      void deleteHitsOfAllBadTracks();

      /**
       * Postprocessing: Delete hits that do not "match" to the given track.
       */
      void deleteBadHitsOfOneTrack(TrackCandidate* trackCandidate);

      /**
       * Postprocessing: Try to merge this track candidate to the others.
       */
      void mergeOneTrack(TrackCandidate* trackCandidate);

      /**
       * Postprocessing: Try to merge all track candidates with all others.
       */
      void mergeAllTracks();

      /**
       * Postprocessing: Try to reassign hits between all tracks.
       */
      void appendHitsOfAllTracks();

      /**
       * Fit all tracks and reestimate charge.
       */
      void fitAllTracks();

      /**
       * Fits one track and reestimates the charge.
       * @param trackCandidate
       */
      void fitOneTrack(TrackCandidate* trackCandidate)
      {
        m_cdcLegendreTrackFitter.fitTrackCandidateFast(trackCandidate);
        trackCandidate->reestimateCharge();
      }

      /**
       * Sets the track drawer.
       */
      void setTrackDrawer(TrackDrawer* cdcLegendreTrackDrawer)
      {
        m_cdcLegendreTrackDrawer = cdcLegendreTrackDrawer;
      }

      /**
       * After processing we want to delete all tracks which are too small.
       */
      void deleteTracksWithASmallNumberOfHits();

      /**
       * For the use in the QuadTree use this hit set.
       * @return the hit set to use in the QuadTree-Finding.
       */
      std::set<TrackHit*> createHitSet();


    private:
      std::vector<TrackHit*> m_axialHitList; /**< Vector which hold axial hits */
      std::list<TrackCandidate*> m_trackList; /**< List of track candidates. */
      TrackDrawer* m_cdcLegendreTrackDrawer; /**< Class which performs in-module drawing */
      TrackFitter m_cdcLegendreTrackFitter; /**< Used for fitting the tracks */

      /**
       * @brief Implementation of check for quality criteria after the track candidate was produced.
       * @warning not used in the moment
       */
      bool fullfillsQualityCriteria(TrackCandidate* cand);

      /**
       * @brief Perform the necessary operations after the track candidate has been constructed
       * @param track The constructed track candidate
       * @param trackHitList list of all hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
       * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
       */
      void processTrack(TrackCandidate* track);

    };
  }
}

