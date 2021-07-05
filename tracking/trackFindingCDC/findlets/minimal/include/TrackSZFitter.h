/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Findlet for fitting a list of tracks in the SZ direction and replace their trajectory 3D.
    class TrackSZFitter : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Fit the tracks.
      void apply(std::vector<CDCTrack>& tracks) final;
    };
  }
}
