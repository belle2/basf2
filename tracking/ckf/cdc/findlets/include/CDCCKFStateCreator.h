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
#include <tracking/trackFindingCDC/numerics/Angle.h>


namespace Belle2 {


  class CDCCKFStateCreator
    : public TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState,
      const TrackFindingCDC::CDCWireHit* const > {

    /// Parent class
    using Super = TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState, const TrackFindingCDC::CDCWireHit* const>;

    /// Store basic wire info for faster access
    struct CDCCKFWireHitCache {
      int     icLayer;
      double  phi;
    };


  public:

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalLayerJump"),
                                    m_maximalLayerJump, "Maximal jump over N layers", m_maximalLayerJump);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalDeltaPhi"),
                                    m_maximalDeltaPhi, "Maximal distance in phi between wires for Z=0 plane", m_maximalDeltaPhi);
    }

    /// Clear the wire cache
    void beginEvent() override
    {
      Super::beginEvent();
      m_wireHitCache.clear();
    }


    void apply(std::vector<CDCCKFState>& nextStates, const CDCCKFPath& path,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits) override
    {
      // TODO: as we do not need any information on the current state (track state) of the path, we could in principle
      // TODO: precalculate everything in here

      // Create cache over wirehits, if empty:
      if (m_wireHitCache.empty()) {
        const size_t nHits = wireHits.size();
        m_wireHitCache.reserve(nHits);
        for (auto  hitPtr : wireHits) {
          m_wireHitCache.push_back(CDCCKFWireHitCache{hitPtr->getWire().getICLayer(), hitPtr->getRefPos2D().phi()});
        }
      }

      // Cache last-on-the-path state info too:
      const auto& lastState = path.back();
      int lastICLayer =  lastState.isSeed() ? 0 : lastState.getWireHit()->getWire().getICLayer();
      double lastPhi  =  lastState.isSeed() ? 0 : lastState.getWireHit()->getRefPos2D().phi();

      // Get sorted vector of wireHits on the path for faster search
      std::vector<const TrackFindingCDC::CDCWireHit*> wireHitsOnPath;
      for (auto const& state : path) {
        if (! state.isSeed()) {
          wireHitsOnPath.push_back(state.getWireHit());
        }
      }
      std::sort(wireHitsOnPath.begin(), wireHitsOnPath.end());


      for (size_t i = 0; i < wireHits.size(); i++) {

        const TrackFindingCDC::CDCWireHit* wireHit = wireHits[i];

        const auto iCLayer =  m_wireHitCache[i].icLayer; // wireHit->getWire().getICLayer();
        if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump) {
          continue;
        }

        if (std::binary_search(wireHitsOnPath.begin(), wireHitsOnPath.end(), wireHit)) {
          continue;
        }


        if (! lastState.isSeed()) {
          double deltaPhi = TrackFindingCDC::AngleUtil::normalised(lastPhi - m_wireHitCache[i].phi);
          if (fabs(deltaPhi)  > m_maximalDeltaPhi)  {
            continue;
          }
        }

        nextStates.emplace_back(wireHit);
      }
    }

  private:
    int m_maximalLayerJump = 2;
    double m_maximalDeltaPhi =  TMath::Pi() / 8;
    std::vector<CDCCKFWireHitCache> m_wireHitCache = {};
  };
}
