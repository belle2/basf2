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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     *  This helper supports the legacy behaviour that the wire hits can be obtained through CDCWireHitTopology.
     *  It does so by giving a reference to the wire hit vector to the CDCWireHitTopology.
     *  Since the memory of the wire hits is provided externally by the DataStore in the new scheme
     *  the vector providing the hits should not be destroyed as long as the hits need to be accessed
     *  from the CDCWireHitTopology.
     */
    class WireHitTopologyFiller : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Short description of the findlet
      std::string getDescription() override final;

      /// Signals the start of the event processing
      void initialize() override final;

      /// Main method that publishes the given wire hits in the CDCWireHitTopology.
      void apply(std::vector<CDCWireHit>& wireHits) override final;
    };
  }
}
