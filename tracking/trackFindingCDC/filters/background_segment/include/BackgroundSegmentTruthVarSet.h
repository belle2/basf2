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

#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentVarSet.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const backgroundSegmentTruthNames[] = {
      "segment_is_fake_truth",
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class BackgroundSegmentTruthVarNames : public VarNames<CDCRecoSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 2;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return backgroundSegmentTruthNames[iName];
      }

      /// Marking that the basic cluster variables should be included.
      typedef BackgroundSegmentVarSet NestedVarSet;
    };

    /** Class that computes floating point variables from a wire hit clusters.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class BackgroundSegmentTruthVarSet : public VarSet<BackgroundSegmentTruthVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      BackgroundSegmentTruthVarSet(const std::string& prefix = "") : VarSet<BackgroundSegmentTruthVarNames>(prefix) { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCRecoSegment2D* segment) IF_NOT_CINT(override final);

    };
  }
}
