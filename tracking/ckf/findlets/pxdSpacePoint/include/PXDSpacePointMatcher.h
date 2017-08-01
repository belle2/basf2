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

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/ChainedIterator.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class PXDSpacePointMatcher : public TrackFindingCDC::ProcessingSignalListener {
    using ReturnType = TrackFindingCDC::ChainedArray<TrackFindingCDC::VectorRange<const SpacePoint*>>;
    using Layer = unsigned int;
    using Ladder = unsigned int;
    using LayerAndLadder = std::pair<Layer, Ladder>;
  public:
    /// return the next hits for a given state, which are the hits on the next layer (or the same for overlaps)
    template<class AStateObject>
    ReturnType getMatchingHits(AStateObject& currentState);

    /// Fill the cache of hits for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector);

    /// Expose parameters (if we would have such a thing)
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {
    }

  private:
    /// Cache for sorted hits, grouped by layer and ladder
    std::map<LayerAndLadder, TrackFindingCDC::VectorRange<const SpacePoint*>> m_cachedHitMapOnLadder;
    /// Cache for sorted hits, grouped by layer
    std::map<Layer, TrackFindingCDC::VectorRange<const SpacePoint*>> m_cachedHitMapOnLayer;
  };

  template<class AStateObject>
  PXDSpacePointMatcher::ReturnType PXDSpacePointMatcher::getMatchingHits(
    AStateObject& currentState)
  {
    const unsigned int currentNumber = currentState.getNumber();

    if (currentNumber == currentState.getMaximumNumber() or isOnOverlapLayer(currentState)) {
      // next layer is not an overlap one. Lets return all possible ladders on the next layer
      const unsigned int currentLayer = extractGeometryLayer(currentState);
      const unsigned int nextLayer = currentLayer - 1;

      unsigned int currentLadder = 0;
      if (currentLayer == 3) {
        // if the current layer is 3, we get the most-inner ladder from the associated reco track, if it has an SVD hit in the
        // lowest layer
        const auto* seed = currentState.getSeedRecoTrack();
        const auto& relatedSVDHits = seed->getSortedSVDHitList();
        if (not relatedSVDHits.empty()) {
          const auto* firstSVDHit = relatedSVDHits.front();
          const auto& firstSVDId = firstSVDHit->getSensorID();

          if (firstSVDId.getLayerNumber() == currentLayer) {
            currentLadder = firstSVDId.getLadderNumber();
          }
        }
      } else if (currentLayer == 2) {
        // if the current layer is 2, we see if we have a PXD hit in this layer. If yes, we use the ladder of this.
        // For this, we have to go up another step in the hierarchy, as we do not want to end up with overlap layers.
        const auto* nonOverlappingParent = currentState.getParent();
        B2ASSERT("No parent!", nonOverlappingParent);
        const auto* spacePoint = nonOverlappingParent->getHit();
        if (spacePoint) {
          currentLadder = spacePoint->getVxdID().getLadderNumber();
        }
      } else {
        B2FATAL("Should not happen!");
      }

      if (currentLadder == 0) {
        // Return everything
        return ReturnType({m_cachedHitMapOnLayer[nextLayer]});
      } else {
        // Return only parts
        /// "SectorMap"-like structure, which defined which ladders may be related to which
        static std::map<LayerAndLadder, std::vector<LayerAndLadder>> ladderMapping = {
          {
            {3, 1},
            {{2, 1}, {2, 2}, {2, 3}}
          },
          {
            {3, 2},
            {{2, 3}, {2, 4}}
          },
          {
            {3, 3},
            {{2, 4}, {2, 5}, {2, 6}}
          },
          {
            {3, 4},
            {{2, 6}, {2, 7}, {2, 8}}
          },
          {
            {3, 5},
            {{2, 8}, {2, 9}}
          },
          {
            {3, 6},
            {{2, 9}, {2, 10}, {2, 11}, {2, 12}}
          },
          {
            {3, 7},
            {{2, 11}, {2, 12}, {2, 1}}
          },


          {
            {2, 1},
            {{1, 1}}
          },
          {
            {2, 2},
            {{1, 1}, {1, 2}}
          },
          {
            {2, 3},
            {{1, 2}, {1, 3}}
          },
          {
            {2, 4},
            {{1, 3}}
          },
          {
            {2, 5},
            {{1, 3}, {1, 4}}
          },
          {
            {2, 6},
            {{1, 4}, {1, 5}}
          },
          {
            {2, 7},
            {{1, 5}}
          },
          {
            {2, 8},
            {{1, 5}, {1, 6}}
          },
          {
            {2, 9},
            {{1, 6}, {1, 7}}
          },
          {
            {2, 10},
            {{1, 7}}
          },
          {
            {2, 11},
            {{1, 7}, {1, 8}}
          },
          {
            {2, 12},
            {{1, 8}, {1, 1}}
          },
        };


        std::vector<TrackFindingCDC::VectorRange<const SpacePoint*>> nextRanges;
        const auto& nextLayerAndLadder = LayerAndLadder(currentLayer, currentLadder);
        for (const auto& nextLayerAndLadder : ladderMapping[nextLayerAndLadder]) {
          nextRanges.push_back(m_cachedHitMapOnLadder[nextLayerAndLadder]);
        }
        return ReturnType(nextRanges);
      }

    } else {
      // TODO: overlaps
      return {};
    }
  }
}
