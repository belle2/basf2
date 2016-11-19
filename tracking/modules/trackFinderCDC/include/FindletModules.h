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
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitMCMultiLoopBlocker.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundBlocker.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorSegmentHough.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/PrimaryWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

/******* Minimal Findlets **********/

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the WireHitCreator
     */
    class WireHitCreatorModule : public FindletModule<WireHitCreator> {

      /// Type of the base class
      using Super = FindletModule<WireHitCreator>;

    public:
      /// Constructor setting the default store vector names
      WireHitCreatorModule()
        : Super{{{"CDCWireHitVector"}}}
      {
      }
    };

    /**
     * Module implementation using the SuperClusterCreator
     */
    class SuperClusterCreatorModule : public FindletModule<SuperClusterCreator> {

      /// Type of the base class
      using Super = FindletModule<SuperClusterCreator>;

    public:
      /// Constructor setting the default store vector names
      SuperClusterCreatorModule()
        : Super{{{"CDCWireHitVector", "CDCWireHitSuperClusterVector"}}}
      {
      }
    };

    /**
     * Module implementation using the ClusterRefiner
     */
    class ClusterRefinerModule : public FindletModule<ClusterRefiner<>> {

      /// Type of the base class
      using Super = FindletModule<ClusterRefiner<>>;

    public:
      /// Constructor setting the default store vector names
      ClusterRefinerModule()
        : Super{{{"CDCWireHitSuperClusterVector", "CDCWireHitClusterVector"}}}
      {
      }
    };

    /**
     * Module implementation using the ClusterBackgroundDetector
     */
    class ClusterBackgroundDetectorModule
      : public FindletModule<ClusterBackgroundDetector<ChooseableClusterFilter>> {
      /// Type of the base class
      using Super = FindletModule<ClusterBackgroundDetector<ChooseableClusterFilter>>;

    public:
      /// Constructor setting the default store vector names
      ClusterBackgroundDetectorModule()
        : Super{{{"CDCWireHitClusterVector"}}}
      {
      }
    };

    /**
     * Module implementation using the ChooseableFacetFilter
     */
    class FacetCreatorModule : public FindletModule<FacetCreator<ChooseableFacetFilter>> {

      /// Type of the base class
      using Super = FindletModule<FacetCreator<ChooseableFacetFilter>>;

    public:
      /// Constructor setting the default store vector names
      FacetCreatorModule()
        : Super{{{"CDCWireHitClusterVector", "CDCFacetVector"}}}
      {
      }
    };

    /**
     * Module implementation using the SegmentCreatorFacetAutomaton
     */
    class SegmentCreatorFacetAutomatonModule : public FindletModule<SegmentCreatorFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      SegmentCreatorFacetAutomatonModule()
        : Super{{{"CDCFacetVector", "" /*to be set externally*/}}}
      {
      }
    };

    /**
     * Module implementation using the SegmentLinker
     */
    class SegmentLinkerModule
      : public FindletModule<SegmentLinker<ChooseableSegmentRelationFilter>> {
    };

    /**
     * Module implementation using the SegmentOrienter
     */
    class SegmentOrienterModule : public FindletModule<SegmentOrienter> {
    };

    /**
     * Module implementation using the SegmentFitter
     */
    class SegmentFitterModule : public FindletModule<SegmentFitter> {
    };

    /**
     * Module implementation using the SegmentCreatorMCTruth
     */
    class SegmentCreatorMCTruthModule : public FindletModule<SegmentCreatorMCTruth> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorMCTruth>;

    public:
      /// Constructor setting the default store vector names
      SegmentCreatorMCTruthModule()
        : Super{{{"CDCWireHitVector", "CDCSegment2DVector"}}}
      {
      }
    };

    /**
     * Module implementation using the AxialTrackCreatorSegmentHoughModule
     */
    class AxialTrackCreatorSegmentHoughModule
      : public FindletModule<AxialTrackCreatorSegmentHough> {

      /// Type of the base class
      using Super = FindletModule<AxialTrackCreatorSegmentHough>;

    public:
      /// Constructor setting the default store vector names
      AxialTrackCreatorSegmentHoughModule()
        : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}}
      {
      }
    };

    /**
     * Module implementation using the ChooseableAxialSegmentPairFilter
     */
    class AxialSegmentPairCreatorModule
      : public FindletModule<AxialSegmentPairCreator<SimpleAxialSegmentPairFilter>> {

      /// Type of the base class
      using Super = FindletModule<AxialSegmentPairCreator<SimpleAxialSegmentPairFilter>>;

    public:
      /// Constructor setting the default store vector names
      AxialSegmentPairCreatorModule()
        : Super{{{"CDCSegment2DVector", "CDCAxialSegmentPairVector"}}}
      {
      }
    };

    /**
     * Module implementation using the ChooseableSegmentPairFilter
     */
    class SegmentPairCreatorModule
      : public FindletModule<SegmentPairCreator<ChooseableSegmentPairFilter>> {

      /// Type of the base class
      using Super = FindletModule<SegmentPairCreator<ChooseableSegmentPairFilter>>;

    public:
      /// Constructor setting the default store vector names
      SegmentPairCreatorModule()
        : Super{{{"CDCSegment2DVector", "CDCSegmentPairVector"}}}
      {
      }
    };

    /**
     * Module implementation using the SimpleSegmentTripleFilter
     */
    class SegmentTripleCreatorModule
      : public FindletModule<SegmentTripleCreator<SimpleSegmentTripleFilter>> {

      /// Type of the base class
      using Super = FindletModule<SegmentTripleCreator<SimpleSegmentTripleFilter>>;

    public:
      /// Constructor setting the default store vector names
      SegmentTripleCreatorModule()
        : Super{{{"CDCSegment2DVector", "CDCAxialSegmentVector", "CDCSegmentTripleVector"}}}
      {
      }
    };

    /**
     * Module implementation using the TrackCreatorSegmentPairAutomaton
     */
    class TrackCreatorSegmentPairAutomatonModule
      : public FindletModule<TrackCreatorSegmentPairAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackCreatorSegmentPairAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackCreatorSegmentPairAutomatonModule()
        : Super{{{"CDCSegmentPairVector", "" /*to be set externally*/}}}
      {
      }
    };

    /**
     * Module implementation using the TrackCreatorSegmentTripleAutomaton
     */
    class TrackCreatorSegmentTripleAutomatonModule
      : public FindletModule<TrackCreatorSegmentTripleAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackCreatorSegmentTripleAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackCreatorSegmentTripleAutomatonModule()
        : Super{{{"CDCSegmentTripleVector", "" /*to be set externally*/}}}
      {
      }
    };

    /**
     * Module implementation using the TrackCreatorSingleSegments
     */
    class TrackCreatorSingleSegmentsModule : public FindletModule<TrackCreatorSingleSegments> {

      /// Type of the base class
      using Super = FindletModule<TrackCreatorSingleSegments>;

    public:
      /// Constructor setting the default store vector names
      TrackCreatorSingleSegmentsModule()
        : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}}
      {
      }
    };

    /**
     * Module implementation using the TrackLinker
     */
    class TrackLinkerModule : public FindletModule<TrackLinker<ChooseableTrackRelationFilter>> {
    };

    /**
     * Module implementation using the TrackOrienter
     */
    class TrackOrienterModule : public FindletModule<TrackOrienter> {
    };

    /**
     * Module implementation using the TrackFitter
     */
    class TrackFlightTimeAdjusterModule : public FindletModule<TrackFlightTimeAdjuster> {
    };

    /**
     * Module implementation using the TrackExporter
     */
    class TrackExporterModule : public FindletModule<TrackExporter> {

    private:
      /// Type of the base class
      using Super = FindletModule<TrackExporter>;

    public:
      /// Constructor setting the default store vector names
      TrackExporterModule()
        : Super{{{"CDCTrackVector"}}}
      {
      }
    };
  }
}

