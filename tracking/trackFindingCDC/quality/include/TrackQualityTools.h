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

    };
  }
}
