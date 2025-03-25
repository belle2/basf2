/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/BaseSegmentTripleRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of segment triples based on simple criterions.
    class SimpleSegmentTripleRelationFilter : public BaseSegmentTripleRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseSegmentTripleRelationFilter;

    public:
      /// Implementation currently accepts all combinations
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCSegmentTriple& fromSegmentTriple,
                                           const TrackingUtilities::CDCSegmentTriple& toSegmentTriple) final;
    };
  }
}
