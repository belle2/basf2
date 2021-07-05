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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Names of the variables to be generated
    constexpr
    static char const* const bendFacetVarNames[] = {
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

      "curv",
      "curv_sigma",
      "curv_pull",
    };

    /// Vehicle class to transport the variable names
    struct BendFacetVarNames : public VarNames<const CDCFacet> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(bendFacetVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return bendFacetVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BendFacetVarSet : public VarSet<BendFacetVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
