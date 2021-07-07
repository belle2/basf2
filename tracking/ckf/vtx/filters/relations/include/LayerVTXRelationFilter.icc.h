/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/vtx/filters/relations/LayerVTXRelationFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {

  template <class AFilter, class APrefilter>
  LayerVTXRelationFilter<AFilter, APrefilter>::LayerVTXRelationFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
    Super::addProcessingSignalListener(&m_prefilter);
  }

  template <class AFilter, class APrefilter>
  void LayerVTXRelationFilter<AFilter, APrefilter>::beginRun()
  {
    Super::beginRun();

    // Fill maximum number cache
    auto& geoCache = VXD::GeoCache::getInstance();
    const auto& layers = geoCache.getLayers(VXD::SensorInfoBase::SensorType::VTX);
    for (const auto& layerVXDID : layers) {
      m_maximalLadderCache[layerVXDID.getLayerNumber()] = geoCache.getLadders(layerVXDID).size();
    }
  }

  template <class AFilter, class APrefilter>
  LayerVTXRelationFilter<AFilter, APrefilter>::~LayerVTXRelationFilter() = default;

  template <class AFilter, class APrefilter>
  std::vector<CKFToVTXState*>
  LayerVTXRelationFilter<AFilter, APrefilter>::getPossibleTos(CKFToVTXState* currentState,
                                                              const std::vector<CKFToVTXState*>& states) const
  {
    std::vector<CKFToVTXState*> possibleNextStates;
    possibleNextStates.reserve(states.size());

    const CKFToVTXState::stateCache& currentStateCache = currentState->getStateCache();
    const unsigned int currentLayer = currentStateCache.geoLayer;
    const unsigned int nextPossibleLayer = std::max(static_cast<int>(currentLayer) - 1 - m_param_hitJumping, 0);

    for (CKFToVTXState* nextState : states) {
      if (currentState == nextState) {
        continue;
      }

      const CKFToVTXState::stateCache& nextStateCache = nextState->getStateCache();
      const unsigned int nextLayer = nextStateCache.geoLayer;

      if (std::max(currentLayer, nextPossibleLayer) >= nextLayer and nextLayer >= std::min(currentLayer, nextPossibleLayer)) {

        if (currentLayer == nextLayer) {
          // next layer is an overlap one, so lets return all hits from the same layer, that are on a
          // ladder which is below the last added hit.
          const unsigned int fromLadderNumber = currentStateCache.ladder;
          const unsigned int maximumLadderNumber = m_maximalLadderCache.find(currentLayer)->second;

          // the reason for this strange formula is the numbering scheme in the VXD / VTX.
          // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
          // then we add (PXD and VTX)/subtract(SVD) one to get to the next (overlapping) ladder and do a % N to also cope for the
          // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
          // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
          const int direction = 1;
          const unsigned int overlappingLadder =
            ((fromLadderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

          if (nextStateCache.ladder != overlappingLadder) {
            continue;
          }

          // Next we make sure to not have any cycles in our graph: we do this by defining only the halves of the
          // sensor as overlapping. So if the first hit is coming from sensor 1 and the second from sensor 2,
          // they are only related if the one from sensor 1 is on the half, that is pointing towards sensor 2
          // and the one on sensor 2 is on the half that is pointing towards sensor 1.
          //
          //                       X                            X                                  X
          //                      ----|----                    ----|----                    ----|----
          //  This is fine:       X          This not      X              This not:          X
          //                ----|----                    ----|----                    ----|----
          if (currentStateCache.localNormalizedu <= 0.8) {
            continue;
          }

          if (nextStateCache.localNormalizedu > 0.2) {
            continue;
          }
        }

        // Some loose prefiltering of possible states
        TrackFindingCDC::Weight weight = m_prefilter(std::make_pair(currentState, nextState));
        if (std::isnan(weight)) {
          continue;
        }

        possibleNextStates.push_back(nextState);
      }
    }

    return possibleNextStates;
  }

  template <class AFilter, class APrefilter>
  void LayerVTXRelationFilter<AFilter, APrefilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumping"), m_param_hitJumping,
                                  "Make it possible to jump over N layers.", m_param_hitJumping);

    m_filter.exposeParameters(moduleParamList, prefix);
    m_prefilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("pre", prefix));
  }

  template <class AFilter, class APrefilter>
  TrackFindingCDC::Weight LayerVTXRelationFilter<AFilter, APrefilter>::operator()(const CKFToVTXState& from, const CKFToVTXState& to)
  {
    return m_filter(std::make_pair(&from, &to));
  }
}
