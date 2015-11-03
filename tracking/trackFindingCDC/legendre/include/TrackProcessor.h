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

      /// Assign new hits to the track basing on the distance from the hit to the track
      static void assignNewHits(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

      /// Assign new hits to all tracks (using assignNewHits(CDCTrack&) method)
      static void assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList);

      /// Check p-value of the track
      static void checkTrackProb(CDCTrackList& cdcTrackList);

      /// Perform track postprocessing
      static void postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                   CDCTrackList& cdcTrackList);

    private:
      /// Set MASKED flag of all reco hits to false and TAKEN flag to true. Does not touch the settings of the track.
      static void unmaskHitsInTrack(CDCTrack& track);

      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       * This is done by checking the distance between the hits and the trajectory, which should not exceed the
       * maximum_distance parameter.
       *
       * As this function used the masked flag, all hits should have their masked flag set to false before calling
       * this function.
       */
      static void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate, double maximum_distance = 0.2);


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

