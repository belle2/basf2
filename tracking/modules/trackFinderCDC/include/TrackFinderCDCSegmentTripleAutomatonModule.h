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
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <vector>
namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generates tracks from segments using a cellular automaton build from segment triples.
    class TrackFinderCDCSegmentTripleAutomatonModule
      : public FindletModule<TrackFinderSegmentTripleAutomaton<SimpleAxialSegmentPairFilter,
        SimpleSegmentTripleFilter,
        SimpleSegmentTripleRelationFilter,
        ChooseableTrackRelationFilter> > {

      /// Type of the base class
      using Super = FindletModule<TrackFinderSegmentTripleAutomaton<SimpleAxialSegmentPairFilter,
            SimpleSegmentTripleFilter,
            SimpleSegmentTripleRelationFilter,
            ChooseableTrackRelationFilter> >;

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentTripleAutomatonModule() :
        Super{{{"CDCRecoSegment2DVector", "CDCTrackVector"}}}
      {}

    }; // end class TrackFinderCDCSegmentTripleAutomatonModule
  }
}
