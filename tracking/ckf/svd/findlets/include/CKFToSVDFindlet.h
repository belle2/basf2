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
#include <tracking/ckf/svd/findlets/SpacePointLoader.h>

#include <tracking/ckf/svd/filters/relations/ChooseableSVDRelationFilter.h>
#include <tracking/ckf/svd/filters/results/ChooseableSVDResultFilter.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class SVDCluster;

  class ModuleParamList;

  /**
   * Combinatorial Kalman Filter to extrapolate CDC Reco Tracks into the VXD (SVD) and collect space points.
   *
   * The implementation is split up in four parts and factored out into three sub findlets.
   * * Fetch the SpacePoints and the reco tracks from the data store (CDCTrackSpacePointStoreArrayHandler)
   * * Construct all possible candidates starting from a RecoTrack and going through the layers of the VXD collecting
   *   space points (this is handles by the TreeSearchFindlet, which works on States. The selection
   *   of space points is handled by the CDCToSpacePointHitSelector)
   * * Find a non-overlapping set of results (only one candidate per space point and seed) (OverlapResolverFindlet,
   *   quality is determined by a filter)
   * * Write the results out to the data store (again the CDCTrackSpacePointStoreArrayHandler)
   *
   * If you want to reimplement this algorithm for a different purpose, you probably only have to
   * implement a new state class and a new hit selector (and maybe the store array handling). The rest should be taken
   * care by the framework.
   */
  class CKFToSVDFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToSVDFindlet();

    /// Default desctructor
    ~CKFToSVDFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Parameters
    /// Minimal hit requirement for the results (counted in number of space points)
    unsigned int m_param_minimalHitRequirement = 2;


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
    OverlapResolver<ChooseableSVDResultFilter> m_overlapResolver;
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
