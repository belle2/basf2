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

namespace Belle2 {
  class CDCCKFStateCreator
    : public TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState,
      const TrackFindingCDC::CDCWireHit* const> {
  public:
    void apply(std::vector<CDCCKFState>& nextStates, const CDCCKFPath& path,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits) override
    {

      const auto& lastState = path.back();
      for (const TrackFindingCDC::CDCWireHit* wireHit : wireHits) {
        const auto iCLayer = wireHit->getWire().getICLayer();
        if (lastState.isSeed()) {
          if (iCLayer > m_maximalLayerJump) {
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
