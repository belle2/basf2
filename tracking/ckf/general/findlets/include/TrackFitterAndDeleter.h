/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

namespace Belle2 {
  class RecoTrack;
  /**
   * Findlet to fit tracks and remove all non fitted ones.
   */
  class TrackFitterAndDeleter : public TrackFindingCDC::Findlet<RecoTrack*> {
  public:
    /// Fit the tracks and remove unfittable ones.
    void apply(std::vector<RecoTrack*>& recoTracks) override;

    /// Set up the fitter.
    void beginRun() override;

  private:
    /// The track fitter algorithm to use. Is a pointer, as we can only construct the fitter with a proper geometry.
    std::unique_ptr<TrackFitter> m_trackFitter;
  };
}
