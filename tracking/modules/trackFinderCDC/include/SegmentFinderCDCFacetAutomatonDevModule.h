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

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Module for the cellular automaton tracking for the CDC on regular events
    class SegmentFinderCDCFacetAutomatonDevModule :
      public SegmentFinderCDCFacetAutomatonModule {

    public:
      /// Constructor of the module. Setting up parameters and description.
      SegmentFinderCDCFacetAutomatonDevModule();

      /// Signal beginning of event processing
      virtual void initialize() override;

    }; // end class
  }
} // end namespace Belle2
