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

    /// Findlet to exports CDCTracks as RecoTracks
    class CDCMCCloneLookUpFiller : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Write give tracks into track store array
      void apply(std::vector<CDCTrack>& cdcTracks) final;
    };
  }
}
