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

#include <vector>
#include <list>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCRecoHit3D;
    class CDCTrajectory2D;
    class CDCWireHit;
    class Vector2D;

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
      static void updateRecoHit3D(CDCTrajectory2D& trajectory2D, CDCRecoHit3D& hit);

      /** Hits reassignment */
      static void reassignHitsFromOtherTracks(std::list<CDCTrack>& trackList);

      /** Append unused hits to tracks */
      static void appendUnusedHits(std::list<CDCTrack>& tracks, const std::vector<const CDCWireHit*>& axialWireHits);

      /** Delete all hits marked as bad (MASKED) in a track. Should be called after every hit reassignment. */
      static void deleteAllMarkedHits(CDCTrack& track);

      /** Delete all hits marked as bad (MASKED) in a list. Should be called after every hit reassignment. */
      static void deleteAllMarkedHits(std::vector<const CDCWireHit*>& wireHits);

      /** Tries to split back-to-back tracks into two different tracks */
      static std::vector<const CDCWireHit*> splitBack2BackTrack(CDCTrack& track);

      /** Checks whether the track has hits on both arms as seen from the origin */
      static bool isBack2BackTrack(CDCTrack& track);

      /**
       *  Calculate whether the majority of hits is to the right or to the left
       *  relative to the line from origin to given center.
       *
       *  @retval ESign::c_Right most hits are on the right
       *  @retval ESign::c_Left most hits are on the left
       *  @retval ESign::c_Zero no majority
       *  @retval ESign::c_Invalid given center has a nan value
       */
      static ESign getMajorArmSign(const CDCTrack& track, const Vector2D& center);

      /// Reset all masked hits.
      static void resetMaskedHits(std::list<CDCTrack>& trackList, std::vector<const CDCWireHit*>& wireHits);

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
      static void deleteHitsFarAwayFromTrajectory(CDCTrack& track, double maximumDistance = 0.2);

      /// Assign new hits to the track basing on the distance from the hit to the track.
      static void assignNewHitsToTrack(CDCTrack& track,
                                       const std::vector<const CDCWireHit*>& allAxialWireHits,
                                       double minimalDistance = 0.15);

      /// Mask hits after breaks in the superlayer as masked and delete them.
      static void maskHitsWithPoorQuality(CDCTrack& track);

    private:
      /**
       *  Calculate whether the hits is to the right or to the left
       *  relative to the line from origin to the given center.
       *
       *  @retval ESign::c_Right   hit is on the right
       *  @retval ESign::c_Left    hit is on the left
       *  @retval ESign::c_Zero    rare boarderline case
       *  @retval ESign::c_Invalid given hit has a nan value
       */
      static ESign getArmSign(const CDCRecoHit3D& hit, const Vector2D& center);

      /** Helper function to extract the first filled entry in the array of super layers ( = the start superlayer of the track). */
      static int startingSLayer(const std::vector<double>& startingArmSLayers);

      /** Helper function to extract the last filled entry in the array of super layers ( = the final superlayer of the track). */
      static int endingSLayer(const std::vector<double>& startingArmSLayers);

      /** Return true of both the starting arm and the ending arm array has one non-zero entry. */
      static bool isTwoSided(const std::vector<double>& startingArmSLayers,
                             const std::vector<double>& endingArmSLayers);

      /** Fill the two arrays emptyStartingSLayers (for the outgoing arm) and emptyEndingSLayers (for the ingoing arm) with the superlayers between start end end of the track that are empty.
       * Return true if we have found such a case. */
      static bool hasHoles(const std::vector<double>& startingArmSLayers,
                           const std::vector<double>& endingArmSLayers,
                           std::vector<int>& emptyStartingSLayers,
                           std::vector<int>& emptyEndingSLayers);
    };
  }
}
