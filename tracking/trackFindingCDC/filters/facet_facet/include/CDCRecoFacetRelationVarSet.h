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

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

#include <tracking/trackFindingCDC/filters/facet/CDCRecoFacetFitlessVarSet.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/algorithms/Relation.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCRecoFacet.
    class CDCRecoFacet;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const facetRelationNames[] = {
      "from_middle_phi",
      "to_middle_phi"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet relation
     */
    class CDCRecoFacetRelationVarNames : public VarNames<Relation<CDCRecoFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return facetRelationNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<CDCRecoFacetFitlessVarSet> NestedVarSet;
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class CDCRecoFacetRelationVarSet : public VarSet<CDCRecoFacetRelationVarNames> {

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      CDCRecoFacetRelationVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet relation
      virtual bool extract(const Relation<CDCRecoFacet>* ptrFacetRelation) IF_NOT_CINT(override final);
    };
  }
}
