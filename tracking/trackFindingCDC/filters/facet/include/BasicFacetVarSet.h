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
    static char const* const basicFacetVarNames[] = {
      "superlayer_id",

      // "start_layer_id",
      "start_drift_length",
      "start_drift_length_sigma",

      // "middle_layer_id",
      "middle_drift_length",
      "middle_drift_length_sigma",

      // "end_layer_id",
      "end_drift_length",
      "end_drift_length_sigma",

      "oclock_delta",
      "twist",
      "cell_extend",
      "n_crossing",

      "alpha",
    };

    /// Vehicle class to transport the variable names
    struct BasicFacetVarNames : public VarNames<const CDCFacet> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(basicFacetVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicFacetVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicFacetVarSet : public VarSet<BasicFacetVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
