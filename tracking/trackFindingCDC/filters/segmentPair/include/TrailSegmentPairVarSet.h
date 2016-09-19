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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const trailSegmentPairVarNames[] = {
      "from_rl_asym",
      "to_rl_asym",

      "from_rl_switches",
      "to_rl_switches",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class TrailSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(trailSegmentPairVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return trailSegmentPairVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment pair.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class TrailSegmentPairVarSet : public VarSet<TrailSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TrailSegmentPairVarNames>;

    public:
      /// Construct the varset.
      explicit TrailSegmentPairVarSet();

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