/******* Combined Findlets **********/
namespace Belle2 {
  namespace TrackFindingCDC {
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>

    /*
     * Combined Module using a combination of filters for segments
     */
    class SegmentFinderFacetAutomatonModule
      : public FindletModule<SegmentFinderFacetAutomaton<ChooseableClusterFilter,
        ChooseableFacetFilter,
        ChooseableFacetRelationFilter,
        ChooseableSegmentRelationFilter>> {

      /// Type of the base class
      using Super = FindletModule<SegmentFinderFacetAutomaton<ChooseableClusterFilter,
            ChooseableFacetFilter,
            ChooseableFacetRelationFilter,
            ChooseableSegmentRelationFilter>>;

    public:
      /// Constructor setting the default store vector names
      SegmentFinderFacetAutomatonModule()
        : Super{{{"CDCWireHitVector", "CDCSegment2DVector"}}}
      {
      }
    };

    /*
     * Combined Module using a combination of filters on segment pairs
     */
    class TrackFinderSegmentPairAutomatonModule
      : public FindletModule<TrackFinderSegmentPairAutomaton<ChooseableSegmentPairFilter,
        ChooseableSegmentPairRelationFilter,
        ChooseableTrackRelationFilter>> {

      /// Type of the base class
      using Super =
        FindletModule<TrackFinderSegmentPairAutomaton<ChooseableSegmentPairFilter,
        ChooseableSegmentPairRelationFilter,
        ChooseableTrackRelationFilter>>;

    public:
      /// Constructor setting the default store vector names
      TrackFinderSegmentPairAutomatonModule()
        : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}}
      {
      }
    };

    /*
     * Combined Module using a combination of filters on segment triples
     */
    class TrackFinderSegmentTripleAutomatonModule
      : public FindletModule<TrackFinderSegmentTripleAutomaton<SimpleAxialSegmentPairFilter,
        SimpleSegmentTripleFilter,
        SimpleSegmentTripleRelationFilter,
        ChooseableTrackRelationFilter>> {

      /// Type of the base class
      using Super =
        FindletModule<TrackFinderSegmentTripleAutomaton<SimpleAxialSegmentPairFilter,
        SimpleSegmentTripleFilter,
        SimpleSegmentTripleRelationFilter,
        ChooseableTrackRelationFilter>>;

    public:
      /// Constructor setting the default store vector names
      TrackFinderSegmentTripleAutomatonModule()
        : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}}
      {
      }
    };
  }
}
