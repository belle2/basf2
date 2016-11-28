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
    class CDCSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const trailSegmentPairVarNames[] = {
      "from_rl_asym",
      "to_rl_asym",

      "from_rl_switches",
      "to_rl_switches",
    };

    /// Vehicle class to transport the variable names
    struct TrailSegmentPairVarNames : public VarNames<CDCSegmentPair> {

      /// Number of variables to be generated
      static const size_t nVars = size(trailSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return trailSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class TrailSegmentPairVarSet : public VarSet<TrailSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
