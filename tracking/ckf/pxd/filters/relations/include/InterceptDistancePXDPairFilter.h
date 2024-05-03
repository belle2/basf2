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
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Filter potential relations in phi between seed states (based on PXDIntercepts) and hit states
    double m_param_PhiInterceptToHitCut = 0.1;
    /// Filter potential relations in theta between seed states (based on PXDIntercepts) and hit states
    double m_param_ThetaInterceptToHitCut = 0.1;
    /// Filter potential relations in phi between seed states (based on RecoTracks) and hit states
    double m_param_PhiRecoTrackToHitCut = 0.5;
    /// Filter potential relations in theta between seed states (based on RecoTracks) and hit states
    double m_param_ThetaRecoTrackToHitCut = 0.5;
    /// Filter potential relations in phi between hit states
    double m_param_PhiHitHitCut = 0.2;
    /// Filter potential relations in theta between hit states
    double m_param_ThetaHitHitCut = 0.2;
    /// Name of the PXDIntercepts StoreArray
    std::string m_param_PXDInterceptsName = "PXDIntercepts";
  };
}
