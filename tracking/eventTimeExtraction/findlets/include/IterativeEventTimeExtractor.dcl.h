/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.dcl.h>
#include <framework/dataobjects/EventT0.h>

#include <vector>

namespace Belle2 {
  class RecoTrack;

  /**
   * Generic findlet applying a certain time extractor multiple times.
   * Will abort if the extraction was not successful (can be controlled by a parameter).
   */
  template <class AFindlet>
  class IterativeEventTimeExtractor : public BaseEventTimeExtractor<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<RecoTrack*>;

  public:
    /// Add the subfindlet as listener
    IterativeEventTimeExtractor();

    /// Expose the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>& recoTracks) override final;

  private:
    /// Parameter: how many iterations should be done?
    unsigned int m_param_iterations = 2;
    /// Parameter: abort if a single iteration is not working
    bool m_param_abortOnUnsuccessfulStep = true;
    /// Parameter: use the last event t0 instead of the best one
    bool m_param_useLastEventT0 = true;
    /// The subfindlet
    AFindlet m_findlet;
    /// Pool for the event t0s with chi2
    std::vector<std::pair<EventT0::EventT0Component, double>> m_eventT0WithQuality;
  };

}
