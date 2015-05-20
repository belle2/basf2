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

#include <tracking/trackFindingCDC/filters/segment_pair/SegmentPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_pair_relation/SegmentPairRelationFilterFactory.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class TrackFinderCDCSegmentPairAutomatonDevModule:
    public Belle2::TrackFindingCDC::TrackFinderCDCSegmentPairAutomatonImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCSegmentPairAutomatonDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:
    /**
       Factory for the cluster filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentPairFilterFactory m_segmentPairFilterFactory;

    /**
       Factory for the cluster filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentPairRelationFilterFactory m_segmentPairRelationFilterFactory;

  }; // end class
} // end namespace Belle2
