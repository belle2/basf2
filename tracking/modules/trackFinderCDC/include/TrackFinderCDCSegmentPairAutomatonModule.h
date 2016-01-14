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

#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackFinderCDCSegmentPairAutomatonModule
      : public FindletModule<TrackFinderSegmentPairAutomaton<ChooseableSegmentPairFilter,
        ChooseableSegmentPairRelationFilter,
        ChooseableTrackRelationFilter> > {

      /// Type of the base class
      using Super = FindletModule<TrackFinderSegmentPairAutomaton<ChooseableSegmentPairFilter,
            ChooseableSegmentPairRelationFilter,
            ChooseableTrackRelationFilter> >;

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentPairAutomatonModule() :
        Super{{{"CDCRecoSegment2DVector", "CDCTrackVector"}}}
      {}

    }; // end class TrackFinderCDCSegmentPairAutomatonModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
