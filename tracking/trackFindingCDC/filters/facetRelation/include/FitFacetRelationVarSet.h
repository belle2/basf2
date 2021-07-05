/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitFacetRelationVarNames[] = {
      "cos_delta",
      "from_middle_cos_delta",
      "to_middle_cos_delta",


      "alpha_0",
      "chi2_0",
      "chi2_0_per_s",
      "erf_0",
      "fit_0_phi0",
      "fit_0_cos_delta",

      "chi2_1",
      "chi2_1_per_s",
      "fit_1_phi0",
      "fit_1_cos_delta",

      "chi2",
      "chi2_per_s",
      "fit_phi0",
      "fit_cos_delta",

      "phi0_from_sigma",
      "phi0_to_sigma",

      "phi0_ref_pull",
      "phi0_ref_diff",
      "phi0_ref_sigma",

      "chi2_comb",
      "phi0_comb_pull",
      "phi0_comb_diff",
      "phi0_comb_sigma",

      "chi2_kari_unit",
      "abs_curv_unit",

      "chi2_kari_l",
      "abs_curv_l",

      "chi2_kari_pseudo",
      "abs_curv_pseudo",

      "chi2_kari_proper",
      "abs_curv_proper",
    };

    /// Vehicle class to transport the variable names
    struct FitFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitFacetRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitFacetRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitFacetRelationVarSet : public VarSet<FitFacetRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCFacet>* ptrFacetRelation) final;
    };
  }
}
