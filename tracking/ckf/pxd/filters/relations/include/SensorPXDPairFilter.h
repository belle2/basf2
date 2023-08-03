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
  class SensorPXDPairFilter : public BasePXDPairFilter {
  public:
    /// Return the weight based on layer
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Pre-filter relations in phi between seed states and hit states
    /// Here we compare RecoTrack -> mSoP -> getPos().Phi() with the phi value of the centre of the sensor the hit is on.
    float m_param_PhiRecoTrackToHitCut = 0.8;
    /// Pre-filter relations in phi between hit states
    /// We compare hitA.sensorCenterPhi with hitB.sensorCenterPhi
    float m_param_PhiHitHitCut = 1.0;
  };
}
