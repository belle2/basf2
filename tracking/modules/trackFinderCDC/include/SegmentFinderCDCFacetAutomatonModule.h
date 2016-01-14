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

#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generates segments from hits using a cellular automaton build from hit triples (facets).
    class SegmentFinderCDCFacetAutomatonModule :
      public FindletModule<SegmentFinderFacetAutomaton<ChooseableClusterFilter,
      ChooseableFacetFilter,
      ChooseableFacetRelationFilter,
      ChooseableSegmentRelationFilter> > {


    private:
      /// Type of the base class
      using Super = FindletModule<SegmentFinderFacetAutomaton<ChooseableClusterFilter,
            ChooseableFacetFilter,
            ChooseableFacetRelationFilter,
            ChooseableSegmentRelationFilter> >;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonModule();

    }; // end class SegmentFinderCDCFacetAutomatonModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
