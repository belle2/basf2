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

      static void removeHitsAfterCDCWall(CDCTrack& track, double m_outerCylindricalRFactor = 1.1);

      static void removeHitsIfSmall(CDCTrack& track, unsigned int m_minimalHits = 7);

      static void removeHitsAfterLayerBreak(CDCTrack& track, double m_maximumArcLength2DDistance = 10);

      static void removeHitsAfterLayerBreak2(CDCTrack& track);

      static void removeHitsInTheBeginningIfAngleLarge(CDCTrack& track, double m_maximalAngle = 0.7);

      static void removeHitsIfOnlyOneSuperLayer(CDCTrack& track);

      static void removeHitsOnTheWrongSide(CDCTrack& track);

      static void removeArcLength2DHoles(CDCTrack& track, double m_maximumArcLength2DDistance = 10);

      static void splitSecondHalfOfTrack(CDCTrack& track, std::vector<CDCTrack>& tracks);

      static void moveToNextAxialLayer(CDCTrack& track);

      static void normalizeTrack(CDCTrack& track);
    };
  }
}
