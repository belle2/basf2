/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
