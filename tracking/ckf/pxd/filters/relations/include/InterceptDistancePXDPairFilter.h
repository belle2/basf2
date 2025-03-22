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
  class InterceptDistancePXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackingUtilities::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Calculate delta phi
    float deltaPhi(float phi1, float phi2);
    /// Calculate delta eta from theta
    float deltaEtaFromTheta(float theta1, float theta2);
    /// Convert theta to eta (pseudorapidity)
    float convertThetaToEta(float cosTheta);

    /// Threshold to apply inverse pT dependent cut [GeV]
    double m_param_PtThresholdTrackToHitCut = 0.5;
    /// Filter potential relations in phi between seed states (based on PXDIntercepts) and hit states
    double m_param_PhiInterceptToHitCut = 0.2;
    /// Filter potential relations in eta between seed states (based on PXDIntercepts) and hit states
    double m_param_EtaInterceptToHitCut = 0.2;
    /// Filter potential relations in phi between seed states (based on RecoTracks) and hit states
    double m_param_PhiRecoTrackToHitCut = 0.5;
    /// Filter potential relations in eta between seed states (based on RecoTracks) and hit states
    double m_param_EtaRecoTrackToHitCut = 0.4;
    /// Filter potential relations in phi between hit states
    double m_param_PhiHitHitCut = 0.8;
    /// Filter potential relations in eta between hit states
    double m_param_EtaHitHitCut = 0.8;
    /// Filter potential relations in phi between hit states in ladder overlap
    double m_param_PhiOverlapHitHitCut = 0.15;
    /// Filter potential relations in eta between hit states in ladder overlap
    double m_param_EtaOverlapHitHitCut = 0.15;

    /// Name of the PXDIntercepts StoreArray
    std::string m_param_PXDInterceptsName = "PXDIntercepts";
  };
}
