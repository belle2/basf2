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

#include <tracking/modules/cdcToVXDExtrapolator/filterBased/StoreArrayMerger.h>
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CKFCDCToVXDTreeSearchFindlet.h>
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CKFCDCToVXDResultObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CDCToVXDExtrapolatorFindlet : public TrackFindingCDC::Findlet<> {
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CDCToVXDExtrapolatorFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/git finding/merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for handling the store array access and write out
    StoreArrayMerger m_storeArrayMerger;
    /// Findlet doing the main work: the tree finding
    CKFCDCToVXDTreeSearchFindlet m_treeSearchFindlet;

    // Parameters
    bool m_param_exportTracks = true;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_cdcRecoTrackVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// Vector for storing the results
    std::vector<std::pair<RecoTrack*, std::vector<const SpacePoint*>>> m_results;
  };
}