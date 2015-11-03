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

      /// Static use only.
      TrackProcessor() = delete;

      /// Do not copy this class.
      TrackProcessor(const TrackProcessor& copy) = delete;

      /// Do not copy this class.
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /// Create CDCTrack using CDCWireHit hits and store it in the list
      static void addCandidateWithHits(std::vector<const CDCWireHit*>& hits,
                                       const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                       CDCTrackList& cdcTrackList);

      /// Assign new hits to all tracks (using assignNewHits(CDCTrack&) method)
      static void assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList);

      /// Check the p-values of the tracks. If they are below the given value, delete the track from the list.
      static void deleteTracksWithLowFitProbability(CDCTrackList& cdcTrackList, double minimal_probability_for_good_fit = 0.4);

      /// Perform all track postprocessing.
      static void postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                   CDCTrackList& cdcTrackList);

    private:
      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       * This is done by checking the distance between the hits and the trajectory, which should not exceed the
       * maximum_distance parameter.
       *
       * As this function used the masked flag, all hits should have their masked flag set to false before calling
       * this function.
       */
      static void deleteHitsFarAwayFromTrajectory(CDCTrack& trackCandidate, double maximum_distance = 0.2);

      /// Assign new hits to the track basing on the distance from the hit to the track.
      static void assignNewHitsToTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                       double minimal_distance_to_track = 0.15);

      /// Check chi2 of the fit using the given two quantiles of the chi2 distribution.
      static bool isChi2InQuantiles(CDCTrack& track, double lower_quantile = 0.025, double upper_quantile = 0.975);

      /**
       * Calculate the quantile of chi2 with the given number of freedoms using the known chi2 distribution.
       * @param alpha quantile of chi2
       * @param n number degrees of freedom
       */
      static double calculateChi2ForQuantile(double alpha, double n);

      /// Check track quality -- currently based on number of hits only.
      static bool checkTrackQuality(const CDCTrack& track, CDCTrackList& cdcTrackList);
    };
  }
}

