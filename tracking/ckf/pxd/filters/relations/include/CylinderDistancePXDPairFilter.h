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
  class CylinderDistancePXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Is backward or forward ToPXDCKF?
    bool m_param_isBackwardCKF = true;
    /// Filter potential relations in phi between seed states (based on MeasuredStateOnPlane) and hit states
    double m_param_CylinderExtrapolationToHitPhiCut = 0.1;
    /// Filter potential relations in theta between seed states (based on MeasuredStateOnPlane) and hit states
    double m_param_CylinderExtrapolationToHitThetaCut = 0.1;
    /// Filter potential relations in phi between seed states (based on RecoTracks) and hit states
    double m_param_RecoTrackToHitPhiCut = 0.5;
    /// Filter potential relations in theta between seed states (based on RecoTracks) and hit states
    double m_param_RecoTrackToHitThetaCut = 0.5;
    /// Filter potential relations in phi between hit states
    double m_param_HitHitPhiCut = 0.2;
    /// Filter potential relations in theta between hit states
    double m_param_HitHitThetaCut = 0.2;
    /// PXD layer radii
    const std::array<double, 2> c_PXDLayerRadii = {1.42854, 2.21218};
  };
}
