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
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>

namespace Belle2 {
  template<class AVarSet>
  class OverlapTeacher : public TrackFindingCDC::Findlet<typename AVarSet::Object> {
  public:
    /// The pair of seed and hit vector to check
    using ResultPair = typename AVarSet::Object;
    /// The parent class
    using Super = TrackFindingCDC::Findlet<ResultPair>;

    OverlapTeacher() : m_varSetFilter("truth_number_of_correct_hits")
    {
      this->addProcessingSignalListener(&m_varSetFilter);
    }

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_varSetFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter("enableOverlapTeacher", m_param_enableOverlapTeacher,
                                    "Enable adding truth information from the teacher before the overlap resolving starts.",
                                    m_param_enableOverlapTeacher);
    }

    /// Main function of this findlet: add truth information from the passed var set
    void apply(std::vector<ResultPair>& resultElements) final {
      if (not m_param_enableOverlapTeacher)
      {
        return;
      }

      // Use the filter to add the number of correct hits as a weight
      for (ResultPair& result : resultElements)
      {
        const auto& numberOfCorrectHits = m_varSetFilter(result);

        const auto& hits = result.getHits();
        // All results, which more than 1 wrong hit are discarded
        // TODO: make this more general!
        if (2 * hits.size() - numberOfCorrectHits > 1) {
          result.setWeight(-1);
        } else {
          result.setWeight(numberOfCorrectHits);
        }
      }

      // Group the result elements by their seed, sort the by their number of correct hits and set the weight to
      // 1 for all results with the maximal number of correct hits and
      // 0 for al others
      const auto& extractSeed = [](const ResultPair & result)
      {
        return result.getSeed();
      };

      const auto& bySeedSorter = [&extractSeed](const ResultPair & lhs, const ResultPair & rhs)
      {
        return lhs.getSeed() < rhs.getSeed();
      };

      std::sort(resultElements.begin(), resultElements.end(), bySeedSorter);
      const auto& groupedBySeeds = TrackFindingCDC::adjacent_groupby(resultElements.begin(), resultElements.end(), extractSeed);

      const auto& byWeightSorter = [&extractSeed](const ResultPair & lhs, const ResultPair & rhs)
      {
        return lhs.getWeight() < rhs.getWeight();
      };

      for (const auto& resultsWithSameSeed : groupedBySeeds)
      {
        const auto& maximalWeightElement = std::max_element(resultsWithSameSeed.begin(), resultsWithSameSeed.end(), byWeightSorter);
        const auto& maximalWeight = maximalWeightElement->getWeight();

        for (auto& result : resultsWithSameSeed) {
          if (result.getWeight() == maximalWeight and maximalWeight != -1) {
            result.setWeight(1);
          } else {
            result.setWeight(0);
          }
        }
      }
    }

  private:
    /// Subfindlet: A filter compiled from the given var set
    TrackFindingCDC::NamedChoosableVarSetFilter<AVarSet> m_varSetFilter;

    // Parameters
    /// Parameter: Enable adding truth information from the teacher before the overlap resolving starts.
    double m_param_enableOverlapTeacher = false;
  };
}
