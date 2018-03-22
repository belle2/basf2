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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

#include <memory>

#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {


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
    template <class AObject, class ARelationFilter>
    class WeightedRelationCreator : public Findlet<AObject* const, WeightedRelation<AObject>> {

    private:
      /// Type of the base class
      using Super = Findlet<AObject, WeightedRelation<AObject> >;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      WeightedRelationCreator()
      {
        this->addProcessingSignalListener(&m_relationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Constructs geometrically constrained relations between " +
        getClassMnemomicParameterDescription((AObject*)nullptr) +
        " filter by some acceptance criterion.";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_relationFilter.exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "onlyBest"),
        m_param_onlyBest,
        "Maximal number of the best relation to keep from each " +
        getClassMnemomicParameterDescription((AObject*)nullptr),
        m_param_onlyBest);
      }

      /// Main function
      void apply(const std::vector<AObject*>& inputObjects,
                 std::vector<WeightedRelation<AObject>>& weightedRelations) final {

        B2ASSERT("Expected the objects on which relations are constructed to be sorted",
        std::is_sorted(inputObjects.begin(), inputObjects.end(), LessOf<Deref>()));

        RelationFilterUtil::appendUsing(m_relationFilter, inputObjects, weightedRelations);

        if (m_param_onlyBest > 0)
        {
          const int nMaxRepetitions = m_param_onlyBest;
          int nCurrentRepetitions = 1;
          auto sameFrom =
          [&nMaxRepetitions,
          &nCurrentRepetitions](const WeightedRelation<AObject>& relation,
          const WeightedRelation<AObject>& otherRelation) -> bool {
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
