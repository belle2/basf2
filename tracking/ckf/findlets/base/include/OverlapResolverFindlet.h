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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>

namespace Belle2 {
  // TODO: Let filter determine the template classes
  template<class AFilter>
  class OverlapResolverFindlet : public TrackFindingCDC::Findlet<typename AFilter::Object> {
  public:
    using ResultPair = typename AFilter::Object;
    using Super = TrackFindingCDC::Findlet<ResultPair>;

    OverlapResolverFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_qualityFilter);

      m_overlapResolverInfos.reserve(100);
      m_temporaryResults.reserve(100);
    }

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_qualityFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter("minimalActivityState", m_param_minimalActivityState, "", m_param_minimalActivityState);
    }

    void beginEvent() final {
      Super::beginEvent();

      m_overlapResolverInfos.clear();
      m_temporaryResults.clear();
    }

    /// Main function of this findlet: traverse a tree starting from a given seed object.
    void apply(std::vector<ResultPair>& resultElements) final {
      // Create overlaps using the input elements
      // TODO: Can this be done better? I think so.
      for (unsigned int resultIndex = 0; resultIndex < resultElements.size(); resultIndex++)
      {
        const ResultPair& resultPair = resultElements[resultIndex];

        TrackFindingCDC::Weight weight = m_qualityFilter(resultPair);
        m_overlaps.clear();

        for (unsigned int loopResultIndex = 0; loopResultIndex < resultElements.size(); loopResultIndex++) {
          const ResultPair& loopResultPair = resultElements[loopResultIndex];

          if (loopResultPair.first == resultPair.first) {
            m_overlaps.push_back(loopResultIndex);
            break;
          }

          for (const auto& hit : resultPair.second) {
            if (TrackFindingCDC::is_in(hit, loopResultPair.second)) {
              m_overlaps.push_back(loopResultIndex);
              break;
            }
          }
        }
        // activity state has no meaning here.
        m_overlapResolverInfos.emplace_back(weight, resultIndex, m_overlaps, 0);
      }

      // do hopfield
      m_hopfieldNetwork.doHopfield(m_overlapResolverInfos);

      // copy results
      for (OverlapResolverNodeInfo& node : m_overlapResolverInfos)
      {
        if (node.activityState > m_param_minimalActivityState) {
          m_temporaryResults.push_back(resultElements[node.trackIndex]);
        }
      }

      resultElements.swap(m_temporaryResults);
    }

  private:
    /// Subfindlet: The quality filter
    AFilter m_qualityFilter;
    /// Sub algorithm: the hopfield algorithm class
    // TODO: export parameters
    HopfieldNetwork m_hopfieldNetwork;

    /// Parameters
    double m_param_minimalActivityState = 0.75;

    /// Object Pools
    std::vector<OverlapResolverNodeInfo> m_overlapResolverInfos;
    std::vector<ResultPair> m_temporaryResults;
    std::vector<unsigned short> m_overlaps;
  };
}
