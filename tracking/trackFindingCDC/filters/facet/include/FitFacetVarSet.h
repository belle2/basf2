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

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const facetFitNames[] = {
      "chi2_0",
      "chi2_0_per_s",
      "fit_0_phi0",
      "fit_0_phi0_sigma",
      "chi2_1",
      "chi2_1_per_s",
      "fit_1_phi0",
      "fit_1_phi0_sigma",
      "chi2",
      "chi2_per_s",
      "fit_phi0",
      "fit_phi0_sigma",
      "erf",
      "tanh",
      "d2",
      "start_distance",
      "middle_distance",
      "end_distance",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a facet
     */
    class FitFacetVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetFitNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetFitNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a facet.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitFacetVarSet : public VarSet<FitFacetVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitFacetVarNames>;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit FitFacetVarSet();

      /// Generate and assign the variables from the facet
      virtual bool extract(const CDCFacet* facet) override final;
    };
  }
}
