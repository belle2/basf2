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
#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Deprecated : Module for development of the cellular automaton tracking for the CDC
    class TrackFinderCDCAutomatonDevModule
      : public TrackFinderCDCAutomatonModule {

    public:
      /// Constructor of the module. Setting up parameters and description.
      TrackFinderCDCAutomatonDevModule();

      ///  Initialize the Module before event processing
      virtual void initialize();

    }; // end class

  }
} // end namespace Belle2

