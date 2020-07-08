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
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ModuleParamList;
  class RecoTrack;

  /**
   * Simplified relation creator only creating relations between states of CDC Reco Track seeds and
   * states with SpacePoints, that:
   * (a) for the seed states: connect every seed with every lst hit of the SVD Reco Tracks
   * (b) for the hit states: are in the same SVD Reco Track and follow each other directly
   *
   * So if one travels the path of the relations, one would perfectly travel the path of one SVD track.
   */
  class RelationFromSVDTracksCreator : public TrackFindingCDC::Findlet<CKFToSVDState, CKFToSVDState,
    TrackFindingCDC::WeightedRelation<CKFToSVDState>> {
  public:
    /// The parent findlet
    using Super = TrackFindingCDC::Findlet<CKFToSVDState, CKFToSVDState,
          TrackFindingCDC::WeightedRelation<CKFToSVDState>>;

    /// Construct this findlet and add the subfindlet as listener
    RelationFromSVDTracksCreator();

    /// Require the store array
    void initialize() final;

    /// Default destructor
    ~RelationFromSVDTracksCreator();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Create relations between seeds and hits or hits and hits
    void apply(std::vector<CKFToSVDState>& seedStates, std::vector<CKFToSVDState>& states,
               std::vector<TrackFindingCDC::WeightedRelation<CKFToSVDState>>& relations) final;

  private:
    // Parameters
    /// Store Array name coming from VXDTF2
    std::string m_param_vxdTracksStoreArrayName = "VXDRecoTracks";
    /// Store Array name coming from CDCTF
    std::string m_param_cdcTracksStoreArrayName = "CDCRecoTracks";
    /// Store Array name of the space point track candidates coming from VXDTF2
    std::string m_param_spacePointTrackCandidateName = "SPTrackCands";

    // Object pools
    /// Store Array of the VXD tracks to use
    StoreArray<RecoTrack> m_vxdRecoTracks;
  };
}