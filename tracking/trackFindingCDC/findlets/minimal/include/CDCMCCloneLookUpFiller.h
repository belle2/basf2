/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <vector>

namespace Belle2 {


  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {

    /// Findlet to exports CDCTracks as RecoTracks
    class CDCMCCloneLookUpFiller : public TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Write give tracks into track store array
      void apply(std::vector<TrackingUtilities::CDCTrack>& cdcTracks) final;
    };
  }
}
