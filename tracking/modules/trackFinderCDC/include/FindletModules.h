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

#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

/******* Minimal Findlets **********/

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the ChooseableFacetFilter
     */
    class FacetCreatorModule : public FindletModule<FacetCreator> {

      /// Type of the base class
      using Super = FindletModule<FacetCreator>;

    public:
      /// Constructor setting the default store vector names
      FacetCreatorModule()
        : Super( {"CDCWireHitClusterVector", "CDCFacetVector"})
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
        : Super( {"CDCFacetVector", "" /*to be set externally*/})
      {
      }
    };

    /**
     * Module implementation using the SegmentLinker
     */
    class SegmentLinkerModule : public FindletModule<SegmentLinker> {
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
     * Module implementation using the SegmentRejecter
     */
    class SegmentRejecterModule : public FindletModule<SegmentRejecter> {
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
        : Super( {"CDCWireHitVector", "CDCSegment2DVector"})
      {
      }
    };

    /**
     * Module implementation using the ChooseableAxialSegmentPairFilter
     */
    class AxialSegmentPairCreatorModule : public FindletModule<AxialSegmentPairCreator> {

      /// Type of the base class
      using Super = FindletModule<AxialSegmentPairCreator>;

    public:
      /// Constructor setting the default store vector names
      AxialSegmentPairCreatorModule()
        : Super( {"CDCSegment2DVector", "CDCAxialSegmentPairVector"})
      {
      }
    };

    /**
     * Module implementation using the ChooseableSegmentPairFilter
     */
    class SegmentPairCreatorModule : public FindletModule<SegmentPairCreator> {

      /// Type of the base class
      using Super = FindletModule<SegmentPairCreator>;

    public:
      /// Constructor setting the default store vector names
      SegmentPairCreatorModule()
        : Super( {"CDCSegment2DVector", "CDCSegmentPairVector"})
      {
      }
    };

    /**
     * Module implementation using the ChooseableSegmentTripleFilter
     */
    class SegmentTripleCreatorModule : public FindletModule<SegmentTripleCreator> {

      /// Type of the base class
      using Super = FindletModule<SegmentTripleCreator>;

    public:
      /// Constructor setting the default store vector names
      SegmentTripleCreatorModule()
        : Super( {"CDCSegment2DVector", "CDCAxialSegmentVector", "CDCSegmentTripleVector"})
      {
      }
    };

    /**
     * Module implementation using the TrackCreatorSegmentPairAutomaton
     */
    class TrackCreatorSegmentPairAutomatonModule : public FindletModule<TrackCreatorSegmentPairAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackCreatorSegmentPairAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackCreatorSegmentPairAutomatonModule()
        : Super( {"CDCSegmentPairVector", "" /*to be set externally*/})
      {
      }
    };

    /**
     * Module implementation using the TrackCreatorSegmentTripleAutomaton
     */
    class TrackCreatorSegmentTripleAutomatonModule : public FindletModule<TrackCreatorSegmentTripleAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackCreatorSegmentTripleAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackCreatorSegmentTripleAutomatonModule()
        : Super( {"CDCSegmentTripleVector", "" /*to be set externally*/})
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
        : Super( {"CDCSegment2DVector", "CDCTrackVector"})
      {
      }
    };

    /**
     * Module implementation using the TrackLinker
     */
    class TrackLinkerModule : public FindletModule<TrackLinker> {
    };

    /**
     * Module implementation using the TrackCombiner
     */
    class TrackCombinerModule : public FindletModule<TrackCombiner> {
    };
  }
}

/******* Combined Findlets **********/
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentTripleAutomaton.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Combined Module using a combination of filters for segments
     */
    class SegmentFinderFacetAutomatonModule : public FindletModule<SegmentFinderFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentFinderFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      SegmentFinderFacetAutomatonModule()
        : Super( {"CDCWireHitVector", "CDCSegment2DVector"})
      {
      }
    };

    /**
     * Combined Module using a combination of filters on segment pairs
     */
    class TrackFinderSegmentPairAutomatonModule : public FindletModule<TrackFinderSegmentPairAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackFinderSegmentPairAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackFinderSegmentPairAutomatonModule()
        : Super( {"CDCSegment2DVector", "CDCTrackVector"})
      {
      }
    };

    /**
     * Combined Module using a combination of filters on segment triples
     */
    class TrackFinderSegmentTripleAutomatonModule : public FindletModule<TrackFinderSegmentTripleAutomaton> {

      /// Type of the base class
      using Super = FindletModule<TrackFinderSegmentTripleAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TrackFinderSegmentTripleAutomatonModule()
        : Super( {"CDCSegment2DVector", "CDCTrackVector"})
      {
      }
    };
  }
}
