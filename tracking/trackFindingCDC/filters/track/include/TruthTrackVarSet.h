/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated.
    constexpr
    static char const* const truthTrackTruthVarNames[] = {
      "track_is_fake_truth",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class TruthTrackVarNames : public VarNames<CDCTrack> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(truthTrackTruthVarNames);

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return truthTrackTruthVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class TruthTrackVarSet : public VarSet<TruthTrackVarNames> {

    public:
      /// Generate and assign the variables from the cluster
      bool extract(const CDCTrack* track) override;
    };
  }
}
