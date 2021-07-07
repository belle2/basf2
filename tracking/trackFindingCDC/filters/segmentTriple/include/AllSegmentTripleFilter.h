/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
