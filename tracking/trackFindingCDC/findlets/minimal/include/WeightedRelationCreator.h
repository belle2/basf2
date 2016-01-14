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
                            const std::string& prefix = "") override final
      {
        m_relationFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main function
      virtual void apply(typename Super::template ToVector<AItem>& inputObjects,
                         std::vector<WeightedRelation<AItem> >& weightedRelations) override final
      {
        B2ASSERT("Expected the objects on which relations are constructed to be sorted",
                 std::is_sorted(inputObjects.begin(), inputObjects.end()));
        WeightedNeighborhood<AItem>::appendUsing(m_relationFilter, inputObjects, weightedRelations);
      }

    private:
      /// Reference to the relation filter
      ARelationFilter m_relationFilter;

    }; // end class WeightedRelationCreator

  } //end namespace TrackFindingCDC
} //end namespace Belle2
