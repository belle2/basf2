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

#include <tracking/ckf/findlets/vxdToCDC/VXDToCDCMatcher.h>
#include <tracking/ckf/filters/vxdToCDC/result/VXDCDCTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/vxdToCDC/state/CKFVXDToWireHitStateFilterFactory.h>
#include <tracking/ckf/findlets/vxdToCDC/VXDTrackWireHitStoreArrayHandler.h>

#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>
#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>
#include <tracking/ckf/findlets/base/TrackFitterAndDeleter.h>
#include <tracking/ckf/findlets/base/TreeSearchFindlet.h>
#include <tracking/ckf/findlets/base/OverlapResolverFindlet.h>
#include <tracking/ckf/findlets/base/WireHitTagger.h>

#include <tracking/ckf/states/CKFStateObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  /**
   * Combinatorial Kalman Filter to extrapolate VXD Reco Tracks into the CDC and collect hits.
   */
  class VXDToCDCCKFFindlet : public TrackFindingCDC::Findlet<TrackFindingCDC::CDCWireHit&> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<TrackFindingCDC::CDCWireHit&>;

  public:
    /// Constructor, for setting module description and parameters.
    VXDToCDCCKFFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply(std::vector<TrackFindingCDC::CDCWireHit>& wireHits) override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for retrieving the vxd tracks
    TrackFindingCDC::StoreArrayLoader<RecoTrack> m_tracksLoader;
    /// Findlet for fitting the tracks
    TrackFitterAndDeleter m_trackFitter;
    /// Findlet doing the main work: the tree finding
    TreeSearchFindlet<RecoTrack, TrackFindingCDC::CDCRLWireHit, VXDToCDCMatcher, CKFVXDToWireHitStateFilterFactory, 56>
    m_treeSearchFindlet;
    /// Findlet for resolving overlaps
    OverlapResolverFindlet<VXDCDCTrackCombinationFilterFactory> m_overlapResolver;
    /// Findlet for handling the store array write out
    VXDTrackWireHitStoreArrayHandler<RecoTrack, TrackFindingCDC::CDCRLWireHit> m_storeArrayHandler;
    /// Findlet for tagging the used space points
    WireHitTagger<RecoTrack, TrackFindingCDC::CDCRLWireHit> m_wireHitTagger;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_vxdRecoTrackVector;
    /// RLWireHits as a vector
    std::vector<TrackFindingCDC::CDCRLWireHit> m_rlWireHitVector;
    /// Pointers to the RLWireHits as a vector
    std::vector<const TrackFindingCDC::CDCRLWireHit*> m_rlWireHitPointerVector;
    /// Vector for storing the results
    std::vector<CKFResult<RecoTrack, TrackFindingCDC::CDCRLWireHit>> m_results;
  };
}
