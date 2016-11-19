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
  /**
   * Module implementation using the WireHitCreator
   */
  class WireHitCreatorModule:
    public TrackFindingCDC::FindletModule<TrackFindingCDC::WireHitCreator> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::WireHitCreator>;

  public:
    /// Constructor setting the default store vector names
    WireHitCreatorModule() : Super{{{"CDCWireHitVector"}}} {}
  };

  /**
   * Module implementation using the SuperClusterCreator
   */
  class SuperClusterCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SuperClusterCreator> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SuperClusterCreator>;

  public:
    /// Constructor setting the default store vector names
    SuperClusterCreatorModule() : Super{{{"CDCWireHitVector", "CDCWireHitSuperClusterVector"}}} {}
  };

  /**
   * Module implementation using the ClusterRefiner
   */
  class ClusterRefinerModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::ClusterRefiner<> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::ClusterRefiner<> >;

  public:
    /// Constructor setting the default store vector names
    ClusterRefinerModule() : Super{{{"CDCWireHitSuperClusterVector", "CDCWireHitClusterVector"}}} {}
  };

  /**
   * Module implementation using the ClusterBackgroundDetector
   */
  class ClusterBackgroundDetectorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::ClusterBackgroundDetector<
    TrackFindingCDC::ChooseableClusterFilter> > {
    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::ClusterBackgroundDetector<
                  TrackFindingCDC::ChooseableClusterFilter> >;

  public:
    /// Constructor setting the default store vector names
    ClusterBackgroundDetectorModule() : Super{{{"CDCWireHitClusterVector"}}} {}
  };

  /**
   * Module implementation using the ChooseableFacetFilter
   */
  class FacetCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::FacetCreator<
    TrackFindingCDC::ChooseableFacetFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::FacetCreator<
                  TrackFindingCDC::ChooseableFacetFilter> >;

  public:
    /// Constructor setting the default store vector names
    FacetCreatorModule() : Super{{{"CDCWireHitClusterVector", "CDCFacetVector"}}} {}
  };

  /**
   * Module implementation using the SegmentCreatorFacetAutomaton
   */
  class SegmentCreatorFacetAutomatonModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorFacetAutomaton> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorFacetAutomaton>;

  public:
    /// Constructor setting the default store vector names
    SegmentCreatorFacetAutomatonModule() : Super{{{"CDCFacetVector", "" /*to be set externally*/}}} {}
  };

  /**
   * Module implementation using the SegmentLinker
   */
  class SegmentLinkerModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentLinker<
    TrackFindingCDC::ChooseableSegmentRelationFilter> > {
  };

  /**
   * Module implementation using the SegmentOrienter
   */
  class SegmentOrienterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentOrienter> {
  };

  /**
   * Module implementation using the SegmentFitter
   */
  class SegmentFitterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentFitter> {
  };

  /**
   * Module implementation using the SegmentCreatorMCTruth
   */
  class SegmentCreatorMCTruthModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorMCTruth> {

    /// Type of the base class
    using Super =  TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorMCTruth>;

  public:
    /// Constructor setting the default store vector names
    SegmentCreatorMCTruthModule() : Super{{{"CDCWireHitVector", "CDCSegment2DVector"}}} {}
  };

  /**
   * Module implementation using the AxialTrackCreatorSegmentHoughModule
   */
  class AxialTrackCreatorSegmentHoughModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::AxialTrackCreatorSegmentHough> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::AxialTrackCreatorSegmentHough>;

  public:
    /// Constructor setting the default store vector names
    AxialTrackCreatorSegmentHoughModule() : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}} {}
  };

  /**
   * Module implementation using the ChooseableAxialSegmentPairFilter
   */
  class AxialSegmentPairCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::AxialSegmentPairCreator<
    TrackFindingCDC::SimpleAxialSegmentPairFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::AxialSegmentPairCreator<
                  TrackFindingCDC::SimpleAxialSegmentPairFilter> >;

  public:
    /// Constructor setting the default store vector names
    AxialSegmentPairCreatorModule() : Super{{{"CDCSegment2DVector", "CDCAxialSegmentPairVector"}}} {}
  };

  /**
   * Module implementation using the ChooseableSegmentPairFilter
   */
  class SegmentPairCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentPairCreator<
    TrackFindingCDC::ChooseableSegmentPairFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentPairCreator<
                  TrackFindingCDC::ChooseableSegmentPairFilter> >;

  public:
    /// Constructor setting the default store vector names
    SegmentPairCreatorModule() : Super{{{"CDCSegment2DVector", "CDCSegmentPairVector"}}} {}
  };


  /**
   * Module implementation using the SimpleSegmentTripleFilter
   */
  class SegmentTripleCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentTripleCreator<
    TrackFindingCDC::SimpleSegmentTripleFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentTripleCreator<
                  TrackFindingCDC::SimpleSegmentTripleFilter> >;

  public:
    /// Constructor setting the default store vector names
    SegmentTripleCreatorModule() : Super{{{"CDCSegment2DVector", "CDCAxialSegmentVector", "CDCSegmentTripleVector"}}} {}
  };


  /**
   * Module implementation using the TrackCreatorSegmentPairAutomaton
   */
  class TrackCreatorSegmentPairAutomatonModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSegmentPairAutomaton> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSegmentPairAutomaton>;

  public:
    /// Constructor setting the default store vector names
    TrackCreatorSegmentPairAutomatonModule() : Super{{{"CDCSegmentPairVector", "" /*to be set externally*/}}} {}
  };

  /**
   * Module implementation using the TrackCreatorSegmentTripleAutomaton
   */
  class TrackCreatorSegmentTripleAutomatonModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSegmentTripleAutomaton> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSegmentTripleAutomaton>;

  public:
    /// Constructor setting the default store vector names
    TrackCreatorSegmentTripleAutomatonModule() : Super{{{"CDCSegmentTripleVector", "" /*to be set externally*/}}} {}
  };

  /**
   * Module implementation using the TrackCreatorSingleSegments
   */
  class TrackCreatorSingleSegmentsModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSingleSegments> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackCreatorSingleSegments>;

  public:
    /// Constructor setting the default store vector names
    TrackCreatorSingleSegmentsModule() : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}} {}
  };

  /**
   * Module implementation using the TrackLinker
   */
  class TrackLinkerModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackLinker<
    TrackFindingCDC::ChooseableTrackRelationFilter> > {
  };

  /**
   * Module implementation using the TrackOrienter
   */
  class TrackOrienterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackOrienter> {
  };

  /**
   * Module implementation using the TrackFitter
   */
  class TrackFlightTimeAdjusterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackFlightTimeAdjuster> {
  };

  /**
   * Module implementation using the TrackExporter
   */
  class TrackExporterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackExporter> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackExporter>;

  public:
    /// Constructor setting the default store vector names
    TrackExporterModule() : Super{{{"CDCTrackVector"}}} {}
  };
}



