/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKFINDERCDCAUTOMATONDEVFILTERS_H_
#define TRACKFINDERCDCAUTOMATONDEVFILTERS_H_

// Configuring definitions
#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonDevConfig.h>


#include <tracking/trackFindingCDC/filters/facet/SimpleFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_facet/SimpleFacetNeighborChooser.h>

#include <tracking/trackFindingCDC/filters/facet/AllFacetFilter.h>

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_facet/MCFacetNeighborChooser.h>

#include <tracking/trackFindingCDC/filters/facet/EvaluateFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_facet/EvaluateFacetNeighborChooser.h>



#include <tracking/trackFindingCDC/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_axial/EvaluateAxialAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axial_stereo/SimpleAxialStereoSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_stereo/MCAxialStereoSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_stereo/EvaluateAxialStereoSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/MCAxialStereoSegmentPairNeighborChooser.h>
#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/SimpleAxialStereoSegmentPairNeighborChooser.h>



#include <tracking/trackFindingCDC/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple_segment_triple/SimpleSegmentTripleNeighborChooser.h>

#include <tracking/trackFindingCDC/filters/segment_triple/MCSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple_segment_triple/MCSegmentTripleNeighborChooser.h>



namespace Belle2 {
  namespace CDCLocalTracking {

    // First stage filter setup

#if  !defined TRACKFINDINGCDC_USE_MC_FILTERS_FIRST_STAGE && \
  !defined TRACKFINDINGCDC_USE_EVALUATE_FILTERS_FIRST_STAGE && \
  !defined TRACKFINDINGCDC_USE_SIMPLE_FILTERS_FIRST_STAGE
#define TRACKFINDINGCDC_USE_SIMPLE_FILTERS_FIRST_STAGE
#endif

#if defined TRACKFINDINGCDC_USE_MC_FILTERS_FIRST_STAGE || \
  defined TRACKFINDINGCDC_USE_EVALUATE_FILTERS_FIRST_STAGE
#ifndef TRACKFINDINGCDC_USE_MC_INFORMATION
#error "Use of Monte Carlo Filters requested in second stage, but Monte Carlo information is not setup with TRACKFINDINGCDC_USE_MC_INFORMATION."
#endif
#endif



#ifdef TRACKFINDINGCDC_USE_SIMPLE_FILTERS_FIRST_STAGE
    typedef SimpleFacetFilter FacetFilter;
    typedef SimpleFacetNeighborChooser FacetNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_MC_FILTERS_FIRST_STAGE
    typedef MCFacetFilter FacetFilter;
    typedef MCFacetNeighborChooser FacetNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_EVALUATE_FILTERS_FIRST_STAGE
    typedef EvaluateFacetFilter<SimpleFacetFilter> FacetFilter;
    typedef EvaluateFacetNeighborChooser<SimpleFacetNeighborChooser> FacetNeighborChooser;
#endif




    // Second stage filter setup
    // Segment triple tracking
#if !defined TRACKFINDINGCDC_USE_MC_FILTERS_SECOND_STAGE && \
  !defined TRACKFINDINGCDC_USE_EVALUATE_FILTERS_SECOND_STAGE && \
  !defined TRACKFINDINGCDC_USE_SIMPLE_FILTERS_SECOND_STAGE
#define TRACKFINDINGCDC_USE_SIMPLE_FILTERS_SECOND_STAGE
#endif

#if defined TRACKFINDINGCDC_USE_MC_FILTERS_SECOND_STAGE || \
  defined TRACKFINDINGCDC_USE_EVALUATE_FILTERS_SECOND_STAGE
#ifndef TRACKFINDINGCDC_USE_MC_INFORMATION
#error "Use of Monte Carlo Filters requested in second stage, but Monte Carlo information is not setup with TRACKFINDINGCDC_USE_MC_INFORMATION."
#endif
#endif

#ifdef TRACKFINDINGCDC_USE_SIMPLE_FILTERS_SECOND_STAGE
    typedef SimpleAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;
    typedef SimpleSegmentTripleFilter SegmentTripleFilter;
    typedef SimpleSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_MC_FILTERS_SECOND_STAGE
    typedef MCAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;
    typedef MCSegmentTripleFilter SegmentTripleFilter;
    typedef MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_EVALUATE_FILTERS_SECOND_STAGE
    typedef EvaluateAxialAxialSegmentPairFilter< SimpleAxialAxialSegmentPairFilter> AxialAxialSegmentPairFilter;
    typedef MCSegmentTripleFilter SegmentTripleFilter;
    typedef MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#endif



    // Segment pair tracking
#ifdef TRACKFINDINGCDC_USE_SIMPLE_FILTERS_SECOND_STAGE
    typedef SimpleAxialStereoSegmentPairFilter AxialStereoSegmentPairFilter;
    typedef SimpleAxialStereoSegmentPairNeighborChooser AxialStereoSegmentPairNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_MC_FILTERS_SECOND_STAGE
    typedef MCAxialStereoSegmentPairFilter AxialStereoSegmentPairFilter;
    typedef MCAxialStereoSegmentPairNeighborChooser AxialStereoSegmentPairNeighborChooser;
#endif

#ifdef TRACKFINDINGCDC_USE_EVALUATE_FILTERS_SECOND_STAGE
    typedef EvaluateAxialStereoSegmentPairFilter< SimpleAxialStereoSegmentPairFilter> AxialStereoSegmentPairFilter;
    typedef MCAxialStereoSegmentPairNeighborChooser AxialStereoSegmentPairNeighborChooser;
#endif


  } // end namespace CDCLocalTracking
} // end namespace Belle2

#endif /* TRACKFINDERCDCAUTOMATONDEVFILTERS_H_ */
