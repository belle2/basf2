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

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks all hits that are not on the first loop of the track by considering the mc truth information as background
    class WireHitMCMultiLoopBlocker : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /// Expose the parameters of the wire hit preparation
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix) override;

      /// Signals the start of the event processing
      virtual void initialize() override final;

      /// Prepare the Monte Carlo information at the start of the event
      virtual void beginEvent() override final;

      /// Main algorithm marking the hit of higher loops as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final;

    private:
      /// Parameter : Maximal fraction of loops of the mc particles trajectory needs to the hit to unblock it.
      double m_param_useNLoops = INFINITY;

    };

  }
}
