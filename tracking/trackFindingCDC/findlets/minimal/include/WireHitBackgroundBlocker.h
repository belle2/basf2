/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Marks hits as background based on simple heuristics
    class WireHitBackgroundBlocker : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm marking hit as background
      void apply(std::vector<CDCWireHit>& wireHits) final;

    private:
      /// Parameter : Switch to drop negative drift lengths from the created wire hits
      bool m_param_blockNegativeDriftLength = false;

      /// Parameter : Threshold below, which hits are consider electronic noise background
      double m_param_noiseChargeDeposit = 0;
    };
  }
}
