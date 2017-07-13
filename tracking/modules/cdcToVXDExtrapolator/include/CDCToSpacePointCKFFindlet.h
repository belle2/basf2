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
#include <tracking/ckf/findlets/base/HitSelector.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointMatcher.h>
#include <tracking/ckf/findlets/base/OverlapTeacher.h>
#include <tracking/ckf/findlets/base/OverlapResolverFindlet.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointTagger.h>

#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationTruthVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectFilterFactory.h>

#include <tracking/ckf/utilities/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>


namespace Belle2 {
  /**
   * Combinatorial Kalman Filter to extrapolate CDC Reco Tracks into the VXD (SVD) and collect space points.
   *
   * The implementation is split up in four parts and factored out into three sub findlets.
   * * Fetch the SpacePoints and the reco tracks from the data store (CDCTrackSpacePointStoreArrayHandler)
   * * Construct all possible candidates starting from a RecoTrack and going through the layers of the VXD collecting
   *   space points (this is handles by the TreeSearchFindlet, which works on CKFCDCToVXDStateObject. The selection
   *   of space points is handled by the CDCToSpacePointHitSelector)
   * * Find a non-overlapping set of results (only one candidate per space point and seed) (OverlapResolverFindlet,
   *   quality is determined by a filter)
   * * Write the results out to the data store (again the CDCTrackSpacePointStoreArrayHandler)
   *
   * If you want to reimplement this algorithm for a different purpose, you probably only have to
   * implement a new state class and a new hit selector (and maybe the store array handling). The rest should be taken
   * care by the framework.
   */
  class CDCToSpacePointCKFFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CDCToSpacePointCKFFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Some typedefs
    using CDCToSpacePointHitSelector = HitSelector<CKFCDCToSpacePointStateObjectFilterFactory>;

    // Findlets
    /// Findlet for retrieving the cdc tracks
    TrackFindingCDC::StoreArrayLoader<RecoTrack> m_cdcTracksLoader;
    /// Findlet for retrieving the hits
    TrackFindingCDC::StoreArrayLoader<const SpacePoint> m_hitsLoader;
    /// Findlet doing the main work: the tree finding
    TreeSearchFindlet<CKFCDCToVXDStateObject, CDCToSpacePointMatcher, CDCToSpacePointHitSelector> m_treeSearchFindlet;
    /// Findlet for adding truth information before the overlap finder
    OverlapTeacher<CDCVXDTrackCombinationTruthVarSet> m_overlapTeacher;
    /// Findlet for resolving overlaps
    OverlapResolverFindlet<TrackFindingCDC::ChooseableFilter<CDCVXDTrackCombinationFilterFactory>> m_overlapResolver;
    /// Findlet for handling the store array write out
    CDCTrackSpacePointStoreArrayHandler<CKFCDCToVXDStateObject::ResultObject> m_storeArrayHandler;
    /// Findlet for tagging the used space points
    SpacePointTagger<CKFCDCToVXDStateObject::ResultObject, SVDCluster> m_svdSpacePointTagger;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_cdcRecoTrackVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// Vector for storing the results
    std::vector<CKFCDCToVXDStateObject::ResultObject> m_results;
  };
}
