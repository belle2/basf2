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

#include <tracking/modules/trackFinderCDC/TrackFinderCDCSegmentPairAutomatonModule.h>

#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

namespace Belle2 {

  /// Module for the second stage cellular automaton tracking for the CDC on regular events with filter adjustable in steering file.
  class TrackFinderCDCSegmentPairAutomatonDevModule
    : public TrackFindingCDC::TrackFinderCDCSegmentPairAutomatonImplModule<TrackFindingCDC::ChooseableSegmentPairFilter,
      TrackFindingCDC::ChooseableSegmentPairRelationFilter,
      TrackFindingCDC::ChooseableTrackRelationFilter> {
  public:
    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCSegmentPairAutomatonDevModule();

  }; // end class
} // end namespace Belle2
