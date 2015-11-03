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

#include <tracking/trackFindingCDC/creators/QuadTreeHitWrapperCreator.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCTrackList.h>

#include <list>
#include <vector>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCTrajectory2D;

    class TrackProcessor {
    public:

      /// Default constructor does nothing.
      TrackProcessor() { }

      /// Do not copy this class.
      TrackProcessor(const TrackProcessor& copy) = delete;

      /// Do not copy this class.
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /// Create CDCTrack using CDCWireHit hits and store it in the list
      void addCandidateWithHits(std::vector<const CDCWireHit*>& hits);

      /// Assign new hits to the track basing on the distance from the hit to the track
      void assignNewHits(CDCTrack& track);

      /// Assign new hits to all tracks (using assignNewHits(CDCTrack&) method)
      void assignNewHits();

      /// Check p-value of the track
      void checkTrackProb();

      /// Perform track postprocessing
      void postprocessTrack(CDCTrack& track);

      /**
       * Reset all masked hits
       */
      void resetMaskedHits()
      {
        m_hitFactory.resetMaskedHits(m_cdcTrackList.getCDCTracks());
      }

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        m_hitFactory.clear();
        m_cdcTrackList.clear();
      }

      QuadTreeHitWrapperCreator& getHitFactory()
      {
        return m_hitFactory;
      }

      std::list<CDCTrack>& getCDCTrackList()
      {
        return m_cdcTrackList.getCDCTracks();
      }

      /// Perform provided function to all tracks
      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        m_cdcTrackList.doForAllTracks(function);
      }

    private:
      QuadTreeHitWrapperCreator m_hitFactory; /**< HitFactory object.*/
      CDCTrackList m_cdcTrackList; /**< a list handling the found CDC tracks as an object.*/

      /// Set MASKED flag of automaton cells to false
      void unmaskHitsInTrack(CDCTrack& track);

      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       */
      void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate);


      /// Check chi2 of the fit (using quantiles of chi2 distribution)
      bool checkChi2(CDCTrack& track);

      /** Calculate quantile of chi2
       * @param alpha quntile of chi2
       * @param n number degrees of freedom
       */
      double calculateChi2ForQuantile(double alpha, double n);

      void assignNewHitsUsingSegments(CDCTrack& track, float fraction = 0.3);

      void removeBadSLayers(CDCTrack& track);

      /// Check track -- currently based on number of hits only.
      bool checkTrack(CDCTrack& track);
    };
  }
}

