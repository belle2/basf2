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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
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
    static char const* const facetRelationBendNames[] = {
      "delta_phi",
      "delta_phi_pull",
      "delta_phi_pull_per_r",
      "delta_curv",
      "delta_curv_pull",
      "delta_curv_pull_per_r",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facetRelation
     */
    class BendFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationBendNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetRelationBendNames[iName];
      }
    };

    /** Class that computes floating point variables from a facetRelation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BendFacetRelationVarSet : public VarSet<BendFacetRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BendFacetRelationVarNames>;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit BendFacetRelationVarSet();

      /// Generate and assign the variables from the facetRelation
      virtual bool extract(const Relation<const CDCFacet>* facetRelation) override final;
    };
  }
}
