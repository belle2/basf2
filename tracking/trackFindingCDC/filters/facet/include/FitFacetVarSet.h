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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitFacetVarNames[] = {
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

    /// Vehicle class to transport the variable names
    struct FitFacetVarNames : public VarNames<const CDCFacet> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitFacetVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitFacetVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitFacetVarSet : public VarSet<FitFacetVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
