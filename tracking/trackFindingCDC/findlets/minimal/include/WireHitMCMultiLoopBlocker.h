/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     *  Marks all hits that are not on the first loop of the track as background.
     *
     *  By marking them as background hits are excluded from the track finding procedures
     *  Application of this findlet somewhat reduces the complexity of the track finding
     *  since only hits within a limited time need to be tracked.
     *  It is however useful to validated fitting procedures and to bias multivariate to
     *  favor the first loops and do not apply a penalty for uncovered higher order loops
     */
    class WireHitMCMultiLoopBlocker : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signals the start of the event processing
      void initialize() final;

      /// Prepare the Monte Carlo information at the start of the event
      void beginEvent() final;

      /// Main algorithm marking the hit of higher loops as background.
      void apply(std::vector<CDCWireHit>& wireHits) final;

    private:
      /// Parameter : Maximal fraction of loops of the mc particles trajectory needs to the hit to unblock it.
      double m_param_useNLoops = INFINITY;
    };
  }
}
