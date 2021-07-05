/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/svd/findlets/SpacePointLoader.h>
#include <tracking/ckf/general/findlets/TrackLoader.h>
#include <tracking/ckf/general/findlets/StateCreator.dcl.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.dcl.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.dcl.h>
#include <tracking/ckf/general/findlets/TreeSearcher.dcl.h>
#include <tracking/ckf/general/findlets/OverlapResolver.dcl.h>
#include <tracking/ckf/pxd/findlets/PXDStateRejecter.h>
#include <tracking/ckf/general/findlets/SpacePointTagger.dcl.h>
#include <tracking/ckf/general/findlets/ResultStorer.dcl.h>

#include <tracking/ckf/pxd/filters/relations/ChooseablePXDRelationFilter.h>
#include <tracking/ckf/pxd/filters/results/ChooseablePXDResultFilter.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <string>
#include <vector>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class PXDCluster;
  class CKFToPXDResult;
  class CKFToPXDState;

  class ModuleParamList;

  /**
   * Combinatorial Kalman Filter to extrapolate CDC Reco Tracks into the VXD (PXD) and collect space points.
   *
   * The implementation is split up in four parts and factored out into five parts.
   * * Fetch the SpacePoints and the reco tracks from the data store
   * * Construct all possible two-state-relations between hits and seeds or hits and hits.
   * * Construct all possible candidates starting from a RecoTrack and going through the layers of the VXD collecting
   *   space points.
   * * Find a non-overlapping set of results (only one candidate per seed) (OverlapResolverFindlet,
   *   quality is determined by a filter)
   * * Write the results out to the data store
   *
   */
  class CKFToPXDFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToPXDFindlet();

    /// Default desctructor
    ~CKFToPXDFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the tree search.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Parameters
    /// Minimal hit requirement for the results (counted in number of space points)
    unsigned int m_param_minimalHitRequirement = 1;
    /// Cut away tracks without SVD hits or not
    bool m_param_onlyUseTracksWithSVD = true;
    /// Reverse the seed
    bool m_param_reverseSeed = false;

    // Findlets
    /// Findlet for retrieving the cdc tracks and writing the result out
    TrackLoader m_dataHandler;
    /// Findlet for loading the space points
    SpacePointLoader m_hitsLoader;
    /// Findlet for creating states out of tracks
    StateCreatorWithReversal<CKFToPXDState> m_stateCreatorFromTracks;
    /// Findlet for creating states out of hits
    StateCreator<const SpacePoint, CKFToPXDState> m_stateCreatorFromHits;
    /// Findlet for creating relations between states
    CKFRelationCreator<CKFToPXDState, ChooseablePXDRelationFilter> m_relationCreator;
    /// Findlet doing the main work: the tree finding
    TreeSearcher<CKFToPXDState, PXDStateRejecter, CKFToPXDResult> m_treeSearchFindlet;
    /// Findlet for resolving overlaps
    OverlapResolver<ChooseablePXDResultFilter> m_overlapResolver;
    /// Findlet for tagging the used space points
    SpacePointTagger<CKFToPXDResult, PXDCluster> m_spacePointTagger;
    /// Findlet for storing the results
    ResultStorer<CKFToPXDResult> m_resultStorer;

    // Object pools
    /// Pointers to the Reco tracks as a vector
    std::vector<RecoTrack*> m_recoTracksVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// States for the tracks
    std::vector<CKFToPXDState> m_seedStates;
    /// States for the hits
    std::vector<CKFToPXDState> m_states;
    /// Relations between states
    std::vector<TrackFindingCDC::WeightedRelation<CKFToPXDState>> m_relations;
    /// Vector for storing the results
    std::vector<CKFToPXDResult> m_results;
    /// Vector for storing the filtered results
    std::vector<CKFToPXDResult> m_filteredResults;
  };
}
