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
  class SimpleCDCToVXDExtrapolationFilter : public TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet> {
  public:
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYNorm"),
      m_param_maximumXYNorm, "", m_param_maximumXYNorm);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumZNorm"),
      m_param_maximumZNorm, "", m_param_maximumZNorm);
    }

    TrackFindingCDC::Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final {
      TrackFindingCDC::Weight superWeight = TrackFindingCDC::FilterOnVarSet<CDCTrackSpacePointCombinationVarSet>::operator()(currentState);

      if (std::isnan(superWeight))
      {
        return std::nan("");
      }

      const Float_t* xyDistance = getVarSet().find("xy_distance");
      const Float_t* zDistance = getVarSet().find("z_distance");
      const Float_t* sameHemisphere = getVarSet().find("same_hemisphere");

      if (*xyDistance > m_param_maximumXYNorm or fabs(*zDistance) > m_param_maximumZNorm or * sameHemisphere != 1)
      {
        return std::nan("");
      }

      return 1;
    }

  private:
    double m_param_maximumXYNorm = 0.5;
    double m_param_maximumZNorm = 5;
  };
}
