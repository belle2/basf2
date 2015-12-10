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

#include <tracking/trackFindingCDC/filters/segmentPair/MCSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Class that computes floating point variables from segment pair.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class  CDCSegmentPairTruthVarSet : public FilterVarSet<MCSegmentPairFilter> {
    public:
      /// Construct the varset and take an optional prefix.
      explicit CDCSegmentPairTruthVarSet(const std::string& prefix = "");
    };
  }
}
