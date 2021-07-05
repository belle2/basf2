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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Findlet for normalizing the track (trajectory) into common requirements (e.g. let it start at the first hit etc.)
    class TrackNormalizer : public Findlet<CDCTrack&> {

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
