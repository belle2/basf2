/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class FitlessSegmentPairFilter : public FilterOnVarSet<SkimmedHitGapSegmentPairVarSet> {

    private:
      /// Type of the base class
      using Super = FilterOnVarSet<SkimmedHitGapSegmentPairVarSet>;

    public:
      /// Checks if a pair of segments is a good combination
      Weight operator()(const CDCSegmentPair& segmentPair) final;
    };
  }
}
