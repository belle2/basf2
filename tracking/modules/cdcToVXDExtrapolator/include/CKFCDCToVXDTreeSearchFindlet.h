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

#include <tracking/trackFindingCDC/findlets/minimal/TreeSearchFindlet.h>
#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/modules/cdcToVXDExtrapolator/CKFCDCToVXDResultObject.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>
#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDTreeSearchFindlet : public TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDResultObject> {
  public:
    CKFCDCToVXDTreeSearchFindlet() : TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDResultObject>()
    {
      addProcessingSignalListener(&m_hitFilter);
    }

    void apply(std::vector<RecoTrack*>& seedsVector,
               std::vector<const SpacePoint*>& filteredHitVector) final {

      const auto& hitSorterByLayer = [](const SpacePoint * lhs, const SpacePoint * rhs)
      {
        return lhs->getVxdID().getLayerNumber() < rhs->getVxdID().getLayerNumber();
      };

      std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByLayer);

      for (unsigned int layerID = 0; layerID < 8; ++layerID)
      {

        const auto& onGivenLayerCheck = [layerID](const SpacePoint * spacePoint) {
          return spacePoint->getVxdID().getLayerNumber() == layerID;
        };

        const auto first = std::find_if(filteredHitVector.begin(), filteredHitVector.end(), onGivenLayerCheck);
        const auto last = std::find_if_not(first, filteredHitVector.end(), onGivenLayerCheck);

        m_cachedHitMap.emplace(layerID, TrackFindingCDC::SortedVectorRange<const SpacePoint*>(first, last));
        B2INFO("Storing in " << layerID << " " << std::distance(first, last));
      }
    }

  protected:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(const CKFCDCToVXDResultObject& currentResult) override
    {
      const unsigned int nextLayer = currentResult.getLastLayer() - 1;
      const auto& hitsOnNextLayer = m_cachedHitMap[nextLayer];
      return hitsOnNextLayer;
      /*matchingHits.reserve(hitsOnNextLayer.size());

      for(const SpacePoint* spacePoint : hitsOnNextLayer) {
        const auto& weight = m_hitFilter(std::make_pair(currentResult, spacePoint));
        if(not std::isnan(weight)) {
          matchingHits.push_back(spacePoint);
        }
      }*/
    }

    bool useResult(const CKFCDCToVXDResultObject& currentResult) override
    {
      return true;
    }

  private:
    /// Cache
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Subfindlet: Filter
    TrackFindingCDC::ChooseableFilter<CDCTrackSpacePointCombinationFilterFactory> m_hitFilter;
  };
}