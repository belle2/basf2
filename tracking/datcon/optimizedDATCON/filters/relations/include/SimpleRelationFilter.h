/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/relations/BaseRelationFilter.h>
#include <tracking/datcon/optimizedDATCON/entities/HitDataCache.h>
#include <math.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  class SimpleRelationFilter : public BaseRelationFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const std::pair<const HitDataCache*, const HitDataCache*>& relation) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// Cut on relations in theta for overlay region on same layer but different ladder
    double m_param_SimpleThetaOverlayRegionCut = 0.3;
    /// Pre-filter relations in phi between hit states
    double m_param_SimplePhiCut = M_PI / 2.;
    /// Pre-filter relations in theta between hit states
    double m_param_SimpleThetaCut = 0.3;
  };
}
