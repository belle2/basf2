/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {

    /// Findlet for fitting a list of tracks in the SZ direction and replace their trajectory 3D.
    class TrackSZFitter : public TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Fit the tracks.
      void apply(std::vector<TrackingUtilities::CDCTrack>& tracks) final;
    };
  }
}
