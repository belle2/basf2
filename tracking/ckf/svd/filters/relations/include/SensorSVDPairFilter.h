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

#include <tracking/ckf/svd/filters/relations/BaseSVDPairFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

namespace Belle2 {
  /// Base filter for CKF SVD states
  class SensorSVDPairFilter : public BaseSVDPairFilter {
  public:
    /// Return the weight based on layer
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Pre-filter relations in theta for overlay
    double m_param_ThetaRecoTrackToHitCut = 0.4;
    /// Pre-filter relations in phi between seed states and hit states
    double m_param_PhiRecoTrackToHitCut = 0.5;
    /// Pre-filter relations in phi between hit states
    double m_param_ThetaHitHitCut = M_PI / 3.;
    /// Pre-filter relations in theta between hit states
    double m_param_PhiHitHitCut = (M_PI - 2.);
  };
}
