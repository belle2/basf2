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

#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentTriple;

    /// Filter for the constuction of segment triples based on simple criterions
    class AllSegmentTripleFilter : public BaseSegmentTripleFilter {

    public:
      /// All implementation returns the size of the segment triples accepting all.
      Weight operator()(const CDCSegmentTriple& segmentTriple) final;
    };
  }
}
