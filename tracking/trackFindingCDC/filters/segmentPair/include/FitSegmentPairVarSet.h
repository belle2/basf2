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
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
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

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class FitSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(fitSegmentPairVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return fitSegmentPairVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment pair.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitSegmentPairVarSet : public VarSet<FitSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitSegmentPairVarNames>;

    public:
      /// Construct the varset.
      explicit FitSegmentPairVarSet(bool preliminaryFit = false);

      /// Generate and assign the variables from the segment pair
      bool extract(const CDCSegmentPair* ptrSegmentPair) override;

      /// Get access to the values and names of the variables - includes a prefix_ "pre" for the prelimiary fit
      std::vector<Named<Float_t*>> getNamedVariables(std::string prefix) override;

    private:
      /// Indicator that only the prelimiary fit should be used.
      bool m_preliminaryFit = false;
    };
  }
}
