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
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/ClassMnemomics.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment triples
    template<class AItem, class ARelationFilter>
    class WeightedRelationCreator
      : public Findlet<AItem, WeightedRelation<AItem> > {

    private:
      /// Type of the base class
      using Super = Findlet<AItem, WeightedRelation<AItem> >;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      WeightedRelationCreator()
      {
        this->addProcessingSignalListener(&m_relationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs geometrically constrained relations between " +
               ClassMnemomics::getParameterDescription((AItem*)nullptr) +
               " filter by some acceptance criterion.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix) override final
      {
        m_relationFilter.exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "onlyBest"),
                                      m_param_onlyBest,
                                      "Maximal number of the best relation to keep from each " +
                                      ClassMnemomics::getParameterDescription((AItem*)nullptr),
                                      m_param_onlyBest);
      }

      /// Main function
      virtual void apply(typename Super::template ToVector<AItem>& inputObjects,
                         std::vector<WeightedRelation<AItem> >& weightedRelations) override final
      {
        B2ASSERT("Expected the objects on which relations are constructed to be sorted",
                 std::is_sorted(inputObjects.begin(), inputObjects.end()));
        WeightedNeighborhood<AItem>::appendUsing(m_relationFilter, inputObjects, weightedRelations);

        if (m_param_onlyBest > 0) {
          const int nMaxRepetitions = m_param_onlyBest;
          int nCurrentRepetitions = 1;
          auto sameFrom = [&nMaxRepetitions, &nCurrentRepetitions]
                          (const WeightedRelation<AItem>& relation,
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

          auto itLast = std::unique(weightedRelations.begin(),
                                    weightedRelations.end(),
                                    sameFrom);

          weightedRelations.erase(itLast, weightedRelations.end());
        }
      }

    private:
      /// Reference to the relation filter
      ARelationFilter m_relationFilter;

      /// Maximal number of the best relations from each item to keep
      int m_param_onlyBest = 0;

    };

  }
}
