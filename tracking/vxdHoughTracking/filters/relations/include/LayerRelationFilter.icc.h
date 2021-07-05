/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/relations/LayerRelationFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    template <class AFilter>
    LayerRelationFilter<AFilter>::LayerRelationFilter() : Super()
    {
      Super::addProcessingSignalListener(&m_filter);
    }

    template <class AFilter>
    void LayerRelationFilter<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumping"), m_param_hitJumping,
                                    "Make it possible to jump over N layers.", m_param_hitJumping);

      m_filter.exposeParameters(moduleParamList, prefix);
    }

    template <class AFilter>
    void LayerRelationFilter<AFilter>::beginRun()
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
    LayerRelationFilter<AFilter>::~LayerRelationFilter() = default;

    template <class AFilter>
    std::vector<VXDHoughState*>
    LayerRelationFilter<AFilter>::getPossibleTos(VXDHoughState* currentHit, const std::vector<VXDHoughState*>& hits) const
    {
      std::vector<VXDHoughState*> possibleNextHits;
      possibleNextHits.reserve(hits.size());

      const VXDHoughState::DataCache& currentVXDHoughState = currentHit->getDataCache();
      const unsigned int currentLayer = currentVXDHoughState.layer;
      const unsigned int nextPossibleLayer = std::max(static_cast<int>(currentLayer) - 1 - m_param_hitJumping, 0);

      for (VXDHoughState* nextHit : hits) {
        if (currentHit == nextHit) {
          continue;
        }

        const VXDHoughState::DataCache& nextVXDHoughState = nextHit->getDataCache();
        const unsigned int nextLayer = nextVXDHoughState.layer;

        if (std::max(currentLayer, nextPossibleLayer) >= nextLayer and nextLayer >= std::min(currentLayer, nextPossibleLayer)) {

          if (currentLayer == nextLayer) {
            // next layer is an overlap one, so lets return all hits from the same layer, that are on a
            // ladder which is below the last added hit.
            const unsigned int fromLadderNumber = currentVXDHoughState.ladder;
            const unsigned int maximumLadderNumber = m_maximalLadderCache.at(currentLayer);

            // the reason for this strange formula is the numbering scheme in the VXD.
            // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
            // then we add (PXD)/subtract(SVD) one to get to the next (overlapping) ladder and do a % N to also cope for the
            // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
            // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
            const int direction = -1;
            const unsigned int overlappingLadder =
              ((fromLadderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

            if (nextVXDHoughState.ladder != overlappingLadder) {
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
            if (currentVXDHoughState.localNormalizedu > 0.2) {
              continue;
            }

            if (nextVXDHoughState.localNormalizedu <= 0.8) {
              continue;
            }
          }

          possibleNextHits.push_back(nextHit);
        }
      }

      return possibleNextHits;
    }

    template <class AFilter>
    TrackFindingCDC::Weight LayerRelationFilter<AFilter>::operator()(const VXDHoughState& from, const VXDHoughState& to)
    {
      return m_filter(std::make_pair(&from, &to));
    }

  }
}
