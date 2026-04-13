/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/CDCMCCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/filters/track/TrackQualityFilterFactory.h>
#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {

    /// Deletes fake tracks that have been rejected by a filter
    class TrackQualityEstimator : public TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&>;

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
      void apply(std::vector<TrackingUtilities::CDCTrack>& tracks) final;

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
      TrackingUtilities::ChooseableFilter<TrackQualityFilterFactory> m_trackQualityFilter;
    };
  }
}
