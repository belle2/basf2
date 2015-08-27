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

#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/LegendreHit.h>

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
    class CDCTrack;

    class TrackProcessorNew {
    public:

      /**
       * Please note that the implemented methods do only use the axial hits!
       * We use the fitter and the drawer as a pointer to have the possibility to use different classes.
       */
      TrackProcessorNew() :
        m_cdcTracks() { }

      /**
       * Do not copy this class
       */
      TrackProcessorNew(const TrackProcessorNew& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackProcessorNew& operator=(const TrackProcessorNew&) = delete;

      /**
       * Compile the hitList from the wire hit topology.
       */
      void initializeLegendreHits();

      CDCTrack& createLegendreTrackCandidateFromHits(std::vector<LegendreHit*>& trackHits);

      /** Created CDCTracks from the stored CDCLegendreTrackCandidates */
      void createCDCTrackCandidates(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) ;

      int estimateCharge(double theta, double r, std::vector<TrackHit*>& trackHits);

      /**
       * Get the list with currently stored tracks.
       */
      const std::vector<CDCTrack>& getTracks()
      {
        return m_cdcTracks;
      }

      /**
       * Get the list with currently stored tracks.
       */
      const std::vector<LegendreHit>& getLegendreHits()
      {
        return m_legendreHits;
      }

      /**
       * For the use in the QuadTree use this hit set.
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      std::vector<LegendreHit*> createLegendreHitsForQT();



      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       */
      void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate);




      /**
       * Reset all masked hits
       */
      void resetMaskedHits()
      {
        doForAllHits([](LegendreHit & hit) {
          hit.setMaskedFlag(false);
        });
      }

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        m_legendreHits.clear();
        m_cdcTracks.clear();
      }

    private:
      std::vector<CDCTrack> m_cdcTracks; /**< List of track candidates. */
      std::vector<LegendreHit> m_legendreHits; /**< Vector which hold axial hits */

      /**
       * @brief Perform the necessary operations after the track candidate has been constructed
       * @param track The constructed track candidate
       * @param trackHitList list of all hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
       * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
       */
      void processTrack(CDCTrack& track);

      /**
       * Do a certain function for each track in the track list
       */
      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        for (CDCTrack& track : m_cdcTracks) {
          function(track);
        }
      }

      /**
       * Do a certain function for each track in the track list
       */
      void doForAllHits(std::function<void(LegendreHit& hit)> function)
      {
        for (LegendreHit& hit : m_legendreHits) {
          function(hit);
        }
      }


    };
  }
}

