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

#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCTrackSpacePointCombinationVarSet.h>
#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>

namespace Belle2 {
  /// Filter for the constuction of axial to axial segment pairs based on simple criterions
  class SimpleCDCToVXDExtrapolationFilter : public TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet> {
  public:
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYNorm"),
      m_param_maximumXYNorm, "", m_param_maximumXYNorm);
    }

    /// Checks if a pair of axial segments is a good combination
    TrackFindingCDC::Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final {
      TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet>::operator()(currentState);

      const Float_t* distance = getVarSet().find("xy_distance");

      return (*distance) < m_param_maximumXYNorm;
    }

  private:
    double m_param_maximumXYNorm = 2;
  };
}
