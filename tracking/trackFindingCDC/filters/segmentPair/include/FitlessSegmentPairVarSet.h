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

#include <tracking/trackFindingCDC/filters/segment/BasicSegmentVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentPairFitlessVarNames[] = {
      "start_fit_superlayer_id",
      "end_fit_superlayer_id",

      "start_fit_superlayer_id_difference",
      "end_fit_superlayer_id_difference",

      "next_superlayer_id_difference",
      "previous_superlayer_id_difference",


      "end_first_to_start_last_hit_pos_phi_difference",
      "end_first_to_start_last_hit_mom_phi_difference",
      "end_first_to_start_last_hit_phi_difference",

      "stereo_first_hit_z",
      "stereo_last_hit_z",

      "stereo_first_hit_dist_z_forward_wall",
      "stereo_first_hit_dist_z_backward_wall",

      "stereo_last_hit_dist_z_forward_wall",
      "stereo_last_hit_dist_z_backward_wall",

      "stereo_hits_min_dist_z_forward_wall",
      "stereo_hits_max_dist_z_forward_wall",

      "stereo_hits_min_dist_z_backward_wall",
      "stereo_hits_max_dist_z_backward_wall",

      "start_arc_length_front_offset",
      "end_arc_length_front_offset",
      "start_arc_length_back_offset",
      "end_arc_length_back_offset",

      // "arc_length2d_gap",

      // "start_fit_chi2",
      // "end_fit_chi2",

      // "start_fit_curvature_xy",
      // "end_fit_curvature_xy",

      // "startFit_totalPerpS_startSegment",
      // "endFit_totalPerpS_startSegment" ,

      // "startFit_totalPerpS_endSegment",
      // "endFit_totalPerpS_endSegment",

      // "startFit_isForwardOrBackwardTo_startSegment",
      // "endFit_isForwardOrBackwardTo_startSegment",

      // "startFit_isForwardOrBackwardTo_endSegment",
      // "endFit_isForwardOrBackwardTo_endSegment",

      // "startFit_perpSGap",
      // "endFit_perpSGap",

      // "startFit_perpSFrontOffset" ,
      // "endFit_perpSFrontOffset",

      // "startFit_perpSBackOffset",
      // "endFit_perpSBackOffset",

      // "startFit_dist2DToCenter_endSegment",
      // "endFit_dist2DToCenter_startSegment",

      // "startFit_dist2DToFront_endSegment",
      // "endFit_dist2DToBack_startSegment",

      // "startFit_absMom2D",
      // "endFit_absMom2D",

      // "momAngleAtCenter_endSegment" ,
      // "momAngleAtCenter_startSegment" ,

      // "axialFit_curvatureXY" ,
      // "axialFit_curvatureXY_variance" ,
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class FitlessSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentPairFitlessVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return segmentPairFitlessVarNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<BasicSegmentVarSet> NestedVarSet;

      /// Unpack the object for for the nested variable set
      static
      const std::pair<const CDCRecoSegment2D*, const CDCRecoSegment2D*>
      getNested(const CDCSegmentPair* segmentPair)
      {
        return std::make_pair(segmentPair->getFromSegment(), segmentPair->getToSegment());
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitlessSegmentPairVarSet : public VarSet<FitlessSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitlessSegmentPairVarNames>;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit FitlessSegmentPairVarSet();

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
