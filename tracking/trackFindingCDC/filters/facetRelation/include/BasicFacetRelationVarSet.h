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

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetRelationBasicVarNames[] = {
      "from_middle_phi",
      "to_middle_phi"
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a facet relation
     */
    class BasicFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationBasicVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetRelationBasicVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BasicFacetRelationVarSet : public VarSet<BasicFacetRelationVarNames> {

    public:
      /// Generate and assign the variables from the facet relation
      bool extract(const Relation<const CDCFacet>* ptrFacetRelation) final;
    };
  }
}
