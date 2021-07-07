/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/ckf/general/findlets/TrackLoader.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.dcl.h>
#include <tracking/ckf/general/findlets/TreeSearcher.dcl.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>
#include <tracking/ckf/vtx/findlets/VTXStateRejecter.h>
#include <tracking/ckf/svd/findlets/SpacePointLoader.h>
#include <tracking/ckf/vtx/findlets/RelationFromVTXTracksCreator.h>
#include <tracking/ckf/vtx/findlets/VTXRecoTrackRelator.h>
#include <tracking/ckf/vtx/findlets/VTXRelationApplier.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  class ModuleParamList;

  /**
   * Findlet for combining CDC tracks with VTX tracks.
   *
   * The idea is to use every fitted CDC track, extrapolate it to every
   * VTX track and extrapolate/Kalman fit it with every hit of the track.
   *
   * Then, a filter is applied to every CDC-VTX combination and the combinations
   * are resolved using this filter information.
   *
   * This module does only output relations - the combination
   * has to be done afterwards.
   */
  class CKFToVTXSeedFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToVTXSeedFindlet();

    /// Default desctructor
    ~CKFToVTXSeedFindlet();

    /// Expose the parameters (also of the sub findlets).
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track merging.
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Parameters
    /// Minimal hit requirement for the results (counted in number of space points)
    unsigned int m_param_minimalHitRequirement = 2;

    // Findlets
    /// Findlet for retrieving the cdc tracks and writing the result out
    TrackLoader m_dataHandler;
    /// Findlet for loading the space points
    SpacePointLoader m_hitsLoader;
    /// Findlet for creating states out of tracks
    StateCreatorWithReversal<CKFToVTXState> m_stateCreatorFromTracks;
    /// Findlet for creating states out of hits
    StateCreator<const SpacePoint, CKFToVTXState> m_stateCreatorFromHits;
    /// Relation Creator
    RelationFromVTXTracksCreator m_relationCreator;
    /// Findlet doing the main work: the tree finding
    TreeSearcher<CKFToVTXState, VTXStateRejecter, CKFToVTXResult> m_treeSearchFindlet;
    /// Findlet transforming the hit results to track relations.
    VTXRecoTrackRelator m_recoTrackRelator;
    /// Greedy filter for the relations between VTX and CDC Reco Tracks
    TrackFindingCDC::BestMatchSelector<const RecoTrack, const RecoTrack> m_bestMatchSelector;
    /// Copy the result relations to the store array
    VTXRelationApplier m_relationApplier;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_cdcRecoTracksVector;
    /// Pointers to the (const) SpacePoints as a vector
    std::vector<const SpacePoint*> m_spacePointVector;
    /// States for the tracks
    std::vector<CKFToVTXState> m_seedStates;
    /// States for the hits
    std::vector<CKFToVTXState> m_states;
    /// Relations between states
    std::vector<TrackFindingCDC::WeightedRelation<CKFToVTXState>> m_relations;
    /// Vector for storing the results
    std::vector<CKFToVTXResult> m_results;
    /// Relations between CDC tracks and VTX tracks
    std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> m_relationsCDCToVTX;
  };
}
