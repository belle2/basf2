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

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


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
    class CDCFacetRelationTruthVarNames : public VarNames<Relation<CDCFacet>> {

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
    class CDCFacetRelationTruthVarSet : public VarSet<CDCFacetRelationTruthVarNames> {

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit CDCFacetRelationTruthVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet relation
      virtual bool extract(const Relation<CDCFacet>* ptrFacetRelation) override final;

      /// Initialize the varset before event processing
      virtual void initialize() override final;

      /// Initialize the varset before event processing
      virtual void terminate() override final;

    public:
      /// Facet filter that gives if the facet is a true facet.
      MCFacetFilter m_mcFacetFilter;

    };
  }
}
