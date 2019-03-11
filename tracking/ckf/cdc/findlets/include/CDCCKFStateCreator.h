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

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Angle.h>


namespace Belle2 {

  /// Create CKF states, based on the current path. Perform some basic selection at this stage (based on phi, max. jump of layers)
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
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitFindingDirection"),
                                    m_param_writeOutDirectionAsString, "Start from innermost/outermost CDC layers", m_param_writeOutDirectionAsString);
    }

    /// Clear the wireHit cache
    void beginEvent() override
    {
      Super::beginEvent();
      m_wireHitCache.clear();

      //Determine direction of track building
      m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);
    }

    /// Main method of the findlet. Select + create states (output parameter nextStates) suitable for the input path, based on input wireHits
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
      double lastPhi = 0;
      double lastICLayer = 0;
      if (lastState.isSeed()) {
        if (m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Backward) {
          lastICLayer = 56;
        } else if (m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Unknown
                   || m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Invalid) {
          B2WARNING("CDCCKFStateCreator: No valid direction specified. Please use foward/backward.");
        }
      } else {
        lastPhi = lastState.getWireHit()->getRefPos2D().phi();
        lastICLayer = lastState.getWireHit()->getWire().getICLayer();
      }

      // Get sorted vector of wireHits on the path for faster search
      std::vector<const TrackFindingCDC::CDCWireHit*> wireHitsOnPath;
      for (auto const& state : path) {
        if (! state.isSeed()) {
          wireHitsOnPath.push_back(state.getWireHit());
        }
      }
      std::sort(wireHitsOnPath.begin(), wireHitsOnPath.end());

      for (size_t i = 0; i < wireHits.size(); i++) {
        const auto iCLayer =  m_wireHitCache[i].icLayer; // wireHit->getWire().getICLayer();
        if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump) {
          continue;
        }

        const TrackFindingCDC::CDCWireHit* wireHit = wireHits[i];

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
    /// Maximum allowed step over layers
    int m_maximalLayerJump = 2;
    /// Maximal distance in phi between the path last hit/seed and the candidate hit
    double m_maximalDeltaPhi =  TMath::Pi() / 8;
    /// Parameter for the direction in which the tracks are built
    std::string m_param_writeOutDirectionAsString = "forward";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_writeOutDirection = TrackFindingCDC::EForwardBackward::c_Unknown;

    /// Cache to store frequently used information
    std::vector<CDCCKFWireHitCache> m_wireHitCache = {};
  };
}
