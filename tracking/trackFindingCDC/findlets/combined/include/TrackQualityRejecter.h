/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch, Nils Braun, oliver Frost           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/CDCMCCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/filters/track/TrackQualityFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Deletes fake tracks that have been rejected by a filter
    class TrackQualityRejecter : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackQualityRejecter(const std::string& defaultFilterName = "recording");

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm
      void apply(std::vector<CDCTrack>& tracks) final;

    private:
      // Findlet to fill CDCTracks into lookup table (singleton) with clone information
      CDCMCCloneLookUpFiller m_mcCloneLookUpFiller;

      /// Parameter : Switch on to delete the clones, otherwise just mark them as background.
      bool m_param_deleteTracks = true;

      /// Reference to the filter to be used to filter
      ChooseableFilter<TrackQualityFilterFactory> m_trackQualityFilter;
    };
  }
}
