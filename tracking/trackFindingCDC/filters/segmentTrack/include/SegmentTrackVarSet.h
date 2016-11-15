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


#include <vector>
#include <string>
#include <cassert>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCWireHitCluster.
    class CDCSegment2D;
    class CDCTrack;


    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentTrackVarNames[] = {
      "is_stereo",
      "segment_size",
      "track_size",
      "maxmimum_trajectory_distance_front",
      "maxmimum_trajectory_distance_back",
      "maxmimum_hit_distance_front",
      "maxmimum_hit_distance_back",
      "out_of_CDC",
      "hits_in_same_region",
      "fit_full",
      "fit_neigh",
      "pt_of_track",
      "track_is_curler",
      "superlayer_already_full",
      "z_distance",
      "theta_distance",
      "number_of_hits_in_common",
      "mean_hit_z_distance",
      "max_hit_z_distance",
      "stereo_quad_tree_distance",
      "segment_super_layer",
      "phi_between_track_and_segment",
      "perp_s_of_front",
      "perp_s_of_back"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrackVarNames : public VarNames<std::pair<const CDCSegment2D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentTrackVarNames);

      /// Get the name of the corresponding column.
      constexpr
      static char const* getName(int iName)
      {
        return segmentTrackVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a pair of track and segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrackVarSet : public VarSet<SegmentTrackVarNames> {

    public:
      /// Generate and assign the variables from the pair
      bool extract(const std::pair<const CDCSegment2D*, const CDCTrack*>* testPair) final;
    };
  }
}
