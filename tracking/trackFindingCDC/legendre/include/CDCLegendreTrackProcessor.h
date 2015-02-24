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

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackDrawer.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackingSortHit.h>

#include <list>
#include <vector>
#include <string>

using namespace std;

namespace Belle2 {
  namespace TrackFindingCDC {

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
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /**
       * Create track candidate using CDCLegendreQuadTree nodes and return pointer to created candidate
       */
      TrackCandidate* createLegendreTrackCandidateFromQuadNodeList(const std::vector<QuadTreeLegendre*>& nodeList);

      TrackCandidate* createLegendreTrackCandidateFromQuadNode(QuadTreeLegendre* node);

      /** Create tracklet using vector of hits and store it
       * UNUSED AS THE CLASS PATTERN_CHECKER IS UNUSED*/
      TrackCandidate* createLegendreTracklet(std::vector<TrackHit*>& hits);

      /** Creates GeantFit Track Candidates from the stored CDCLegendreTrackCandidates */
      void createGFTrackCandidates(string& m_gfTrackCandsColName);

      /** Sort hits for fitting.
       * This method sorts hits to bring them in a correct order, which is needed for the fitting
       */
      static void sortHits(std::vector<TrackHit*>& hits, int charge);

      /**
       * Get the list with currently processed tracks.
       */
      list<TrackCandidate*>& getTrackList() {
        return m_trackList;
      }

      /**
       * Get the list with currently stored axial hits.
       */
      std::vector<TrackHit*>& getAxialHitsList() {
        return m_axialHitList;
      }

      void initializeHitList(const StoreArray<CDCHit> cdcHits) {
        B2DEBUG(90, "Number of digitized hits: " << cdcHits.getEntries());

        if (cdcHits.getEntries() == 0) {
          B2WARNING("cdcHitsCollection is empty!");
        }

        m_axialHitList.reserve(2048);
        for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
          TrackHit* trackHit = new TrackHit(cdcHits[iHit], iHit);
          if (trackHit->checkHitDriftLength() and trackHit->getIsAxial()) {
            m_axialHitList.push_back(trackHit);
          } else {
            delete trackHit;
          }
        }

        B2DEBUG(90, "Number of hits to be used by track finder: " << m_axialHitList.size());
      }

      void clearVectors() {
        for (TrackHit * hit : m_axialHitList) {
          delete hit;
        }
        m_axialHitList.clear();

        for (TrackCandidate * track : m_trackList) {
          delete track;
        }
        m_trackList.clear();
      }

      void deleteHitsOfAllBadTracks() {
        SimpleFilter::appendUnusedHits(m_trackList, m_axialHitList, 0.8);
        fitAllTracks();
        for (TrackCandidate * trackCandidate : m_trackList) {
          SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
        }
        fitAllTracks();
      }

      void deleteBadHitsOfOneTrack(TrackCandidate* trackCandidate) {
        TrackCandidate* resultSplittedTrack = TrackMerger::splitBack2BackTrack(trackCandidate);
        if (resultSplittedTrack != nullptr) {
          m_trackList.push_back(resultSplittedTrack);
          fitOneTrack(resultSplittedTrack);
          fitOneTrack(trackCandidate);
        }
        SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
        fitOneTrack(trackCandidate);
      }

      void mergeOneTrack(TrackCandidate* trackCandidate) {
        TrackMerger::tryToMergeTrackWithOtherTracks(trackCandidate, m_trackList);

        // Has merging deleted the candidate?
        if (trackCandidate == nullptr) return;

        for (TrackCandidate * cand : m_trackList) {
          SimpleFilter::deleteWrongHitsOfTrack(cand, 0.8);
          m_cdcLegendreTrackFitter.fitTrackCandidateFast(cand);
          cand->reestimateCharge();
        }
      }

      void mergeAllTracks() {
        TrackMerger::doTracksMerging(m_trackList);
        for (TrackCandidate * trackCandidate : m_trackList) {
          SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
        }
        fitAllTracks();
      }

      void appendHitsOfAllTracks() {
        SimpleFilter::reassignHitsFromOtherTracks(m_trackList);
        fitAllTracks();
        SimpleFilter::appendUnusedHits(m_trackList, m_axialHitList, 0.8);
        fitAllTracks();
        for (TrackCandidate * trackCandidate : m_trackList) {
          SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
          fitOneTrack(trackCandidate);
        }
      }

      void fitAllTracks()  {
        for (TrackCandidate * cand : m_trackList) {
          fitOneTrack(cand);
        }
      }

      void fitOneTrack(TrackCandidate* trackCandidate) {
        m_cdcLegendreTrackFitter.fitTrackCandidateFast(trackCandidate);
        trackCandidate->reestimateCharge();
      }

      void setTrackDrawer(TrackDrawer* cdcLegendreTrackDrawer) {
        m_cdcLegendreTrackDrawer = cdcLegendreTrackDrawer;
      }

      void deleteTracksWithASmallNumberOfHits() {
        // Delete a track if we have to few hits left
        m_trackList.erase(std::remove_if(m_trackList.begin(), m_trackList.end(), [](TrackCandidate * trackCandidate) {
          return trackCandidate->getNHits() < 3;
        }), m_trackList.end());
      }

      std::set<TrackHit*> createHitSet() {
        for (TrackCandidate * cand : m_trackList) {
          for (TrackHit * hit : cand->getTrackHits()) {
            hit->setHitUsage(TrackHit::used_in_track);
          }
        }


        std::sort(m_axialHitList.begin(), m_axialHitList.end());
        std::set<TrackHit*> hits_set;
        std::set<TrackHit*>::iterator it = hits_set.begin();
        for (TrackHit * trackHit : m_axialHitList) {
          if ((trackHit->getHitUsage() != TrackHit::used_in_track) && (trackHit->getHitUsage() != TrackHit::background)) it = hits_set.insert(it, trackHit);
        }

        B2DEBUG(90, "In hit set are " << hits_set.size() << " hits.")

        return hits_set;
      }


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

