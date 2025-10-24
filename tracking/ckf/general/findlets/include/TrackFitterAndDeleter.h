/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/dbobjects/DAFConfiguration.h>

namespace Belle2 {
  class RecoTrack;
  /**
   * Findlet to fit tracks and remove all non fitted ones.
   */
  class TrackFitterAndDeleter : public TrackFindingCDC::Findlet<RecoTrack*> {
  public:
    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Fit the tracks and remove unfittable ones.
    void apply(std::vector<RecoTrack*>& recoTracks) override;

  private:
    /// Track Fit type to select the proper DAFParameter from DAFConfiguration; by default c_Default
    int m_trackFitType = DAFConfiguration::c_Default;

  };
}
