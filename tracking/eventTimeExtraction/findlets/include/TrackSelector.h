/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

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
