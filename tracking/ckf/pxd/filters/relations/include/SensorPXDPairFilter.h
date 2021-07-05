/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/relations/BasePXDPairFilter.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class SensorPXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on layer
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Pre-filter relations in phi between seed states and hit states
    float m_param_PhiRecoTrackToHitCut = 0.8;
    /// Pre-filter relations in theta between hit states
    float m_param_PhiHitHitCut = 1.0;
  };
}
