/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/relations/BaseVTXPairFilter.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>

namespace Belle2 {
  /// Base filter for CKF VTX states
  class DistanceVTXPairFilter : public BaseVTXPairFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToVTXState*, const CKFToVTXState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Pre-filter relations in phi between seed states and hit states
    double m_param_PhiSeedHitCut  = 0.2;
    /// Pre-filter relations in theta for overlay
    double m_param_ThetaSeedHitCut = 0.5;
    /// Pre-filter relations in phi between hit states
    double m_param_PhiHitHitCut = 0.05;
    /// Pre-filter relations in theta between hit states
    double m_param_ThetaHitHitCut = 0.25;

  };
}
