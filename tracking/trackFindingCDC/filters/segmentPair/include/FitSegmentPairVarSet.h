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
    class CDCSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitSegmentPairVarNames[] = {
      "ndf",
      "chi2",
      "p_value",
      "curv",
      "z0",
      "tanl",
      "curv_var",
      "z0_var",
      "tanl_var",
    };

    /// Vehicle class to transport the variable names
    struct FitSegmentPairVarNames : public VarNames<CDCSegmentPair> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitSegmentPairVarSet : public VarSet<FitSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitSegmentPairVarNames>;

    public:
      /// Construct the varset with a switch to only do the prelimiary axial stereo fusion fit
      explicit FitSegmentPairVarSet(bool preliminaryFit = false);

      /// Generate and assign the contained variables
      bool extract(const CDCSegmentPair* ptrSegmentPair) final;

      /// Get access to the values and names of the variables - includes a prefix_ "pre" for the prelimiary fit
      std::vector<Named<Float_t*>> getNamedVariables(const std::string& prefix) final;

    private:
      /// Indicator that only the prelimiary fit should be used.
      bool m_preliminaryFit = false;
    };
  }
}
