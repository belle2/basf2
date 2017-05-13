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

#include <tracking/ckf/findlets/cdcToSpacePoint/CDCTrackSpacePointStoreArrayHandler.h>
#include <tracking/ckf/findlets/base/TreeSearchFindlet.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointHitSelector.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>
#include <tracking/ckf/findlets/base/OverlapResolverFindlet.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationFilterFactory.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>


namespace Belle2 {
  class CDCToSpacePointCKFFindlet : public TrackFindingCDC::Findlet<> {
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CDCToSpacePointCKFFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/git finding/merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for handling the store array access and write out
    CDCTrackSpacePointStoreArrayHandler m_storeArrayHandler;
    /// Findlet doing the main work: the tree finding
    TreeSearchFindlet<CKFCDCToVXDStateObject, CDCToSpacePointHitSelector> m_treeSearchFindlet;
    /// Findlet for resolving overlaps
    OverlapResolverFindlet<TrackFindingCDC::ChooseableFilter<CDCVXDTrackCombinationFilterFactory>> m_overlapResolver;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_cdcRecoTrackVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// Vector for storing the results
    std::vector<CKFCDCToVXDStateObject::ResultObject> m_results;
  };
}
