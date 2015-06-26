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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;


    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const backgroundSegmentNames[] = {
      "segment_size",
      "form_function",
      "superlayer",
      "is_stereo",
      "number_of_stretched_layers",
      ""
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class BackgroundSegmentVarNames : public VarNames<CDCRecoSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 1;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return backgroundSegmentNames[iName];
      }
    };

    /** Class that computes floating point variables from a pair of track and segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class BackgroundSegmentVarSet : public VarSet<BackgroundSegmentVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      BackgroundSegmentVarSet(const std::string& prefix = "") : VarSet<BackgroundSegmentVarNames>(prefix) { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const CDCRecoSegment2D* segment) IF_NOT_CINT(override final);
    };
  }
}
