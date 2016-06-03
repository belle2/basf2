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

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>

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
    static char const* const facetRelationFitNames[] = {
      "cos_delta",
      "from_middle_cos_delta",
      "to_middle_cos_delta",

      "chi2_0",
      "erf_0",
      "fit_0_phi0",
      "fit_0_cos_delta",

      "chi2_1",
      "fit_1_phi0",
      "fit_1_cos_delta",

      "chi2",
      "fit_phi0",
      "fit_cos_delta",

      "chi2_kari_unit",
      "abs_curv_unit",

      "chi2_kari_l",
      "abs_curv_l",

      "chi2_kari_pseudo",
      "abs_curv_pseudo",

      "chi2_kari_proper",
      "abs_curv_proper",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet relation
     */
    class FitFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetRelationFitNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return facetRelationFitNames[iName];
      }

      /// Marking that the fit facet variables should be included.
      typedef PairVarSet<FitlessFacetVarSet> NestedVarSet;
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitFacetRelationVarSet : public VarSet<FitFacetRelationVarNames> {

    private:
      /// Type of the super class
      typedef VarSet<FitFacetRelationVarNames> Super;

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit FitFacetRelationVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet relation
      virtual bool extract(const Relation<const CDCFacet>* ptrFacetRelation) override final;
    };
  }
}
