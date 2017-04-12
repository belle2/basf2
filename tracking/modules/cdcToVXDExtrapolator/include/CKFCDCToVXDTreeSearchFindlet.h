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
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDTreeSearchFindlet : public TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDStateObject> {
  public:
    using Super = TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDStateObject>;

    CKFCDCToVXDTreeSearchFindlet() : Super()
    {
      addProcessingSignalListener(&m_firstFilter);
      addProcessingSignalListener(&m_secondFilter);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "first"));
      m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "second"));
    }

    void apply(std::vector<RecoTrack*>& seedsVector,
               std::vector<const SpacePoint*>& filteredHitVector) final {

      m_cachedHitMap.clear();

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
      }
    }

  protected:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(Super::StateIterator currentState) final {
      const unsigned int nextLayer = currentState->getLastLayer() - 1;
      return m_cachedHitMap[nextLayer];
    }

    bool useResult(Super::StateIterator currentState) final {
      TrackFindingCDC::Weight weight = m_firstFilter(*currentState);
      if (std::isnan(weight))
      {
        return false;
      }

      currentState->advance();

      weight = m_secondFilter(*currentState);
      return not std::isnan(weight);
    }

  private:
    /// Cache
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Subfindlet: Filter
    TrackFindingCDC::ChooseableFilter<CDCTrackSpacePointCombinationFilterFactory> m_firstFilter {"simple"};
    TrackFindingCDC::ChooseableFilter<CDCTrackSpacePointCombinationFilterFactory> m_secondFilter;
  };
}