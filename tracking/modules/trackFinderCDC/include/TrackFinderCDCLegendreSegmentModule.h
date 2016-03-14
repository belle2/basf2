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

//#define LOG_NO_B2DEBUG

// Base track finder module
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>
#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreTrackingModule.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class CDCLegendreTrackingModule:
    public SegmentFinderCDCFacetAutomatonModule,
    public TrackFinderCDCLegendreModule {

  public:

    /// Constructor of the module. Setting up parameters and description.
    CDCLegendreTrackingModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  }; // end class
} // end namespace Belle2

