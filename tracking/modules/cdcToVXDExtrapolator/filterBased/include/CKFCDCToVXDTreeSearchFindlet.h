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

#include <tracking/ckf/findlets/base/TreeSearchFindlet.h>
#include <tracking/ckf/filters/base/LayerToggledFilter.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDTreeSearchFindlet : public TrackFindingCDC::TreeSearchFindlet<CKFCDCToVXDStateObject,
    LayerToggledFilter<CDCTrackSpacePointCombinationFilterFactory>> {
  public:
    using Super = TrackFindingCDC::TreeSearchFindlet<CKFCDCToVXDStateObject,
          LayerToggledFilter<CDCTrackSpacePointCombinationFilterFactory>>;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(Super::StateIterator currentState) override;

    bool fit(Super::StateIterator currentState) final;

    bool advance(Super::StateIterator currentState) final;

    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector) override;

  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;
    /// Parameter: which caching method to use
    bool m_param_useCachingOne = false;
    /// Parameter: use caching it all
    bool m_param_useCaching = false;

    std::vector<unsigned int> m_maximumLadderNumbers = {8, 12, 7, 10, 12, 16};
  };
}
