/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun, Simon Kurz                                  *
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
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

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
      /// layer index
      int     icLayer;
      /// azimuthal coordinate
      double  phi;
    };


  public:

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalLayerJump"),
                                    m_maximalLayerJump, "Maximal jump over N layers", m_maximalLayerJump);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalLayerJumpBackwardSeed"),
                                    m_maximalLayerJump_backwardSeed, "Maximal jump over N layers", m_maximalLayerJump_backwardSeed);
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

      // Determine direction of track building
      m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);

      if (m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Forward) {
        doForward = true;
      } else if (m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Backward) {
        doForward = false;
      } else {
        B2FATAL("CDCCKFStateCreator: No valid direction specified. Please use forward/backward.");
      }
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
          // to speed things up, don't consider background/taken hits at all (and not just in the loop below).
          // I can't just remove them from the list, otherwise the relation to the wireHits is broken
          // so set the layer index to a high number.
          if (hitPtr->getAutomatonCell().hasBackgroundFlag() || hitPtr->getAutomatonCell().hasTakenFlag()) {
            m_wireHitCache.push_back(CDCCKFWireHitCache{99999, 0.});
          } else {
            m_wireHitCache.push_back(CDCCKFWireHitCache{hitPtr->getWire().getICLayer(), hitPtr->getRefPos2D().phi()});
          }
        }
      }

      // Cache last-on-the-path state info too:
      const auto& lastState = path.back();
      double lastPhi = 0;
      double lastICLayer = -1;
      if (lastState.isSeed()) {
        if (doForward) {
          lastICLayer = 0;
        } else {
          const auto& wireTopology = TrackFindingCDC::CDCWireTopology::getInstance();
          const auto& wires = wireTopology.getWires();
          const float maxForwardZ = wires.back().getForwardZ();     // 157.615
          const float maxBackwardZ = wires.back().getBackwardZ();   // -72.0916

          const TrackFindingCDC::Vector3D seedPos(lastState.getSeed()->getPositionSeed());
          const float seedPosZ = seedPos.z();

          if (seedPosZ < maxForwardZ && seedPosZ > maxBackwardZ) {
            lastICLayer = 56;
          } else {
            // do straight extrapolation of seed momentum to CDC outer walls
            TrackFindingCDC::Vector3D seedMomZOne(lastState.getSeed()->getMomentumSeed());
            seedMomZOne = seedMomZOne / seedMomZOne.z();
            // const float maxZ = seedPosZ > 0 ? maxForwardZ : maxBackwardZ;
            // const TrackFindingCDC::Vector3D extrapolatedPos = seedPos - seedMom / seedMom.norm() * (seedPosZ - maxZ);

            // find closest iCLayer
            float minDist = 99999;
            for (const auto& wire : wires) {
              const float maxZ = seedPosZ > 0 ? wire.getForwardZ() : wire.getBackwardZ();
              const TrackFindingCDC::Vector3D extrapolatedPos = seedPos - seedMomZOne * (seedPosZ - maxZ);

              const auto distance = wire.getDistance(extrapolatedPos);
              if (distance < minDist) {
                minDist = distance;
                lastICLayer = wire.getICLayer();
              }
            }
            B2DEBUG(29, lastICLayer << " (d=" << minDist << ")");
          }
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

      size_t nHits = wireHits.size();
      for (size_t i = 0; i < nHits; i++) {
        // adjust direction of loop (minimal speed gain)
        int idx = doForward ? i : nHits - i - 1;

        const auto iCLayer =  m_wireHitCache[idx].icLayer; // wireHit->getWire().getICLayer();
        if (m_param_writeOutDirection == TrackFindingCDC::EForwardBackward::c_Backward && lastState.isSeed()) {
          if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump_backwardSeed) {
            continue;
          }
        } else if (std::abs(lastICLayer - iCLayer) > m_maximalLayerJump) {
          continue;
        }

        if (! lastState.isSeed()) {
          double deltaPhi = TrackFindingCDC::AngleUtil::normalised(lastPhi - m_wireHitCache[idx].phi);
          if (fabs(deltaPhi)  > m_maximalDeltaPhi)  {
            continue;
          }
        }

        const TrackFindingCDC::CDCWireHit* wireHit = wireHits[idx];

        if (std::binary_search(wireHitsOnPath.begin(), wireHitsOnPath.end(), wireHit)) {
          continue;
        }

        nextStates.emplace_back(wireHit);
      }
    }

  private:
    /// Maximum allowed step over layers
    int m_maximalLayerJump = 2;
    /// Maximum allowed step over layers (if outside->in CKF) for first step after seed (e.g. ECLShower)
    int m_maximalLayerJump_backwardSeed = 3;
    /// Maximal distance in phi between the path last hit/seed and the candidate hit
    double m_maximalDeltaPhi =  TMath::Pi() / 8;
    /// Parameter for the direction in which the tracks are built
    std::string m_param_writeOutDirectionAsString = "forward";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_writeOutDirection = TrackFindingCDC::EForwardBackward::c_Unknown;
    /// Direction parameter converted to boolean for convenience
    bool doForward = true;

    /// Cache to store frequently used information
    std::vector<CDCCKFWireHitCache> m_wireHitCache = {};

  };
}
