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
    static char const* const truthSegmentPairVarNames[] = {
      "truth_tanl",
      "truth_z",
      "truth_curv",
      "truth_from_alpha",
      "truth_to_alpha",
      "truth_delta_alpha",
      "truth_track_fraction",
      "__weight__",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment pair
     */
    class TruthSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(truthSegmentPairVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return truthSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment pair.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class TruthSegmentPairVarSet : public VarSet<TruthSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TruthSegmentPairVarNames>;

    public:
      /// Construct the varset.
      explicit TruthSegmentPairVarSet();

      /// Generate and assign the variables from the segment pair
      bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
