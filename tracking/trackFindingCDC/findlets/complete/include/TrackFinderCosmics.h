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

#include <tracking/trackFindingCDC/findlets/complete/TrackFinderAutomaton.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages - version suitable for cosmics
    class TrackFinderCosmics : public TrackFinderAutomaton {

    private:
      /// Type of the base class
      using Super = TrackFinderAutomaton;

    public:
      /// Constructor adjusting some of the parameters to suiteable values for cosmics tracking.
      TrackFinderCosmics();

      /// Short description of the findlet
      std::string getDescription() final;
    };
  }
}
