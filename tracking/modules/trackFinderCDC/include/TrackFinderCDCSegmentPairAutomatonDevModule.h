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

namespace Belle2 {

  namespace TrackFindingCDC {
    /// Deprectated : Module for the second stage cellular automaton tracking for the CDC on regular events with filter adjustable in steering file.
    class TrackFinderCDCSegmentPairAutomatonDevModule
      : public TrackFinderCDCSegmentPairAutomatonModule {

    public:
      /// Constructor of the module. Setting up parameters and description.
      TrackFinderCDCSegmentPairAutomatonDevModule();

      /// Signal beginning of event processing
      virtual void initialize() override;

    }; // end class
  }
} // end namespace Belle2
