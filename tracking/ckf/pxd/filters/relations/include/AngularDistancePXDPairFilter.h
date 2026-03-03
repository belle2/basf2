/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/relations/BasePXDPairFilter.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class AngularDistancePXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackingUtilities::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Filter potential relations in phi between seed states and hit states
    double m_param_PhiRecoTrackToHitCut = 0.8;
    /// Filter potential relations in phi between hit states
    double m_param_PhiHitHitCut = 1.0;
  };
}
