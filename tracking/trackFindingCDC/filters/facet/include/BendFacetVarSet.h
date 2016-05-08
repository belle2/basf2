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

#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <vector>
#include <string>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetBendNames[] = {
      "start_phi",
      "start_phi_sigma",
      "start_phi_pull",

      "middle_phi",
      "middle_phi_sigma",
      "middle_phi_pull",

      "end_phi",
      "end_phi_sigma",
      "end_phi_pull"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet
     */
    class BendFacetVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetBendNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetBendNames[iName];
      }
    };

    /** Class that computes floating point variables from a facet.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BendFacetVarSet : public VarSet<BendFacetVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BendFacetVarNames>;

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit BendFacetVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet
      virtual bool extract(const CDCFacet* facet) override final;

    private:
      /// Feasible right left passage filter for the feasibility cut of the right left passage information.
      FeasibleRLFacetFilter m_feasibleRLFacetFilter;
    };
  }
}
