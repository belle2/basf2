/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <string>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {

  class RecoTrack;
  class MCParticle;
  class RelationArray;

  /** Module turning reco tracks to genfit trackcands (will be unneeded once we use reco tracks everywhere). */
  class GenfitTrackCandidatesCreatorModule : public Module {

  public:
    /** Initialize the module parameters. */
    GenfitTrackCandidatesCreatorModule();

    /** Initialize the needed StoreArrays and ensure they are created properly. */
    void initialize() override;

    /** Loop over all reco tracks and create a genfit track candidates. */
    void event() override;

  private:
    /** StoreArray name of the output genfit trackcands. */
    std::string m_param_genfitTrackCandsStoreArrayName = "TrackCands";
    /** StoreArray name of the input reco tracks. */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks";
    /** StoreArray name of the output MC genfit trackcands. */
    std::string m_param_mcGenfitTrackCandsStoreArrayName = "MCTrackCands";
    /** StoreArray name of the input reco MC tracks. */
    std::string m_param_mcRecoTracksStoreArrayName = "MCRecoTracks";
    /** Flag to turn on combined conversion of PR and MCTrackCands. Can only be used if the MCMatcher was used before. */
    bool m_param_outputMCTrackCandidates = false;

    /// Private helper to output both the MC and the PR reco tracks into TrackCands and make the relations correctly.
    void outputPRAndMCTrackCands(const StoreArray<RecoTrack>& prRecoTracks, StoreArray<genfit::TrackCand>& prGenfitTrackCands,
                                 const StoreArray<MCParticle>& mcParticles, RelationArray& relationsFromPRTrackCandsToPRRecoTracks,
                                 RelationArray& relationsFromPRTrackCandsToMCParticle) const;

    /// Private helper to output the PR reco tracks into TrackCands and make the relations correctly.
    void outputPRTrackCands(const StoreArray<RecoTrack>& recoTracks, StoreArray<genfit::TrackCand>& genfitTrackCands,
                            const StoreArray<MCParticle>& mcParticles, RelationArray& relationsFromTrackCandsToRecoTracks,
                            RelationArray& relationsFromTrackCandsToMCParticle) const;
  };
}

