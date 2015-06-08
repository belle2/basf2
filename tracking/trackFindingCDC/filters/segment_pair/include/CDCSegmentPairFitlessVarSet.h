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
#ifndef CDCAXIALSTEREOSEGMENTPAIRFITLESSVARSET_H
#define CDCAXIALSTEREOSEGMENTPAIRFITLESSVARSET_H

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/filters/segment/CDCRecoSegment2DVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentPairFitlessVarNames[] = {
      "start_fit_superlayer_id",
      "end_fit_superlayer_id",

      "next_superlayer_id_difference",
      "previous_superlayer_id_difference",

      "end_first_to_start_last_hit_phi_difference",

      "end_first_hit_is_outside_cdc",
      "start_last_hit_is_outside_cdc",

      "arc_length2d_gap",

      "start_fit_chi2",
      "end_fit_chi2",

      "start_fit_curvature_xy",
      "end_fit_curvature_xy",

      "startFit_totalPerpS_startSegment",
      "endFit_totalPerpS_startSegment" ,

      "startFit_totalPerpS_endSegment",
      "endFit_totalPerpS_endSegment",

      "startFit_isForwardOrBackwardTo_startSegment",
      "endFit_isForwardOrBackwardTo_startSegment",

      "startFit_isForwardOrBackwardTo_endSegment",
      "endFit_isForwardOrBackwardTo_endSegment",

      "startFit_perpSGap",
      "endFit_perpSGap",

      "startFit_perpSFrontOffset" ,
      "endFit_perpSFrontOffset",

      "startFit_perpSBackOffset",
      "endFit_perpSBackOffset",

      "startFit_dist2DToCenter_endSegment",
      "endFit_dist2DToCenter_startSegment",

      "startFit_dist2DToFront_endSegment",
      "endFit_dist2DToBack_startSegment",

      "startFit_absMom2D",
      "endFit_absMom2D",

      "momAngleAtCenter_endSegment" ,
      "momAngleAtCenter_startSegment" ,

      "axialFit_curvatureXY" ,
      "axialFit_curvatureXY_variance" ,
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class CDCSegmentPairFitlessVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentPairFitlessVarNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentPairFitlessVarNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<CDCRecoSegment2DVarSet> NestedVarSet;

      /// Unpack the object for for the nested variable set
      static
      const std::pair<const CDCRecoSegment2D*, const CDCRecoSegment2D*>
      getNested(const CDCSegmentPair* segmentPair)
      {
        return std::make_pair(segmentPair->getStartSegment(), segmentPair->getEndSegment());
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class CDCSegmentPairFitlessVarSet : public VarSet<CDCSegmentPairFitlessVarNames> {

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      CDCSegmentPairFitlessVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) IF_NOT_CINT(override final);
    };
  }
}


#endif // CDCAXIALSTEREOSEGMENTFITLESSPAIRVARSET_H
