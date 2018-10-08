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

  /// Store basic wire info for faster access
  struct CDCCKFWireHitCache {
    int     icLayer;
    double  phi;
  };


  class CDCCKFStateCreator
    : public TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState,
      const TrackFindingCDC::CDCWireHit* const, const CDCCKFWireHitCache > {
  public:

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalLayerJump"),
                                    m_maximalLayerJump, "Maximal jump over N layers", m_maximalLayerJump);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalDeltaPhi"),
                                    m_maximalDeltaPhi, "Maximal distance in phi between wires for Z=0 plane", m_maximalDeltaPhi);
    }

    void apply(std::vector<CDCCKFState>& nextStates, const CDCCKFPath& path,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits, const std::vector<CDCCKFWireHitCache>& wireHitCache) override
    {

      // Cache last hit layer
      const auto& lastState = path.back();
      int lastICLayer =  lastState.isSeed() ? 0 : lastState.getWireHit()->getWire().getICLayer();
      double lastPhi  =  lastState.isSeed() ? 0 : lastState.getWireHit()->getRefPos2D().phi();

      // Cache all wireHits for the current path
      std::set<const TrackFindingCDC::CDCWireHit*> wireHitsOnPath;
      for (auto const& state : path) {
        if (! state.isSeed()) {
          wireHitsOnPath.insert(state.getWireHit());
        }
      }

      for (size_t i = 0; i < wireHits.size(); i++) {

        const TrackFindingCDC::CDCWireHit* wireHit = wireHits[i];

        const auto iCLayer =  wireHitCache[i].icLayer; // wireHit->getWire().getICLayer();
        if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump) {
          continue;
        }

        if (wireHitsOnPath.find(wireHit) != wireHitsOnPath.end()) {
          continue;
        }

        if (! lastState.isSeed()) {
          //    if ( fabs(fmod(lastPhi - wireHit->getRefPos2D().phi(), TMath::Pi() ))  > m_maximalDeltaPhi)  {
          if (fabs(fmod(lastPhi - wireHitCache[i].phi, TMath::Pi()))  > m_maximalDeltaPhi)  {
            continue;
          }
        }

        nextStates.emplace_back(wireHit);
      }
    }

  private:
    int m_maximalLayerJump = 2;
    double m_maximalDeltaPhi =  TMath::Pi() / 8;
  };
}
