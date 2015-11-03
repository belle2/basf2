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
#include "tracking/trackFindingCDC/numerics/ESign.h"

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

    /**
     * Class performs hit assignment, reassignment and deletion of hits to the tracks
     */
    class HitProcessor {

    public:

      /** This is a static class only. We do not need a constructor here */
      HitProcessor() = delete;

      /** update given CDCRecoHit3D with given trajectory */
      static void updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit);

      /** Returns the probability of hit assignment to the track. Actually this is not a probability at all but only a number between 0 and 1.
       * 1 is best.
       * TODO: more robust criteria should be implemented */
      //static double getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track);

      /** Hits reassignment */
      static void reassignHitsFromOtherTracks(std::list<CDCTrack>& trackCandidates);

      /** Append unused hits to tracks */
      static void appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<ConformalCDCWireHit*>& AxialHitList);

      /** Tries to find and delete all "bad" hits in a track.
       * By doing so we will loose hit efficiency, but gain a low fake rate. */
      static void deleteWrongHitsOfTrack(CDCTrack& trackCandidate);

      /** Delete all hits markes as bad (MASKED) in a track. Should be called after every hit reassignment */
      static void deleteAllMarkedHits(CDCTrack& track);

      /** Unmask hits which are assigned to the track */
      static void unmaskHitsInTrack(CDCTrack& track);

      /** Tries to split back-to-back tracks into two different tracks */
      static std::vector<const CDCWireHit*> splitBack2BackTrack(CDCTrack& trackCandidate);

      /** Tries to split back-to-back tracks into two different tracks */
      static bool checkBack2BackTrack(CDCTrack& trackCandidate);

      /** Estimate sign of the track charge. */
      static ESign getChargeSign(CDCTrack& track) ;

      /** Estimate sign of the point curvature with respect to given point.*/
      static ESign getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy) ;

      /** Get azimutal angle of  the hit.*/
      static double getPhi(const CDCRecoHit3D& hit) ;

      /** Fit track using position of the wire and 1/driftLength taken as weight.*/
      static CDCTrajectory2D fitWhithoutRecoPos(CDCTrack& track);

      /** Fit track using position of the wire and 1/driftLength taken as weight.*/
      static CDCTrajectory2D fitWhithoutRecoPos(std::vector<const CDCWireHit*>& wireHits);

      /**
       * After the candidate-to-merge finding, some hits are marked as bad. This method resets them.
       * @param track to reset
       */
      static void resetHits(CDCTrack& track);

    };
  }
}
