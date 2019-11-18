/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// A very rough filter for all CDC states.
  class RoughCDCStateFilter : public BaseCDCStateFilter {
  public:
    /// return 1 if distance < m_maximalHitDistance, NAN otherwise
    TrackFindingCDC::Weight operator()(const BaseCDCStateFilter::Object& pair) final;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// maximal distance from track to trajectory (in XY)
    double m_maximalHitDistance = 2;
  };
}
