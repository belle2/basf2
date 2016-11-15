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
    class CDCSegment2D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const truthSegmentVarNames[] = {
      "segment_is_fake_truth",
      "track_is_already_found_truth",
      "segment_is_new_track_truth",
      "truth"
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class TruthSegmentVarNames : public VarNames<CDCSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(truthSegmentVarNames);

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return truthSegmentVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class TruthSegmentVarSet : public VarSet<TruthSegmentVarNames> {

    public:
      /// Generate and assign the variables from the cluster
      bool extract(const CDCSegment2D* segment) override;
    };
  }
}
