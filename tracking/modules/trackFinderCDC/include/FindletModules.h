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

#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitTopologyFiller.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentExporter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/PrimaryWireHitRelationFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>
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
    WireHitCreatorModule() : Super{{{"CDCWireHitVector"}}} {}
  };

  /**
   * Module implementation using the WireHitTopologyFiller
   */
  class WireHitTopologyFillerModule:
    public TrackFindingCDC::FindletModule<TrackFindingCDC::WireHitTopologyFiller> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::WireHitTopologyFiller>;
  public:
    WireHitTopologyFillerModule() : Super{{{"CDCWireHitVector"}}} {}
  };

  /**
   * Module implementation using the SuperClusterCreator
   */
  class SuperClusterCreatorModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SuperClusterCreator> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SuperClusterCreator>;
  public:
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
    FacetCreatorModule() : Super{{{"CDCWireHitClusterVector", "CDCFacetVector"}}} {}
  };

  /**
   * Module implementation using the SegmentCreatorFacetAutomaton
   */
  class SegmentCreatorFacetAutomatonModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorFacetAutomaton> {
  public:
    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorFacetAutomaton>;

    SegmentCreatorFacetAutomatonModule() : Super{{{"CDCFacetVector", "" /*to be set externally*/}}} {}
  };

  /**
   * Module implementation using the SegmentMerger
   */
  class SegmentMergerModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentMerger<
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
   * Module implementation using the SegmentExporter
   */
  class SegmentExporterModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentExporter> {
  };

  /**
   * Module implementation using the SegmentCreatorMCTruth
   */
  class SegmentCreatorMCTruthModule :
    public TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorMCTruth> {
  public:
    /// Type of the base class
    using Super =  TrackFindingCDC::FindletModule<TrackFindingCDC::SegmentCreatorMCTruth>;
    SegmentCreatorMCTruthModule() : Super{{{"CDCWireHitVector", "CDCRecoSegment2DVector"}}} {}
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
    SegmentFinderFacetAutomatonModule() : Super{{{"CDCWireHitVector", "CDCRecoSegment2DVector"}}} {}
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
    TrackFinderSegmentPairAutomatonModule() : Super{{{"CDCRecoSegment2DVector", "CDCTrackVector"}}} {}
  };
}
