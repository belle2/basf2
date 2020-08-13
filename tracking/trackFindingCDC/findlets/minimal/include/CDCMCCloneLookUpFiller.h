/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
