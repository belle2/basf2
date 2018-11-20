/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/ckf/cdc/filters/pathPairs/CDCPathPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /// Select the m_maximalCandidatesInFlight paths for further processing
  class CDCCKFPathSelector : public TrackFindingCDC::Findlet<CDCCKFPath> {
  public:
    CDCCKFPathSelector()
    {
      addProcessingSignalListener(&m_filter);
    }

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalCandidatesInFlight"),
                                    m_maximalCandidatesInFlight,
                                    "Maximal candidates in flight", m_maximalCandidatesInFlight);
      m_filter.exposeParameters(moduleParamList, prefix);
    }

    /// main method of the findlet, out of all paths "newPaths" select the best N=m_maximalCandidatesInFlight
    void apply(std::vector<CDCCKFPath>& newPaths) override
    {
      const auto pathComparison = [&](const CDCCKFPath & lhs, const CDCCKFPath & rhs) {
        return m_filter({&lhs, &rhs}) > 0;
      };
      std::sort(newPaths.begin(), newPaths.end(), pathComparison);

      TrackFindingCDC::only_best_N(newPaths, m_maximalCandidatesInFlight);
    }

  private:
    /// Maximum number of paths to select
    size_t m_maximalCandidatesInFlight = 3;
    /// Filter to order paths
    TrackFindingCDC::ChooseableFilter<CDCPathPairFilterFactory> m_filter;
  };
}
