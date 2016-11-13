/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <vector>
#include <string>
#include <cassert>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetRelationTruthNames[] = {
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet relation
     */
    class TruthFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationTruthNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetRelationTruthNames[iName];
      }

    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class TruthFacetRelationVarSet : public VarSet<TruthFacetRelationVarNames> {

    private:
      /// Type of the super class
      using Super = VarSet<TruthFacetRelationVarNames>;

    public:
      /// Generate and assign the variables from the facet relation
      bool extract(const Relation<const CDCFacet>* ptrFacetRelation) final;

      /// Initialize the varset before event processing
      void initialize() final;

      /// Initialize the varset before event processing
      void terminate() final;

    public:
      /// Facet filter that gives if the facet is a true facet.
      MCFacetFilter m_mcFacetFilter;
    };
  }
}
