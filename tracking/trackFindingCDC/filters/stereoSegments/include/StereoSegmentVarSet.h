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
    class CDCSegment3D;

    /// Names of the variables to be generated
    constexpr
    static char const* const stereoSegmentVarNames[] = {
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
      //"number_of_hits_in_common",
      "sum_distance_using_2d",
      "sum_distance_using_z",
    };

    /// Vehicle class to transport the variable names
    struct StereoSegmentVarNames : public VarNames<std::pair<const CDCSegment3D*, const CDCTrack*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(stereoSegmentVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return stereoSegmentVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a stereo segment to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class StereoSegmentVarSet : public VarSet<StereoSegmentVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const std::pair<const CDCSegment3D*, const CDCTrack*>* testPair) override;
    };
  }
}
