/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;

    /**
     *  Findlet to adjust the flight time of tracks relative to the flight time zero.
     *
     *  The flight time of tracks is currently only adjusted in the start trajectories of each given track
     *  and the time which is need to reach the flight time zero location with the speed of flight
     *  on the curved trajectory is set as the flight time.
     */
    class TrackFlightTimeAdjuster : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Adjust the flight time of the given tracks
      void apply(std::vector<CDCTrack>& tracks) final;
    };
  }
}
