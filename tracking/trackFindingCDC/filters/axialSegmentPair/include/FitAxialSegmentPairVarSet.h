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
    class CDCAxialSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitAxialSegmentPairVarNames[] = {
      "is_fitted",
      "curv",
      "curv_var",
      "chi2",
      "chi2_no_rl",
      "chi2_per_ndf",
      "chi2_no_rl_per_ndf",
      "ndf",
      "p_value",
    };

    /// Vehicle class to transport the variable names
    struct FitAxialSegmentPairVarNames : public VarNames<CDCAxialSegmentPair> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitAxialSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitAxialSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitAxialSegmentPairVarSet : public VarSet<FitAxialSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCAxialSegmentPair* ptrAxialSegmentPair) final;
    };
  }
}
