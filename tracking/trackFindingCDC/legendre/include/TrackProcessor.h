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

#include <vector>

using namespace std;

namespace Belle2 {

  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class CDCTrackList;
    class ConformalCDCWireHit;

    /// Class with high-level track (post)processing.
    class TrackProcessor {
    public:
      /// Static use only.
      TrackProcessor() = delete;

      /// Do not copy this class.
      TrackProcessor(const TrackProcessor& copy) = delete;

      /// Do not copy this class.
      TrackProcessor& operator=(const TrackProcessor&) = delete;

      /// Create CDCTrack using CDCWireHit hits and store it in the list. Then call the postprocessing on it.
      static void addCandidateFromHitsWithPostprocessing(std::vector<const CDCWireHit*>& hits,
                                                         const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                                         CDCTrackList& cdcTrackList);

      /// Assign new hits to all tracks (using the assignNewHitsToTrack(CDCTrack&) method of the HitsProcessor).
      static void assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList);

      /// Check the p-values of the tracks. If they are below the given value, delete the track from the list.
      static void deleteTracksWithLowFitProbability(CDCTrackList& cdcTrackList, double minimal_probability_for_good_fit = 0.4);

      /// Perform all track postprocessing.
      static void postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                   CDCTrackList& cdcTrackList);

    private:
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

