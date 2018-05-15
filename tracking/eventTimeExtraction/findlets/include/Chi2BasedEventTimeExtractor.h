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

namespace Belle2 {
  class RecoTrack;

  /**
   * Event time extraction based on the principle of arXiv:0810.2241.
   * Will set a single candidate and the event t0, if successful.
   * Assumes the tracks to already be fitted with the current event0.
   * Will assume an eventt0 of 0, if none is set.
   */
  class Chi2BasedEventTimeExtractor final : public BaseEventTimeExtractor<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<RecoTrack*>;

  public:
    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;

    /// Expose parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

  private:
    /// Hard cut on this value of extracted times in the positive as well as the negative direction.
    double m_param_maximalExtractedT0 = 100;
  };
}