/******* Combined Findlets **********/
namespace Belle2 {

#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>

  /*
   * Combined Module using a combination of filters for segments
   */
  class SegmentFinderFacetAutomatonModule:
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentFinderFacetAutomaton<
    TrackFindingCDC::ChooseableClusterFilter,
    TrackFindingCDC::ChooseableFacetFilter,
    TrackFindingCDC::ChooseableFacetRelationFilter,
    TrackFindingCDC::ChooseableSegmentRelationFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentFinderFacetAutomaton<
                  TrackFindingCDC::ChooseableClusterFilter,
                  TrackFindingCDC::ChooseableFacetFilter,
                  TrackFindingCDC::ChooseableFacetRelationFilter,
                  TrackFindingCDC::ChooseableSegmentRelationFilter> >;
  public:
    /// Constructor setting the default store vector names
    SegmentFinderFacetAutomatonModule() : Super{{{"CDCWireHitVector", "CDCSegment2DVector"}}} {}
  };

  /*
   * Combined Module using a combination of filters on segment pairs
   */
  class TrackFinderSegmentPairAutomatonModule:
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackFinderSegmentPairAutomaton<
    TrackFindingCDC::ChooseableSegmentPairFilter,
    TrackFindingCDC::ChooseableSegmentPairRelationFilter,
    TrackFindingCDC::ChooseableTrackRelationFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackFinderSegmentPairAutomaton<
                  TrackFindingCDC::ChooseableSegmentPairFilter,
                  TrackFindingCDC::ChooseableSegmentPairRelationFilter,
                  TrackFindingCDC::ChooseableTrackRelationFilter> >;
  public:
    /// Constructor setting the default store vector names
    TrackFinderSegmentPairAutomatonModule() : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}} {}
  };


  /*
   * Combined Module using a combination of filters on segment triples
   */
  class TrackFinderSegmentTripleAutomatonModule:
    public TrackFindingCDC::FindletModule<TrackFindingCDC::TrackFinderSegmentTripleAutomaton<
    TrackFindingCDC::SimpleAxialSegmentPairFilter,
    TrackFindingCDC::SimpleSegmentTripleFilter,
    TrackFindingCDC::SimpleSegmentTripleRelationFilter,
    TrackFindingCDC::ChooseableTrackRelationFilter> > {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::TrackFinderSegmentTripleAutomaton<
                  TrackFindingCDC::SimpleAxialSegmentPairFilter,
                  TrackFindingCDC::SimpleSegmentTripleFilter,
                  TrackFindingCDC::SimpleSegmentTripleRelationFilter,
                  TrackFindingCDC::ChooseableTrackRelationFilter> >;
  public:
    /// Constructor setting the default store vector names
    TrackFinderSegmentTripleAutomatonModule() : Super{{{"CDCSegment2DVector", "CDCTrackVector"}}} {}
  };
}
