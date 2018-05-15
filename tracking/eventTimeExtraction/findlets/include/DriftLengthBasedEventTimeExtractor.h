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
   * Event time extraction based on the principle of the CDC drift time calculation.
   * The measured time is given by:
   *    T_meas = T_drift + T_prop + T_0 + T_flight + T_walk
   * As all terms except T_0 are known from the track fit, it is possible
   * to use this information to extract the T0 from this.
   *
   * Will set a single candidate and the event t0, if successful.
   * Assumes the tracks to already be fitted with the current event0.
   * Will assume an eventt0 of 0, if none is set.
   */
  class DriftLengthBasedEventTimeExtractor final : public BaseEventTimeExtractor<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<RecoTrack*>;

  public:
    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;
  };
}
