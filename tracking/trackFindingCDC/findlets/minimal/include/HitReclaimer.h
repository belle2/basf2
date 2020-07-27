/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     * A small findlet that removes flags from hits not accepted by conventional tracking.
     */
    class HitReclaimer : public Findlet<const CDCWireHit> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit>;

    public:
      /// Constructor
      HitReclaimer();

      /// Initialisation before the event processing starts
      void initialize() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Execute over wireHits
      void apply(const std::vector<CDCWireHit>& wireHits);
    };
  }
}
