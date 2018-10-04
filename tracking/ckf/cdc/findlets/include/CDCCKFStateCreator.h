/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class CDCCKFStateCreator
    : public TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState,
      const TrackFindingCDC::CDCWireHit* const> {
  public:

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalLayerJump"),
                                    m_maximalLayerJump, "Maximal jump over N layers", m_maximalLayerJump);
    }

    void apply(std::vector<CDCCKFState>& nextStates, const CDCCKFPath& path,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits) override
    {

      const auto& lastState = path.back();
      for (const TrackFindingCDC::CDCWireHit* wireHit : wireHits) {
        const auto sameWireHit = [wireHit](const auto & state) {
          return not state.isSeed() and state.getWireHit() == wireHit;
        };
        if (TrackFindingCDC::any(path, sameWireHit)) {
          continue;
        }

        const auto iCLayer = wireHit->getWire().getICLayer();
        if (lastState.isSeed()) {
          if (iCLayer >= m_maximalLayerJump) {
            continue;
          }
        } else {
          const TrackFindingCDC::CDCWireHit* lastWireHit = lastState.getWireHit();
          const auto lastICLayer = lastWireHit->getWire().getICLayer();
          if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump) {
            continue;
          }
        }

        nextStates.emplace_back(wireHit);
      }
    }

  private:
    int m_maximalLayerJump = 2;
  };
}
