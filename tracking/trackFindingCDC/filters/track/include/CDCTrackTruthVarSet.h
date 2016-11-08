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
    static char const* const cdcTrackTruthNames[] = {
      "track_is_fake_truth",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class CDCTrackTruthVarNames : public VarNames<CDCTrack> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 2;

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return cdcTrackTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCTrackTruthVarSet : public VarSet<CDCTrackTruthVarNames> {

    public:
      /// Construct the peeler.
      explicit CDCTrackTruthVarSet() : VarSet<CDCTrackTruthVarNames>() { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCTrack* track) override;
    };
  }
}
