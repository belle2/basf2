/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {

  template <class AFilter>
  LayerSVDRelationFilter<AFilter>::LayerSVDRelationFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
  }

  template <class AFilter>
  void LayerSVDRelationFilter<AFilter>::beginRun()
  {
    Super::beginRun();

    // Fill maximum number cache
    auto& geoCache = VXD::GeoCache::getInstance();
    const auto& layers = geoCache.getLayers(VXD::SensorInfoBase::SensorType::SVD);
    for (const auto& layerVXDID : layers) {
      m_maximalLadderCache[layerVXDID.getLayerNumber()] = geoCache.getLadders(layerVXDID).size();
    }
  }

  template <class AFilter>
  LayerSVDRelationFilter<AFilter>::~LayerSVDRelationFilter() = default;

  template <class AFilter>
  std::vector<CKFToSVDState*>
  LayerSVDRelationFilter<AFilter>::getPossibleTos(CKFToSVDState* currentState,
                                                  const std::vector<CKFToSVDState*>& states) const
  {
    std::vector<CKFToSVDState*> possibleNextStates;
    possibleNextStates.reserve(states.size());

    const unsigned int currentLayer = currentState->getGeometricalLayer();
    const unsigned int nextLayer = std::max(static_cast<int>(currentLayer) - 1 - m_param_hitJumping, 0);

    for (CKFToSVDState* nextState : states) {
      const unsigned int layer = nextState->getGeometricalLayer();
      if (std::max(currentLayer, nextLayer) >= layer and layer >= std::min(currentLayer, nextLayer)) {

        if (currentLayer == layer) {
          const SpacePoint* const currentSpacePoint = currentState->getHit();
          const SpacePoint* const nextSpacePoint = nextState->getHit();

          const VxdID& fromVXDID = currentSpacePoint->getVxdID();
          const VxdID& toVXDID = nextSpacePoint->getVxdID();
          // next layer is an overlap one, so lets return all hits from the same layer, that are on a
          // ladder which is below the last added hit.
          const unsigned int fromLadderNumber = fromVXDID.getLadderNumber();
          const unsigned int maximumLadderNumber = m_maximalLadderCache.find(fromVXDID.getLayerNumber())->second;

          // the reason for this strange formula is the numbering scheme in the VXD.
          // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
          // then we add (PXD)/subtract(SVD) one to get to the next (overlapping) ladder and do a % N to also cope for the
          // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
          // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
          const int direction = -1;
          const unsigned int overlappingLadder =
            ((fromLadderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

          if (toVXDID.getLadderNumber() != overlappingLadder) {
            continue;
          }

          // Next we make sure to not have any cycles in our graph: we do this by defining only the halves of the
          // sensor as overlapping. So if the first hit is coming from sensor 1 and the second from sensor 2,
          // they are only related if the one from sensor 1 is on the half, that is pointing towards sensor 2
          // and the one on sensor 2 is on the half that is pointing towards sensor 1.
          //
          //                       X                         X                         X
          //               ----|----                    ----|----                    ----|----
          //  This is fine:         X        This not:                X   This not:          X
          //                      ----|----                    ----|----                    ----|----
          const double currentStateU = currentSpacePoint->getNormalizedLocalU();
          if (currentStateU > 0.2) {
            continue;
          }

          const double nextStateU = nextSpacePoint->getNormalizedLocalU();
          if (nextStateU <= 0.8) {
            continue;
          }

        }


        possibleNextStates.push_back(nextState);
      }
    }

    return possibleNextStates;
  }

  template <class AFilter>
  void LayerSVDRelationFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumping"), m_param_hitJumping,
                                  "Make it possible to jump over N layers.", m_param_hitJumping);

    m_filter.exposeParameters(moduleParamList, prefix);
  }

  template <class AFilter>
  TrackFindingCDC::Weight LayerSVDRelationFilter<AFilter>::operator()(const CKFToSVDState& from, const CKFToSVDState& to)
  {
    return m_filter(std::make_pair(&from, &to));
  }
}