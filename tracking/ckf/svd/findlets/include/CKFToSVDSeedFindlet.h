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

#include <tracking/ckf/general/findlets/TrackLoader.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.dcl.h>
#include <tracking/ckf/general/findlets/TreeSearcher.dcl.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>
#include <tracking/ckf/svd/findlets/SVDStateRejecter.h>
#include <tracking/ckf/svd/findlets/SpacePointLoader.h>
#include <tracking/ckf/svd/findlets/RelationFromSVDTracksCreator.h>
#include <tracking/ckf/svd/findlets/RecoTrackRelator.h>
#include <tracking/ckf/svd/findlets/RelationApplier.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  class ModuleParamList;

  /**
   * Findlet for combining CDC tracks with SVD tracks.
   *
   * The idea is to use every fitted CDC track, extrapolate it to every
   * SVD track and extrapolate/Kalman fit it with every hit of the track.
   *
   * Then, a filter is applied to every CDC-SVD combination and the combinations
   * are resolved using this filter information.
   *
   * This module does only output relations - the combination
   * has to be done afterwards.
   */
  class CKFToSVDSeedFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToSVDSeedFindlet();

    /// Default desctructor
    ~CKFToSVDSeedFindlet();

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
    StateCreatorWithReversal<CKFToSVDState> m_stateCreatorFromTracks;
    /// Findlet for creating states out of hits
    StateCreator<const SpacePoint, CKFToSVDState> m_stateCreatorFromHits;
    /// Relation Creator
    RelationFromSVDTracksCreator m_relationCreator;
    /// Findlet doing the main work: the tree finding
    TreeSearcher<CKFToSVDState, SVDStateRejecter, CKFToSVDResult> m_treeSearchFindlet;
    /// Findlet transforming the hit results to track relations.
    RecoTrackRelator m_recoTrackRelator;
    /// Greedy filter for the relations between SVD and CDC Reco Tracks
    TrackFindingCDC::BestMatchSelector<const RecoTrack, const RecoTrack> m_bestMatchSelector;
    /// Copy the result relations to the store array
    RelationApplier m_relationApplier;

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
    /// Relations between CDC tracks and SVD tracks
    std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> m_relationsCDCToSVD;
  };
}
