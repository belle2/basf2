/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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
    class CDCRecoSegment3D;
    class CDCRecoSegment2D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const stereoSegmentNames[] = {
      "track_size",
      "segment_size",
      "pt",
      "phi_track",
      "track_back_s",
      "track_front_s",
      "track_mean_s",
      "segment_back_s",
      "segment_front_s",
      "track_radius",
      "superlayer_id",
      "number_of_hits_in_same_region",
      "number_of_hits_out_of_cdc",
      "number_of_hits_on_wrong_side",
      "sum_distance_to_track",
      "number_of_hits_in_common"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a stereo hit and a track pair.
     */
    class StereoSegmentVarNames : public
      VarNames<std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>, const CDCTrack&>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 16;

      /// Get the name of the column.
      constexpr
      static char const* getName(int iName)
      {
        return stereoSegmentNames[iName];
      }
    };

    /** Class that computes floating point variables from a stereo hit and a track pair
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class StereoSegmentVarSet : public VarSet<StereoSegmentVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      explicit StereoSegmentVarSet(const std::string& prefix = "") : VarSet<StereoSegmentVarNames>(prefix) { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const std::pair<std::pair<const CDCRecoSegment2D*, const CDCRecoSegment3D>, const CDCTrack&>* testPair)
      override;

    };
  }
}
