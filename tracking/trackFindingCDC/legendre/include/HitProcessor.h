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
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCTrack;
    class ConformalCDCWireHit;
    class CDCRecoHit3D;
    class CDCTrajectory2D;
    class CDCWireHit;
    class Vector2D;
    class CDCTrackList;

    /**
     * Class performs hit assignment, reassignment and deletion of hits to the tracks
     */
    class HitProcessor {

    public:
      /// Static class only.
      HitProcessor() = delete;

      /// Static class only.
      HitProcessor(const HitProcessor& copy) = delete;

      /// Static class only.
      HitProcessor& operator=(const HitProcessor&) = delete;

      /** update given CDCRecoHit3D with given trajectory */
      static void updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit);

      /** Hits reassignment */
      static void reassignHitsFromOtherTracks(CDCTrackList& cdcTrackList);

      /** Append unused hits to tracks */
      static void appendUnusedHits(std::vector<CDCTrack>& tracks, const std::vector<ConformalCDCWireHit*>& conformalCDCWireHitList);

      /** Delete all hits markes as bad (MASKED) in a track. Should be called after every hit reassignment */
      static void deleteAllMarkedHits(CDCTrack& track);

      /** Tries to split back-to-back tracks into two different tracks */
      static std::vector<const CDCWireHit*> splitBack2BackTrack(CDCTrack& trackCandidate);

      /** Tries to split back-to-back tracks into two different tracks */
      static bool checkBack2BackTrack(CDCTrack& trackCandidate);

      /** Estimate sign of the track charge. */
      static ESign getChargeSign(CDCTrack& track) ;

      /** Estimate sign of the point curvature with respect to given point.*/
      static ESign getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy) ;

      /** Fit track using position of the wire and 1/driftLength taken as weight.*/
      static CDCTrajectory2D fitWhithoutRecoPos(CDCTrack& track);

      /** Fit track using position of the wire and 1/driftLength taken as weight.*/
      static CDCTrajectory2D fitWhithoutRecoPos(std::vector<const CDCWireHit*>& wireHits);

      /// Reset all masked hits.
      static void resetMaskedHits(CDCTrackList& cdcTrackList, std::vector<ConformalCDCWireHit>& conformalCDCWireHitList);

      /// Unset the MASKED flag and set the TAKEN flag of all hits but do not touch the track flags.
      static void unmaskHitsInTrack(CDCTrack& track);

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
    };
  }
}
