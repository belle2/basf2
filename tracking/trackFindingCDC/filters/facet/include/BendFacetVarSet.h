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

#include <vector>
#include <string>
#include <cassert>

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
      "start_d",
      "start_chi2",

      "middle_phi",
      "middle_phi_sigma",
      "middle_phi_pull",
      "middle_d",
      "middle_chi2",

      "end_phi",
      "end_phi_sigma",
      "end_phi_pull",
      "end_d",
      "end_chi2",

      "s",
      "alpha",

      "curv",
      "curv_sigma",
      "curv_pull",
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
      /// Construct the varset to be prepended to all variable names.
      explicit BendFacetVarSet();

      /// Generate and assign the variables from the facet
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
