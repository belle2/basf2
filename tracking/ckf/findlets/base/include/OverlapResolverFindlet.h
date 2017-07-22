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
#include <tracking/trackFindingVXD/trackSetEvaluator/Scrooge.h>

#include <framework/dataobjects/EventMetaData.h>

#include <fstream>

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
    /// Helper Functor to get the Seed of a given result
    struct SeedGetter {
      /// Make it a functor
      operator TrackFindingCDC::FunctorTag();

      template<class T>
      auto operator()(const T& t) const -> decltype(t.getSeed())
      {
        return t.getSeed();
      }
    };

    /// Helper Functor to get the Number of hits of a given result
    struct NumberOfHitsGetter {
      /// Make it a functor
      operator TrackFindingCDC::FunctorTag();

      template<class T>
      auto operator()(const T& t) const -> decltype(t->getHits().size())
      {
        return t->getHits().size();
      }
    };


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

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalActivityState"),
                                    m_param_minimalActivityState,
                                    "Minimal activation state below which the node is not accepted.",
                                    m_param_minimalActivityState);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useGreedyAlgorithm"),
                                    m_param_useGreedyAlgorithm,
                                    "Use the greedy algorithm instead of the hopfield algorithm.",
                                    m_param_useGreedyAlgorithm);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "enableOverlapResolving"),
                                    m_param_enableOverlapResolving,
                                    "Enable the overlap resolving.", m_param_enableOverlapResolving);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "allowOverlapBetweenSeeds"),
                                    m_param_allowOverlapBetweenSeeds,
                                    "Allow overlaps between results with different seeds.",
                                    m_param_allowOverlapBetweenSeeds);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "overlapResolverExport"),
                                    m_param_overlapResolverExport,
                                    "Filename to export the overlap resolver info to, if given.",
                                    m_param_overlapResolverExport);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useBestNInSeed"),
                                    m_param_useBestNInSeed,
                                    "In seed mode, use only the best seeds.",
                                    m_param_useBestNInSeed);
    }

    /// Main function of this findlet: find a non overlapping set of results with the best quality.
    void apply(std::vector<ResultPair>& resultElements) final {
      if (not m_param_enableOverlapResolving or resultElements.empty())
      {
        return;
      }

      m_temporaryResults.clear();
      m_resultsWithWeight.clear();
      m_overlapResolverInfos.clear();

      // Sort results by seed, as it makes the next operations faster
      std::sort(resultElements.begin(), resultElements.end(), TrackFindingCDC::LessOf<SeedGetter>());

      if (m_param_allowOverlapBetweenSeeds)
      {
        groupbySeedsAndMaximize(resultElements, m_temporaryResults);
      } else {
        resolveOverlaps(resultElements, m_temporaryResults);
      }

      resultElements.swap(m_temporaryResults);
    }

  private:
    /// Subfindlet: The quality filter
    AFilter m_qualityFilter;
    /// Sub algorithm: the hopfield algorithm class
    HopfieldNetwork m_hopfieldNetwork;
    /// Sub algorithm: the greedy algorithm
    Scrooge m_greedyAlgorithm;

    // Parameters
    /// Parameter: Minimal activity state above a node is seen as active.
    double m_param_minimalActivityState = 0.75;
    /// Parameter: Enable overlap
    bool m_param_enableOverlapResolving = true;
    /// Parameter: Use Greedy Algorithm
    bool m_param_useGreedyAlgorithm = false;
    /// Parameter: Allow overlaps between results with the same seed.
    bool m_param_allowOverlapBetweenSeeds = false;
    /// Parameter: Filename to export the overlap resolver info to, if given
    std::string m_param_overlapResolverExport = "";
    /// Parameter: In seed mode, use only the best seeds
    unsigned long m_param_useBestNInSeed = 3;

    // Object Pools
    /// Overlap resolver infos as input to the hopfield network.
    std::vector<OverlapResolverNodeInfo> m_overlapResolverInfos;
    /// temporary results vector, that will be swapped with the real results vector.
    std::vector<ResultPair> m_temporaryResults;
    /// temporary results vector with weights, out of which the overlaps will be build.
    std::vector<TrackFindingCDC::WithWeight<ResultPair*>> m_resultsWithWeight;

    /// Resolve the overlaps in the given set of results
    void resolveOverlaps(std::vector<ResultPair>& resultElements, std::vector<ResultPair>& outputResults);

    /// Group the results by their seed and handle each group separately.
    void groupbySeedsAndMaximize(std::vector<ResultPair>& resultElements, std::vector<ResultPair>& outputResults);
  };

  template<class AFilter>
  void OverlapResolverFindlet<AFilter>::groupbySeedsAndMaximize(std::vector<ResultPair>& resultElements,
      std::vector<ResultPair>& outputResults)
  {
    // resolve overlaps in each seed separately
    const auto& groupedBySeed = TrackFindingCDC::adjacent_groupby(resultElements.begin(), resultElements.end(), SeedGetter());
    for (const auto& resultElementsWithSameSeed : groupedBySeed) {

      m_resultsWithWeight.clear();
      for (ResultPair& resultPair : resultElementsWithSameSeed) {
        TrackFindingCDC::Weight weight = m_qualityFilter(resultPair);
        if (std::isnan(weight)) {
          continue;
        }
        m_resultsWithWeight.emplace_back(&resultPair, weight);
      }

      if (not m_resultsWithWeight.empty()) {
        const unsigned int useBestNResults = std::min(m_resultsWithWeight.size(), m_param_useBestNInSeed);
        if (useBestNResults < m_resultsWithWeight.size()) {
          std::sort(m_resultsWithWeight.begin(), m_resultsWithWeight.end(), TrackFindingCDC::GreaterWeight());
        }
        const auto& lastItemToUse = std::next(m_resultsWithWeight.begin(), useBestNResults);
        const auto& longestElement = *(std::max_element(m_resultsWithWeight.begin(), lastItemToUse,
                                                        TrackFindingCDC::LessOf<NumberOfHitsGetter>()));
        outputResults.push_back(*(longestElement));
      }
    }
  }

  template<class AFilter>
  void OverlapResolverFindlet<AFilter>::resolveOverlaps(std::vector<ResultPair>& resultElements,
                                                        std::vector<ResultPair>& outputResults)
  {
    TrackFindingCDC::Weight maximalWeight = NAN;
    TrackFindingCDC::Weight minimalWeight = NAN;

    for (ResultPair& resultPair : resultElements) {
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


    for (unsigned int resultIndex = 0; resultIndex < m_resultsWithWeight.size(); resultIndex++) {
      // normalize the weight
      double weight = m_resultsWithWeight[resultIndex].getWeight();
      if (minimalWeight == maximalWeight) {
        weight = 1;
      } else {
        weight = (weight - minimalWeight) / (maximalWeight - minimalWeight);
      }

      // activity state has no meaning here -> set to the minimum.
      // the overlap will be set later on.
      m_overlapResolverInfos.push_back(OverlapResolverNodeInfo(weight, resultIndex, {}, m_param_minimalActivityState));
    }

    for (OverlapResolverNodeInfo& resolverInfo : m_overlapResolverInfos) {
      const ResultPair* resultPair = m_resultsWithWeight[resolverInfo.trackIndex];

      // Find overlaps.
      auto& overlaps = resolverInfo.overlaps;

      for (const OverlapResolverNodeInfo& loopResolverInfo : m_overlapResolverInfos) {
        // We do not allow overlap with ourselves
        if (&loopResolverInfo == &resolverInfo) {
          continue;
        }

        const ResultPair* loopResultPair = m_resultsWithWeight[loopResolverInfo.trackIndex];

        if (loopResultPair->getSeed() == resultPair->getSeed()) {
          overlaps.push_back(loopResolverInfo.trackIndex);
          continue;
        }

        for (const auto& hit : resultPair->getHits()) {
          if (TrackFindingCDC::is_in(hit, loopResultPair->getHits())) {
            overlaps.push_back(loopResolverInfo.trackIndex);
            break;
          }
        }
      }
    }

    // Export overlap resolver information
    if (not m_param_overlapResolverExport.empty()) {
      StoreObjPtr<EventMetaData> eventMetaData;
      unsigned int eventNumber = eventMetaData->getEvent();
      std::ofstream output_file(m_param_overlapResolverExport + std::to_string(eventNumber), std::ios::trunc | std::ios::out);
      for (const auto& resolverInfo : m_overlapResolverInfos) {
        output_file << resolverInfo.trackIndex << "\t"
                    << resolverInfo.qualityIndex << "\t"
                    << m_resultsWithWeight[resolverInfo.trackIndex]->getTeacherInformation() << "\t"
                    << m_resultsWithWeight[resolverInfo.trackIndex]->getSeed() << "\t"
                    << m_resultsWithWeight[resolverInfo.trackIndex]->getHits().size() << "\t";
        for (const auto& overlap : resolverInfo.overlaps) {
          output_file << overlap << ",";
        }
        output_file << std::endl;
      }
    }

    if (m_overlapResolverInfos.size() > 1) {
      if (m_param_useGreedyAlgorithm) {
        m_greedyAlgorithm.performSelection(m_overlapResolverInfos);
      } else {
        m_hopfieldNetwork.doHopfield(m_overlapResolverInfos);
      }
    }

    // copy results to output vector
    for (OverlapResolverNodeInfo& node : m_overlapResolverInfos) {
      if (node.activityState >= m_param_minimalActivityState) {
        outputResults.push_back(*(m_resultsWithWeight[node.trackIndex]));
      }
    }
  }
}
