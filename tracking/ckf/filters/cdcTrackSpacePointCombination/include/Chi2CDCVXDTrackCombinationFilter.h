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

#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointAdvanceAlgorithm.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointKalmanUpdateFitter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  class Chi2CDCVXDTrackCombinationFilter : public BaseCDCVXDTrackCombinationFilter {
  public:
    Chi2CDCVXDTrackCombinationFilter()
    {
      addProcessingSignalListener(&m_advanceAlgorithm);
      addProcessingSignalListener(&m_kalmanAlgorithm);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      BaseCDCVXDTrackCombinationFilter::exposeParameters(moduleParamList, prefix);

      m_advanceAlgorithm.exposeParameters(moduleParamList, prefix);
      m_kalmanAlgorithm.exposeParameters(moduleParamList, prefix);
    }

    TrackFindingCDC::Weight operator()(const BaseCDCVXDTrackCombinationFilter::Object& currentState) final;

  private:
    SpacePointAdvanceAlgorithm m_advanceAlgorithm;
    SpacePointKalmanUpdateFitter m_kalmanAlgorithm;
  };
}
