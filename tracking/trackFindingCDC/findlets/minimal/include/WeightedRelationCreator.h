/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/WeightedRelation.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/ClassMnemomics.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /**
     *  Findlet that combines geometrical constrained pairs of objects to relations and
     *  selects them by the filter given as template parameter.
     *
     *  @tparam AItem            Object type on which relation should be created.
     *  @tparam ARelationFilter  Selection criterion to find the accepted relations.
     *
     *  In addition a parameter is exposed to only keep a fixed number of highest weighted relations
     *  from each segment.
     */
    template <class AItem, class ARelationFilter>
    class WeightedRelationCreator : public Findlet<AItem, WeightedRelation<AItem>> {

    private:
      /// Type of the base class
      using Super = Findlet<AItem, WeightedRelation<AItem> >;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      WeightedRelationCreator()
      {
        this->addProcessingSignalListener(&m_relationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Constructs geometrically constrained relations between " +
        ClassMnemomics::getParameterDescription((AItem*)nullptr) +
        " filter by some acceptance criterion.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_relationFilter.exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "onlyBest"),
        m_param_onlyBest,
        "Maximal number of the best relation to keep from each " +
        ClassMnemomics::getParameterDescription((AItem*)nullptr),
        m_param_onlyBest);
      }

      /// Main function
      void apply(typename Super::template ToVector<AItem>& inputObjects,
                 std::vector<WeightedRelation<AItem>>& weightedRelations) final {
        B2ASSERT("Expected the objects on which relations are constructed to be sorted",
        std::is_sorted(inputObjects.begin(), inputObjects.end()));
        WeightedNeighborhood<AItem>::appendUsing(m_relationFilter, inputObjects, weightedRelations);

        if (m_param_onlyBest > 0)
        {
          const int nMaxRepetitions = m_param_onlyBest;
          int nCurrentRepetitions = 1;
          auto sameFrom =
          [&nMaxRepetitions,
          &nCurrentRepetitions](const WeightedRelation<AItem>& relation,
          const WeightedRelation<AItem>& otherRelation) -> bool {
            if (relation.getFrom() == otherRelation.getFrom())
            {
              ++nCurrentRepetitions;
              return nCurrentRepetitions > nMaxRepetitions;
            } else {
              nCurrentRepetitions = 1;
              return false;
            }
          };

          auto itLast = std::unique(weightedRelations.begin(), weightedRelations.end(), sameFrom);
          weightedRelations.erase(itLast, weightedRelations.end());
        }
      }

    private:
      /// Relation filter used to select relations
      ARelationFilter m_relationFilter;

      /// Parameter : Maximal number of the best relations from each item to keep
      int m_param_onlyBest = 0;
    };
  }
}
