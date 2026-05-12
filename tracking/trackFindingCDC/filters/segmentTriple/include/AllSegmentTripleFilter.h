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
  namespace TrackingUtilities {
    class CDCSegmentTriple;
  }
  namespace TrackFindingCDC {

    /// Filter for the construction of segment triples based on simple criteria
    class AllSegmentTripleFilter : public BaseSegmentTripleFilter {

    public:
      /// All implementation returns the size of the segment triples accepting all.
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCSegmentTriple& segmentTriple) final;
    };
  }
}
