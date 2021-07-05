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
    static char const* const fitlessFacetVarNames[] = {
      "superlayer_id",
      "oclock_delta",
      "abs_oclock_delta",
      "cell_extend",
      "short_arm_is_crossing",
      "long_arm_is_crossing",
      "stable_twist",
      "abs_layer_id_difference",
      "layer_id_difference",
      "crossing_id",
      "shape_id",
      "is_forward_progression",
    };

    /// Vehicle class to transport the variable names
    struct FitlessFacetVarNames : public VarNames<const CDCFacet> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitlessFacetVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitlessFacetVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitlessFacetVarSet : public VarSet<FitlessFacetVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
