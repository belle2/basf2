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

    /// Class excepting all segment triples.
    class AllSegmentTripleRelationFilter : public BaseSegmentTripleRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseSegmentTripleRelationFilter;

    public:
      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Always return the overlap penatlty accepting all relations.
       */
      Weight operator()(const CDCSegmentTriple& fromSegmentTriple,
                        const CDCSegmentTriple& toSegmentTriple) final;
    };
  }
}
