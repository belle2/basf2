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

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/filters/facet/CDCFacetFitlessVarSet.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/ca/Relation.h>


#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetRelationBasicNames[] = {
      "from_middle_phi",
      "to_middle_phi"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet relation
     */
    class CDCFacetRelationBasicVarNames : public VarNames<Relation<CDCFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationBasicNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetRelationBasicNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<CDCFacetFitlessVarSet> NestedVarSet;
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class CDCFacetRelationBasicVarSet : public VarSet<CDCFacetRelationBasicVarNames> {

    private:
      /// Type of the super class
      typedef VarSet<CDCFacetRelationBasicVarNames> Super;

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit CDCFacetRelationBasicVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet relation
      virtual bool extract(const Relation<CDCFacet>* ptrFacetRelation) override final;
    };
  }
}
