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

#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/trackRelation/TrackRelationFilterFactory.h>

namespace Belle2 {

  /// Module for the second stage cellular automaton tracking for the CDC on regular events with filter adjustable in steering file.
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
       Factory for the segment pair filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentPairFilterFactory m_segmentPairFilterFactory;

    /**
       Factory for the segment pair relation filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentPairRelationFilterFactory m_segmentPairRelationFilterFactory;
    /**
       Factory for the track relation filter for merging, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::TrackRelationFilterFactory m_trackRelationFilterFactory;


  }; // end class
} // end namespace Belle2
