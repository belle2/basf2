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
#include <tracking/trackFindingCDC/legendre/HitFactory.h>
#include <tracking/trackFindingCDC/legendre/TrackHolder.h>
#include <framework/datastore/StoreArray.h>

#include <list>
#include <vector>
#include <set>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCTrajectory2D;

    class TrackProcessor {
    public:

      TrackProcessor(HitFactory& hitFactory, TrackHolder& trackHolder) :
        m_hitFactory(hitFactory),
        m_trackHolder(trackHolder),
        m_trackFitter() { }

      /**
       * Do not copy this class
       */
      TrackProcessor(const TrackProcessor& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      void removeBadSLayers(CDCTrack& track);

      /// Create CDCTrack using QuadTreeHitWrapper hits and store it in the list
      void createCDCTrackCandidates(std::vector<QuadTreeHitWrapper*>& trackHits);

      /// Create CDCTrack using CDCWireHit hits and store it in the list
      void createCandidate(std::vector<const CDCWireHit*>& hits);

      /// Update trajectory of the CDCTrack
      void updateTrack(CDCTrack& track);

      /// Check track -- currently based on number of hits only.
      bool checkTrack(CDCTrack& track);

      /// Assign new hits to the track basing on the distance from the hit to the track
      void assignNewHits(CDCTrack& track);

      void assignNewHitsUsingSegments(CDCTrack& track, float fraction = 0.3);

      /// Assign new hits to all tracks (using assignNewHits(CDCTrack&) method)
      void assignNewHits();

      /// Check p-value of the track
      void checkTrackProb();

      /// Check chi2 of the fit (using quantiles of chi2 distribution)
      bool checkChi2(CDCTrack& track);

      /** Calculate quantile of chi2
       * @param alpha quntile of chi2
       * @param n number degrees of freedom
       */
      double calculateChi2ForQuantile(double alpha, double n);

      /// Fit CDCTrack object
      CDCTrajectory2D fit(CDCTrack& track);

      /// Set MASKED flag of automaton cells to false
      void unmaskHitsInTrack(CDCTrack& track);

      /// Perform track postprocessing
      void postprocessTrack(CDCTrack& track);

      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       */
      void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate);

      /**
       * Reset all masked hits
       */
      void resetMaskedHits()
      {
        m_hitFactory.resetMaskedHits(m_trackHolder.getCDCTracks());
      }

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        m_hitFactory.clearVectors();
        m_trackHolder.clearVectors();
      }

      static bool isCurler(CDCTrack& track) { return fabs(track.getStartTrajectory3D().getCurvatureXY()) > 0.017; }

      CDCRiemannFitter& getFitter() {return m_trackFitter;};

    private:
      HitFactory& m_hitFactory; /**< Reference to common HitFactory object.*/
      TrackHolder& m_trackHolder; /**< Reference to common TrackHolder object.*/

      CDCRiemannFitter m_trackFitter;

    };
  }
}

