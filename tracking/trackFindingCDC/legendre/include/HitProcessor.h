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

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCTrack;
    class QuadTreeHitWrapper;
    class CDCRecoHit3D;
    class CDCTrajectory2D;
    class CDCWireHit;

    class HitProcessor {

    public:

      /** This is a static class only. We do not need a constructor here */
      HitProcessor() = delete;

      /** Create CDCRecoHit3D */
      static const CDCRecoHit3D createRecoHit3D(CDCTrajectory2D& trackTrajectory2D, QuadTreeHitWrapper* hit);

      static const CDCRecoHit3D createRecoHit3D(CDCTrajectory2D& trackTrajectory2D, const CDCWireHit* hit);

      /** update given CDCRecoHit3D with given trajectory */
      static void updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit);

      /** Returns the probability of hit assignment to the track. Actually this is not a probability at all but only a number between 0 and 1.
       * 1 is best.
       * TODO: more robust criteria should be implemented */
      //static double getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track);

      /** Hits reassignment */
      static void reassignHitsFromOtherTracks(std::list<CDCTrack>& trackCandidates);

      /** Append unused hits to tracks */
      static void appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<QuadTreeHitWrapper*>& AxialHitList);

      /** Tries to find and delete all "bad" hits in a track.
       * By doing so we will loose hit efficiency, but gain a low fake rate. */
      static void deleteWrongHitsOfTrack(CDCTrack& trackCandidate);

      /** Delete all hits markes as bad in a track. Should be called after every hit reassignment */
      static void deleteAllMarkedHits(CDCTrack& trackCandidate);

    };
  }
}
