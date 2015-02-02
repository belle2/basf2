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

// Base track finder module
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>


// Workers
#include <tracking/trackFindingCDC/workers/SegmentPairTrackingWorker.h>
#include <tracking/trackFindingCDC/workers/SegmentTripleTrackingWorker.h>

// Filters
#include <tracking/trackFindingCDC/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_stereo/SimpleAxialStereoSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/SimpleAxialStereoSegmentPairNeighborChooser.h>

#include <tracking/trackFindingCDC/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple_segment_triple/SimpleSegmentTripleNeighborChooser.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on cosmic events
  class TrackFinderCDCCosmicsModule:  public SegmentFinderCDCFacetAutomatonModule {

  public:

    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCCosmicsModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  private:
    /// Worker to carry out the second stage generating tracks from segments.
    TrackFindingCDC::SegmentTripleTrackingWorker <
    TrackFindingCDC::SimpleAxialAxialSegmentPairFilter,
                    TrackFindingCDC::SimpleSegmentTripleFilter,
                    TrackFindingCDC::SimpleSegmentTripleNeighborChooser
                    > m_segmentTripleTrackingWorker;

    /// Worker to carry out the second stage generating tracks from segments utilizing pairs of segments.
    TrackFindingCDC::SegmentPairTrackingWorker <
    TrackFindingCDC::SimpleAxialStereoSegmentPairFilter,
                    TrackFindingCDC::SimpleAxialStereoSegmentPairNeighborChooser
                    > m_segmentPairTrackingWorker;

  }; // end class
} // end namespace Belle2

#endif /* TRACKFINDERCDCCOSMICSMODULE_H_ */

