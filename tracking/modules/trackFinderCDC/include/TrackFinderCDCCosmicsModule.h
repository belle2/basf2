/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKFINDERCDCCOSMICSMODULE_H_
#define TRACKFINDERCDCCOSMICSMODULE_H_

//#define LOG_NO_B2DEBUG

// Workers
#include <tracking/cdcLocalTracking/workers/FacetSegmentWorker.h>
#include <tracking/cdcLocalTracking/workers/SegmentPairTrackingWorker.h>
#include <tracking/cdcLocalTracking/workers/SegmentTripleTrackingWorker.h>

// Filters
#include <tracking/cdcLocalTracking/filters/facet/SimpleFacetFilter.h>
#include <tracking/cdcLocalTracking/filters/facet_facet/SimpleFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/filters/axial_stereo/SimpleAxialStereoSegmentPairFilter.h>

#include <tracking/cdcLocalTracking/filters/axial_stereo_axial_stereo/SimpleAxialStereoSegmentPairNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/filters/segment_triple_segment_triple/SimpleSegmentTripleNeighborChooser.h>

// Base track finder module
#include <tracking/modules/trackFinderCDC/TrackFinderCDCBaseModule.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on cosmic events
  class TrackFinderCDCCosmicsModule: public TrackFinderCDCBaseModule {

  public:

    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCCosmicsModule();

    /// Destructor of the module.
    virtual ~TrackFinderCDCCosmicsModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Called when entering a new run.
    virtual void beginRun();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Called at the end of a run.
    virtual void endRun();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  private:
    /// Worker to carry out the first stage for segment generation
    CDCLocalTracking::FacetSegmentWorker <
    CDCLocalTracking::SimpleFacetFilter,
                     CDCLocalTracking::SimpleFacetNeighborChooser
                     > m_segmentWorker;

    /// Pool to store the segments between the stages
    std::vector< Belle2::CDCLocalTracking::CDCRecoSegment2D > m_recoSegments;

    /// Worker to carry out the second stage generating tracks from segments.
    CDCLocalTracking::SegmentTripleTrackingWorker <
    CDCLocalTracking::SimpleAxialAxialSegmentPairFilter,
                     CDCLocalTracking::SimpleSegmentTripleFilter,
                     CDCLocalTracking::SimpleSegmentTripleNeighborChooser
                     > m_segmentTripleTrackingWorker;

    /// Worker to carry out the second stage generating tracks from segments utilizing pairs of segments.
    CDCLocalTracking::SegmentPairTrackingWorker <
    CDCLocalTracking::SimpleAxialStereoSegmentPairFilter,
                     CDCLocalTracking::SimpleAxialStereoSegmentPairNeighborChooser
                     > m_segmentPairTrackingWorker;

  }; // end class
} // end namespace Belle2

#endif /* TRACKFINDERCDCCOSMICSMODULE_H_ */

