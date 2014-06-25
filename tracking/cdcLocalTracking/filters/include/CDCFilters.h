/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCFILTERS_H_
#define CDCFILTERS_H_

#include <tracking/cdcLocalTracking/filters/facet/SimpleFacetFilter.h>
#include <tracking/cdcLocalTracking/filters/facet_facet/SimpleFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/facet/MCFacetFilter.h>
#include <tracking/cdcLocalTracking/filters/facet_facet/MCFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/facet/EvaluateFacetFilter.h>
#include <tracking/cdcLocalTracking/filters/facet_facet/EvaluateFacetNeighborChooser.h>



#include <tracking/cdcLocalTracking/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/filters/axial_axial/EvaluateAxialAxialSegmentPairFilter.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/filters/segment_triple_segment_triple/SimpleSegmentTripleNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/MCSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/filters/segment_triple_segment_triple/MCSegmentTripleNeighborChooser.h>




namespace Belle2 {
  namespace CDCLocalTracking {



#ifdef CDCLOCALTRACKING_USE_MC_FILTERS
#ifdef CDCLOCALTRACKING_USE_EVALUATE_FILTERS

    typedef CDCLocalTracking::EvaluateFacetFilter <
    CDCLocalTracking::SimpleFacetFilter
    > FacetFilter;

    typedef CDCLocalTracking::EvaluateFacetNeighborChooser <
    CDCLocalTracking::SimpleFacetNeighborChooser
    > FacetNeighborChooser;

#else

    typedef CDCLocalTracking::MCFacetFilter FacetFilter;
    typedef CDCLocalTracking::MCFacetNeighborChooser FacetNeighborChooser;

#endif

#else

    typedef CDCLocalTracking::SimpleFacetFilter FacetFilter;
    typedef CDCLocalTracking::SimpleFacetNeighborChooser FacetNeighborChooser;

#endif






#ifdef CDCLOCALTRACKING_USE_MC_FILTERS
#ifdef CDCLOCALTRACKING_USE_EVALUATE_FILTERS

    //No optimizing filters and choosers for segment triples yet.
    typedef CDCLocalTracking::EvaluateAxialAxialSegmentPairFilter <
    CDCLocalTracking::SimpleAxialAxialSegmentPairFilter
    > AxialAxialSegmentPairFilter;

    typedef CDCLocalTracking::MCSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;

#else

    typedef CDCLocalTracking::EvaluateAxialAxialSegmentPairFilter <
    CDCLocalTracking::SimpleAxialAxialSegmentPairFilter
    > AxialAxialSegmentPairFilter;

    //typedef CDCLocalTracking::MCAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;
    typedef CDCLocalTracking::MCSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;

#endif

#else

    typedef CDCLocalTracking::SimpleAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;
    typedef CDCLocalTracking::SimpleSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::SimpleSegmentTripleNeighborChooser SegmentTripleNeighborChooser;

#endif



  } // end namespace CDCLocalTracking
} // end namespace Belle2

#endif /* CDCFILTERS_H_ */
