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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks hits as background based on simple heuristics
    class WireHitBackgroundBlocker : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /// Expose the parameters of the wire hit preparation
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override;

      /// Main algorithm marking the hit of higher loops as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final;

    private:
      /// Parameter : Switch to drop negative drift lengths from the created wire hits
      bool m_param_blockNegativeDriftLength = false;

      /// Parameter : Threshold below, which hits are consider electronic noise background
      double m_param_noiseChargeDeposit = 0;

    }; // end class WireHitBackgroundBlocker
  } // end namespace TrackFindingCDC
} // end namespace Belle2
