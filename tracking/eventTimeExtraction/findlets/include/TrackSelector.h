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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <limits>

namespace Belle2 {
  class RecoTrack;

  /**
   * Select the tracks for the event time extraction.
   */
  class TrackSelector final : public TrackFindingCDC::Findlet<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;

    /// Expose the parameters for this module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

  private:
    /// Minimal number of CDC hits for a track
    unsigned int m_param_minNumberCDCHits = 20;
    /// Maximal number of tracks in an event
    unsigned int m_param_maximumNumberOfTracks = 4;
    /// Minimal pt for a track
    double m_param_minimumTrackPt = 0.35;
  };
}
