/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      Weight operator()(const CDCSegmentTriple& fromSegmentTriple,
                        const CDCSegmentTriple& toSegmentTriple) final;
    };
  }
}
