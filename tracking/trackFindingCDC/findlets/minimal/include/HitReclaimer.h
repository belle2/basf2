/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCWireHit;
  }

  namespace TrackFindingCDC {

    /**
     * A small findlet that removes flags from hits not accepted by conventional tracking.
     */
    class HitReclaimer : public TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit>;

    public:
      /// Constructor
      HitReclaimer();

      /// Initialisation before the event processing starts
      void initialize() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Execute over wireHits
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& wireHits);
    };
  }
}
