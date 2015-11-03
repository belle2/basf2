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

#include <tracking/trackFindingCDC/eventdata/collections/CDCTrackList.h>

#include <list>
#include <vector>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCTrajectory2D;
    class ConformalCDCWireHit;

    class TrackProcessor {
    public:

      /// Default constructor does nothing.
      TrackProcessor() { }

      /// Do not copy this class.
      TrackProcessor(const TrackProcessor& copy) = delete;

      /// Do not copy this class.
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /// Create CDCTrack using CDCWireHit hits and store it in the list
      static void addCandidateWithHits(std::vector<const CDCWireHit*>& hits,
                                       const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                       CDCTrackList& cdcTrackList);

      /// Assign new hits to the track basing on the distance from the hit to the track
      static void assignNewHits(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

      /// Assign new hits to all tracks (using assignNewHits(CDCTrack&) method)
      static void assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList);

      /// Check p-value of the track
      static void checkTrackProb(CDCTrackList& cdcTrackList);

      /// Perform track postprocessing
      static void postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                   CDCTrackList& cdcTrackList);

      CDCTrackList& getCDCTrackListTmp()
      {
        return m_cdcTrackList;
      }

    private:
      CDCTrackList m_cdcTrackList; /**< a list handling the found CDC tracks as an object.*/

      /// Set MASKED flag of automaton cells to false
      static void unmaskHitsInTrack(CDCTrack& track);

      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       */
      static void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate);


      /// Check chi2 of the fit (using quantiles of chi2 distribution)
      static bool checkChi2(CDCTrack& track);

      /** Calculate quantile of chi2
       * @param alpha quntile of chi2
       * @param n number degrees of freedom
       */
      static double calculateChi2ForQuantile(double alpha, double n);

      static void assignNewHitsUsingSegments(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                             float fraction = 0.3);

      static void removeBadSLayers(CDCTrack& track);

      /// Check track -- currently based on number of hits only.
      static bool checkTrack(CDCTrack& track);
    };
  }
}

