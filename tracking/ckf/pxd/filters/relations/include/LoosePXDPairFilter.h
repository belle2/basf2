/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/relations/BasePXDPairFilter.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class LoosePXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Pre-filter relations in theta for overlay
    float m_param_ThetaOverlayRegionPrecut = 1.0;
    /// Pre-filter relations in phi between seed states and hit states
    float m_param_PhiSeedHitPrecut = M_PI / 2.;
    /// Pre-filter relations in phi between hit states
    float m_param_PhiHitHitPrecut = M_PI / 2.;
    /// Pre-filter relations in theta between hit states
    float m_param_ThetaHitHitPrecut = M_PI / 2.;
  };
}
