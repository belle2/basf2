/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ModuleParamList;
  class RecoTrack;

  /**
   * Simplified relation creator only creating relations between states of CDC Reco Track seeds and
   * states with SpacePoints, that:
   * (a) for the seed states: connect every seed with every lst hit of the VTX Reco Tracks
   * (b) for the hit states: are in the same VTX Reco Track and follow each other directly
   *
   * So if one travels the path of the relations, one would perfectly travel the path of one VTX track.
   */
  class RelationFromVTXTracksCreator : public TrackFindingCDC::Findlet<CKFToVTXState, CKFToVTXState,
    TrackFindingCDC::WeightedRelation<CKFToVTXState>> {
  public:
    /// The parent findlet
    using Super = TrackFindingCDC::Findlet<CKFToVTXState, CKFToVTXState,
          TrackFindingCDC::WeightedRelation<CKFToVTXState>>;

    /// Construct this findlet and add the subfindlet as listener
    RelationFromVTXTracksCreator();

    /// Require the store array
    void initialize() final;

    /// Default destructor
    ~RelationFromVTXTracksCreator();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Create relations between seeds and hits or hits and hits
    void apply(std::vector<CKFToVTXState>& seedStates, std::vector<CKFToVTXState>& states,
               std::vector<TrackFindingCDC::WeightedRelation<CKFToVTXState>>& relations) final;

  private:
    // Parameters
    /// Store Array name coming from VXDTF2
    std::string m_param_vtxTracksStoreArrayName = "VTXRecoTracks";
    /// Store Array name coming from CDCTF
    std::string m_param_cdcTracksStoreArrayName = "CDCRecoTracks";
    /// Store Array name of the space point track candidates coming from VXDTF2
    std::string m_param_spacePointTrackCandidateName = "SPTrackCands";

    // Object pools
    /// Store Array of the VXD tracks to use
    StoreArray<RecoTrack> m_vxdRecoTracks;
  };
}
