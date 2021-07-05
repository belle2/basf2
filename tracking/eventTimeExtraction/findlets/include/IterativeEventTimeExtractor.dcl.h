/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    /// Parameter: how many iterations should be done maximally?
    unsigned int m_param_maxIterations = 10;
    /// Parameter: how many iterations should be done minimally?
    unsigned int m_param_minIterations = 1;
    /// Parameter: What is the final precision?
    double m_param_minimalDeltaT0 = 0.2;
    /// Parameter: abort if a single iteration is not working
    bool m_param_abortOnUnsuccessfulStep = true;
    /// Parameter: use the last event t0 instead of the best one
    bool m_param_useLastEventT0 = true;
    /// The subfindlet
    AFindlet m_findlet;
    /// Pool for the event t0s with chi2
    std::vector<EventT0::EventT0Component> m_eventT0WithQuality;
  };

}
