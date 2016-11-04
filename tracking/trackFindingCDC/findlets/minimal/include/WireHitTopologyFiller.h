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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** This helper supports the legacy behaviour that the wire hits can be obtained through CDCWireHitTopology.
     *  It does so by giving a reference to the wire hit vector to the CDCWireHitTopology.
     *  Since the memory of the wire hits is provided externally in the new scheme the hits should
     *  the vector providing the hits should not be destroyed as long as the hits need to be accessed
     *  from the CDCWireHitTopology.
     */
    class WireHitTopologyFiller : public Findlet<CDCWireHit&> {
    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit&> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Publishes the wire hits also through the CDCWireHitTopology to support the legacy "
               "behaviour for modules that still rely on it.";
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        CDCWireHitTopology::initialize();
        Super::initialize();
      }
      /// Main method that publishes the given wire hits in the CDCWireHitTopology.
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final
      {
        std::shared_ptr<ConstVectorRange<CDCWireHit> > sharedWireHits{
          new ConstVectorRange<CDCWireHit>(wireHits.begin(), wireHits.end())
        };
        CDCWireHitTopology::getInstance().fill(sharedWireHits);
      }

    }; // end class WireHitTopologyFiller

  } // end namespace TrackFindingCDC
} // end namespace Belle2
