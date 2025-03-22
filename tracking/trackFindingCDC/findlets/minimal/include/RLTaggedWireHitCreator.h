/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCWireHit;
  }

  namespace TrackFindingCDC {
    class CDCRLWireHit;

    /**
     * Findlet for creating a list of RLWireHits (always bot hypothesis) from a list of WireHits
     * using only the non-taken non-axial hits. Additionally, the assigned flag of all
     * those hits is set to false.
     */
    class RLTaggedWireHitCreator : public TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit&, CDCRLWireHit> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit&, CDCRLWireHit&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Generates the segment from wire hits
      void apply(std::vector<TrackingUtilities::CDCWireHit>& inputWireHits, std::vector<CDCRLWireHit>& outputRLWireHits) final;
    };
  }
}
