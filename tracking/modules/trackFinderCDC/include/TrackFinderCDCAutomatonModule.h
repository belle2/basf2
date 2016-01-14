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

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/findlets/complete/TrackFinderAutomaton.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Module for the cellular automaton tracking for the CDC on regular events
    class TrackFinderCDCAutomatonModule
      : public FindletModule<TrackFinderAutomaton> {
    }; // end class
  }
} // end namespace Belle2

