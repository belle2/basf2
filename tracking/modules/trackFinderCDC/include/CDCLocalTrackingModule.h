/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// Base track finder module
#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonModule.h>

namespace Belle2 {

  /// Depricated alias for TrackFinderCDCAutomatonModule
  class CDCLocalTrackingModule : public TrackFinderCDCAutomatonModule {

  public:
    /// Constructor of the module. Setting up parameters and description.
    CDCLocalTrackingModule();

    ///  Initialize the Module before event processing
    void initialize();

  }; // end class
} // end namespace Belle2

