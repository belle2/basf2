/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
