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

      TrackFindingCDC::Weight maximalWeight = 0;
      unsigned int resultIndex = 0;

      for (const ResultPair& resultPair : resultElements)
      {
        TrackFindingCDC::Weight weight = m_qualityFilter(resultPair);
        if (weight > maximalWeight) {
          maximalWeight = weight;
        }

        // activity state has no meaning here -> set to 0.
        // the overlap will be set later on.
        // TODO: why is emplace_back not working here?
        m_overlapResolverInfos.emplace_back(OverlapResolverNodeInfo(weight, resultIndex, {}, 0));
        resultIndex++;
      }

      for (resultIndex = 0; resultIndex < resultElements.size(); resultIndex++)
      {
        auto& resolverInfo = m_overlapResolverInfos[resultIndex];
        ResultPair& resultPair = resultElements[resultIndex];

        // Normalize the weight
        resolverInfo.qualityIndex /= maximalWeight;

        // Find overlaps. TODO: Can this be done better? Maybe with a list of pointers?
        auto& overlaps = resolverInfo.overlaps;

        for (unsigned int loopResultIndex = 0; loopResultIndex < resultElements.size(); loopResultIndex++) {
          const ResultPair& loopResultPair = resultElements[loopResultIndex];

          if (loopResultPair.first == resultPair.first) {
            overlaps.push_back(loopResultIndex);
            break;
          }

          for (const auto& hit : resultPair.second) {
            if (TrackFindingCDC::is_in(hit, loopResultPair.second)) {
              overlaps.push_back(loopResultIndex);
              break;
            }
          }
        }
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

    // Parameters
    /// Minimal activity state above a node is seen as active.
    double m_param_minimalActivityState = 0.75;

    // Object Pools
    /// Overlap resolver infos as input to the hopfield network.
    std::vector<OverlapResolverNodeInfo> m_overlapResolverInfos;
    /// temporary results vector, that will be swapped with the real results vector.
    std::vector<ResultPair> m_temporaryResults;
  };
}
