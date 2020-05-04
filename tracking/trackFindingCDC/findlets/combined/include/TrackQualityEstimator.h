/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
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
    class TrackQualityEstimator : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      explicit TrackQualityEstimator(const std::string& defaultFilterName = "mva");

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Receive and dispatch signal before the start of the event processing
      void initialize() override;

      /// Main algorithm
      void apply(std::vector<CDCTrack>& tracks) final;

    private:
      /// Findlet to fill CDCTracks into lookup table (singleton) with clone information
      CDCMCCloneLookUpFiller m_mcCloneLookUpFiller;

      /// Store output of needsTruthInformation from filter in member variable
      bool m_needsTruthInformation = false;

      /// Delete tracks below threshold instead of just assigning quality indicator
      bool m_param_deleteTracks = false;

      /// Reset taken flag for deleted tracks so that hits can be used by subsequent TFs
      bool m_param_resetTakenFlag = false;

      /// Reference to the filter to be used to filter
      ChooseableFilter<TrackQualityFilterFactory> m_trackQualityFilter;
    };
  }
}
