/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;

    /**
     * Tools to apply post corrections to a track after finding.
     * "Removing" of hits means always to mark then as assigned. The deletion has to be done later.
     */
    class TrackQualityTools {
    public:
      /// Static class only.
      TrackQualityTools() = delete;

      /// Static class only.
      TrackQualityTools(const TrackQualityTools& copy) = delete;

      /// Static class only.
      TrackQualityTools& operator=(const TrackQualityTools&) = delete;

      /**
       * Update all hits to have a positive perpS, a taken flag and no background flag
       * Also set the trajectory to start from the first hit position and point towards the second hit.
       */
      static void normalizeHitsAndResetTrajectory(CDCTrack& track);

      /**
       * Remove all hits which can not belong to the track,
       * as the particle can not exit and enter the CDC again.
       * The radius of the CDC can be "scaled" with the given factor.
       * Works not very good.
       */
      static void removeHitsAfterCDCWall(CDCTrack& track, double outerCylindricalRFactor = 1.1);

      /** Delete a track fully of the number of hits is below minimalHits. */
      static void removeHitsIfSmall(CDCTrack& track, unsigned int minimalHits = 7);

      /**
       * Delete all hits after a large layer break.
       * Works quite well for finding large "breaks" in the track.
       * Implementation one (it is not sure which one is better in the moment.)
       */
      static void removeHitsAfterLayerBreak(CDCTrack& track, double m_maximumArcLength2DDistance = 10);

      /**
       * Delete all hits after a large layer break.
       * Works quite well for finding large "breaks" in the track.
       * Implementation two (it is not sure which one is better in the moment.)
       */
      static void removeHitsAfterLayerBreak2(CDCTrack& track);

      /// If the angle between two following hits is larger than maximalAngle, delete all hits before (!!) the second hit.
      static void removeHitsInTheBeginningIfAngleLarge(CDCTrack& track, double maximalAngle = 0.7);

      /// Remove the whole track if it only consists of one superlayer.
      static void removeHitsIfOnlyOneSuperLayer(CDCTrack& track);

      /// Remove all hits that are on the wrong side of the detector (so to say: "beyond the IP").
      static void removeHitsOnTheWrongSide(CDCTrack& track);

      /// Remove all hits that come after a large hole in the two dimensional arc length.
      static void removeArcLength2DHoles(CDCTrack& track, double m_maximumArcLength2DDistance = 10);

      /**
       * Trasan did output curlers in split two halves - this method can be used to mimic this.
       * The new track is automatically added to the list of tracks.
       */
      static void splitSecondHalfOfTrack(CDCTrack& track, std::vector<CDCTrack>& tracks);

      /// Delete hits of the first superlayer if it is a stereo one (fitting does not work very well when starting with a stereo hit).
      static void moveToNextAxialLayer(CDCTrack& track);
    };
  }
}
