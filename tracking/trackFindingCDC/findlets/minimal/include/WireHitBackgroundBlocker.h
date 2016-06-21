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
      typedef Findlet<CDCWireHit&> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Marks hits as background based on simple heuristics.";
      }

      /// Expose the parameters of the wire hit preparation
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override
      {
        moduleParamList->addParameter(prefixed(prefix, "blockNegativeDriftLength"),
                                      m_param_blockNegativeDriftLength,
                                      "Switch to drop wire hits with negative "
                                      "drift lengths from output",
                                      m_param_blockNegativeDriftLength);

        moduleParamList->addParameter(prefixed(prefix, "noiseChargeDeposit"),
                                      m_param_noiseChargeDeposit,
                                      "Threshold above which the hit is considered"
                                      "drift lengths from output",
                                      m_param_noiseChargeDeposit);



      }

      /// Main algorithm marking the hit of higher loops as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final
      {
        for (CDCWireHit& wireHit : wireHits) {
          bool markAsBackground = false;

          if (wireHit.getRefDriftLength() < 0 and m_param_blockNegativeDriftLength) {
            markAsBackground = true;
          }

          if (wireHit.getRefChargeDeposit() < m_param_noiseChargeDeposit) {
            markAsBackground = true;
          }

          if (markAsBackground) {
            wireHit.getAutomatonCell().setBackgroundFlag();
            wireHit.getAutomatonCell().setTakenFlag();
          }
        }
      }

    private:
      /// Parameter : Switch to drop negative drift lengths from the created wire hits
      bool m_param_blockNegativeDriftLength = false;

      /// Parameter : Threshold below, which hits are consider noise background
      double m_param_noiseChargeDeposit = NAN;

    }; // end class WireHitBackgroundBlocker

  } // end namespace TrackFindingCDC
} // end namespace Belle2
