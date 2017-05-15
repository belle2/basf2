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
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>

namespace Belle2 {
  /**
   * Overlap check which sorts out only a non-overlapping set of states from a vector, which
   * gives (as a sum) the best quality.
   *
   * The quality is calculated using the given filter.
   * The overlaps are defined using
   *   * the seed object (if the seed object is the same, the states are overlapping)
   *   * the hit objects (if one of the hits is the same, the states are overlapping)
   *
   * The implementation is based on the Hopfield network.
   */
  template<class AFilter>
  class OverlapResolverFindlet : public TrackFindingCDC::Findlet<typename AFilter::Object> {
  public:
    /// The pair of seed and hit vector to check
    using ResultPair = typename AFilter::Object;
    /// The parent class
    using Super = TrackFindingCDC::Findlet<ResultPair>;

    /// Reserve space and add the filter as a listener.
    OverlapResolverFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_qualityFilter);

      m_overlapResolverInfos.reserve(100);
      m_temporaryResults.reserve(100);
      m_resultsWithWeight.reserve(100);
    }

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_qualityFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter("minimalActivityState", m_param_minimalActivityState, "", m_param_minimalActivityState);
      moduleParamList->addParameter("enableOverlapResolving", m_param_enableOverlapResolving,
                                    "Enable the overlap resolving.", m_param_enableOverlapResolving);
    }

    /// Main function of this findlet: find a non overlapping set of results with the best quality.
    void apply(std::vector<ResultPair>& resultElements) final {
      if (not m_param_enableOverlapResolving or resultElements.empty())
      {
        return;
      }

      TrackFindingCDC::Weight maximalWeight = std::nan("");
      TrackFindingCDC::Weight minimalWeight = std::nan("");
      m_resultsWithWeight.clear();

      for (ResultPair& resultPair : resultElements)
      {
        TrackFindingCDC::Weight weight = m_qualityFilter(resultPair);
        if (std::isnan(weight)) {
          continue;
        }

        if (std::isnan(maximalWeight) or weight > maximalWeight) {
          maximalWeight = weight;
        }
        if (std::isnan(minimalWeight) or weight < minimalWeight) {
          minimalWeight = weight;
        }

        m_resultsWithWeight.emplace_back(&resultPair, weight);
      }

      m_overlapResolverInfos.clear();

      for (unsigned int resultIndex = 0; resultIndex < m_resultsWithWeight.size(); resultIndex++)
      {
        // normalize the weight
        double weight = m_resultsWithWeight[resultIndex].getWeight();
        weight = (weight - minimalWeight) / (maximalWeight - minimalWeight);

        // activity state has no meaning here -> set to 1.
        // the overlap will be set later on.
        m_overlapResolverInfos.push_back(OverlapResolverNodeInfo(weight, resultIndex, {}, 1));
      }

      for (OverlapResolverNodeInfo& resolverInfo : m_overlapResolverInfos)
      {
        const ResultPair* resultPair = m_resultsWithWeight[resolverInfo.trackIndex];

        // Find overlaps.
        auto& overlaps = resolverInfo.overlaps;

        for (const OverlapResolverNodeInfo& loopResolverInfo : m_overlapResolverInfos) {
          // We do not allow overlap with ourselves
          if (&loopResolverInfo == &resolverInfo) {
            continue;
          }

          const ResultPair* loopResultPair = m_resultsWithWeight[loopResolverInfo.trackIndex];

          if (loopResultPair->first == resultPair->first) {
            overlaps.push_back(loopResolverInfo.trackIndex);
            continue;
          }

          for (const auto& hit : resultPair->second) {
            if (TrackFindingCDC::is_in(hit, loopResultPair->second)) {
              overlaps.push_back(loopResolverInfo.trackIndex);
              break;
            }
          }
        }
      }

      if (m_overlapResolverInfos.size() > 1)
      {
        // do hopfield
        m_hopfieldNetwork.doHopfield(m_overlapResolverInfos);
      }

      // copy results
      m_temporaryResults.clear();
      for (OverlapResolverNodeInfo& node : m_overlapResolverInfos)
      {
        if (node.activityState > m_param_minimalActivityState) {
          m_temporaryResults.push_back(*(m_resultsWithWeight[node.trackIndex]));
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
    /// Parameter: Minimal activity state above a node is seen as active.
    double m_param_minimalActivityState = 0.75;
    /// Parameter: Enable overlap
    bool m_param_enableOverlapResolving = true;

    // Object Pools
    /// Overlap resolver infos as input to the hopfield network.
    std::vector<OverlapResolverNodeInfo> m_overlapResolverInfos;
    /// temporary results vector, that will be swapped with the real results vector.
    std::vector<ResultPair> m_temporaryResults;
    /// temporary results vector with weights, out of which the overlaps will be build.
    std::vector<TrackFindingCDC::WithWeight<ResultPair*>> m_resultsWithWeight;
  };
}
