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
   * Findlet to fit tracks and remove all non fitted ones.
   */
  class TrackFitterAndDeleter : public TrackFindingCDC::Findlet<RecoTrack*> {
  public:
    /// Fit the tracks and remove unfittable ones.
    void apply(std::vector<RecoTrack*>& recoTracks) override;
  };
}
