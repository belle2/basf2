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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  class SimpleCDCToVXDExtrapolationFilter : public TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet> {
  public:
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYDistance"),
      m_param_maximumXYDistance, "", m_param_maximumXYDistance);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumDistance"),
      m_param_maximumDistance, "", m_param_maximumDistance);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2Difference"),
      m_param_maximumChi2Difference, "", m_param_maximumChi2Difference);
    }

    void initialize() override
    {
      TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet>::initialize();

      B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumXYDistance.size() == 4);
      B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumDistance.size() == 4);
      B2ASSERT("You need to provide exactly 4 maximal norms (for four layers).", m_param_maximumChi2Difference.size() == 4);
    }

    TrackFindingCDC::Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final {
      TrackFindingCDC::Weight superWeight = TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet>::operator()(currentState);

      if (std::isnan(superWeight))
      {
        return std::nan("");
      }

      const double& sameHemisphere = *(getVarSet().find("same_hemisphere"));

      if (sameHemisphere != 1)
      {
        return std::nan("");
      }

      const unsigned int& state = currentState.getState();
      const double& layer = *(getVarSet().find("layer"));

      if (state == 0)
      {
        const double& xy_distance = *(getVarSet().find("xy_distance"));
        if (xy_distance > m_param_maximumXYDistance[layer - 3]) {
          return std::nan("");
        }
      } else if (state == 1)
      {
        const double& distance = *(getVarSet().find("distance"));
        if (distance > m_param_maximumDistance[layer - 3]) {
          return std::nan("");
        }
      } else if (state == 2)
      {
        const double& lastChi2 = currentState.getLastChi2();
        const double& chi2 = currentState.getChi2();
        if (chi2 - lastChi2 > m_param_maximumChi2Difference[layer - 3]) {
          return std::nan("");
        }
      } else {
        B2FATAL("Invalid state encountered");
      }


      return 1;
    }

  private:
    std::vector<double> m_param_maximumXYDistance {10, 10, 10, 10};
    std::vector<double> m_param_maximumDistance {20, 20, 20, 20};
    std::vector<double> m_param_maximumChi2Difference {100, 100, 100, 100};
  };
}
