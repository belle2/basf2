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


#include <tracking/trackFindingCDC/filters/segment_triple_relation/BaseSegmentTripleRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of segment triples based on simple criterions.
    class SimpleSegmentTripleRelationFilter : public Filter<Relation<CDCSegmentTriple>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCSegmentTriple>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Implementation currently accepts all combinations
      virtual NeighborWeight operator()(const CDCSegmentTriple&,
                                        const CDCSegmentTriple& neighborTriple) override final;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

