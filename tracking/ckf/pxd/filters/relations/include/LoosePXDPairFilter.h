/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
