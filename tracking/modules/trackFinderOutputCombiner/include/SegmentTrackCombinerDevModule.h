/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerModule.h>

#include <tracking/trackFindingCDC/filters/segment_information_list_track/SegmentInformationListTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentsFilterFactory.h>
#include <tracking/trackFindingCDC/filters/new_segment/NewSegmentsFilterFactory.h>
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>

namespace Belle2 {

  /// Module for the combination of tracks and segments. Development edition.
  class SegmentTrackCombinerDevModule:
    public Belle2::TrackFindingCDC::SegmentTrackCombinerImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentTrackCombinerDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:

    /**
       Factory for the segment track chooser filter for the first step, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentTrackFilterFirstStepFactory m_segmentTrackChooserFirstStepFactory;

    /**
       Factory for the background segment filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::BackgroundSegmentsFilterFactory m_backgroundSegmentsFilterFactory;

    /**
       Factory for the new segment filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::NewSegmentsFilterFactory m_newSegmentsFilterFactory;

    /**
       Factory for the segment track chooser filter for the second step, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentTrackFilterSecondStepFactory m_segmentTrackChooserSecondStepFactory;

    /**
       Factory for the segment train filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentTrainFilterFactory m_segmentTrainFilterFactory;

    /**
       Factory for the segment train filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentInformationListTrackFilterFactory m_segmentTrackFilterFactory;

    /**
       Factory for the track filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::TrackFilterFactory m_trackFilterFactory;

  }; // end class
} // end namespace Belle2
