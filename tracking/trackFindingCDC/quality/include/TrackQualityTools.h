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
      static const TrackQualityTools& getInstance()
      {
        static TrackQualityTools instance;
        return instance;
      }

    private:
      /** Use as singleton */
      TrackQualityTools()
      {}

      /** Do not copy */
      TrackQualityTools(const TrackQualityTools&) = delete;

    public:

      /**
       * Update all hits to have a positive perpS, a taken flag and no background flag
       * Also set the trajectory to start from the first hit position and point towards the second hit.
       */
      void normalizeHitsAndResetTrajectory(CDCTrack& track) const;

      void removeHitsAfterCDCWall(CDCTrack& track) const;

      void removeHitsIfSmall(CDCTrack& track) const;

      void removeHitsAfterLayerBreak(CDCTrack& track) const;

      void removeHitsAfterLayerBreak2(CDCTrack& track) const;

      void removeHitsInTheBeginningIfAngleLarge(CDCTrack& track) const;

      void removeHitsIfOnlyOneSuperLayer(CDCTrack& track) const;

      void removeHitsOnTheWrongSide(CDCTrack& track) const;

      void removeArcLength2DHoles(CDCTrack& track) const;

      void splitSecondHalfOfTrack(CDCTrack& track, std::vector<CDCTrack>& tracks) const;

      void moveToNextAxialLayer(CDCTrack& track) const;

      void unmaskHitsInTrack(CDCTrack& track) const;

      void normalizeTrack(CDCTrack& track) const;

    private:
      const double m_outerCylindricalRFactor = 1.1;
      const unsigned int m_minimalHits = 7;
      const double m_maximalAngle = 0.7;
      const double m_maximumArcLength2DDistance = 10;
    };
  }
}
