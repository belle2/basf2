/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/varsets/VarSet.h>
#include <tracking/trackingUtilities/varsets/VarNames.h>

#include <tracking/trackFindingCDC/filters/segmentTrack/BaseSegmentTrackFilter.h>

#include <utility>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
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
      "perp_s_of_back",
    };

    /// Vehicle class to transport the variable names
    struct SegmentTrackVarNames : public TrackingUtilities::VarNames<BaseSegmentTrackFilter::Object> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = TrackingUtilities::size(segmentTrackVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return segmentTrackVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class SegmentTrackVarSet : public TrackingUtilities::VarSet<SegmentTrackVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const BaseSegmentTrackFilter::Object* testPair) final;
    };
  }
}
