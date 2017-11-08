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
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/ckf/general/findlets/SpacePointTagger.dcl.h>
#include <tracking/ckf/general/findlets/CKFDataHandler.dcl.h>
#include <tracking/ckf/general/findlets/StateCreator.dcl.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.dcl.h>
#include <tracking/ckf/general/findlets/TreeSearcher.dcl.h>
#include <tracking/ckf/general/findlets/OverlapResolver.dcl.h>
#include <tracking/ckf/svd/findlets/SVDStateRejecter.h>
#include <tracking/ckf/svd/findlets/UnusedVXDTracksAdder.h>
#include <tracking/ckf/svd/findlets/SpacePointLoader.h>

#include <tracking/ckf/svd/filters/relations/ChooseableSVDRelationFilter.h>
#include <tracking/ckf/svd/filters/results/SizeSVDResultFilter.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class SVDCluster;

  class ModuleParamList;


  class CKFToSVDSeedFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToSVDSeedFindlet();

    /// Default desctructor
    ~CKFToSVDSeedFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for retrieving the cdc tracks and writing the result out
    CKFDataHandler<CKFToSVDResult> m_dataHandler;
    /// Findlet for loading the space points
    SpacePointLoader m_hitsLoader;
    /// Findlet for creating states out of tracks
    StateCreator<RecoTrack, CKFToSVDState> m_stateCreatorFromTracks;
    /// Findlet for creating states out of hits
    StateCreator<const SpacePoint, CKFToSVDState> m_stateCreatorFromHits;
    /// Findlet for creating relations between states
    CKFRelationCreator<CKFToSVDState, ChooseableSVDRelationFilter> m_relationCreator;
    /// Findlet doing the main work: the tree finding
    TreeSearcher<CKFToSVDState, SVDStateRejecter, CKFToSVDResult> m_treeSearchFindlet;
    /// Findlet for resolving overlaps
    OverlapResolver<SizeSVDResultFilter> m_overlapResolver;
    /// Findlet for adding unused VXDTF2 results
    UnusedVXDTracksAdder m_unusedTracksAdder;
    /// Findlet for tagging the used space points
    SpacePointTagger<CKFToSVDResult, SVDCluster> m_spacePointTagger;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_cdcRecoTrackVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// States for the tracks
    std::vector<CKFToSVDState> m_seedStates;
    /// States for the hits
    std::vector<CKFToSVDState> m_states;
    /// Relations between states
    std::vector<TrackFindingCDC::WeightedRelation<CKFToSVDState>> m_relations;
    /// Vector for storing the results
    std::vector<CKFToSVDResult> m_results;
    /// Vector for storing the filtered results
    std::vector<CKFToSVDResult> m_filteredResults;
  };
}
