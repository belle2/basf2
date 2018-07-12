/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Forst, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/modules/cosmicsTrackMerger/PhiRecoTrackRelationFilter.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/WithAutomatonCell.h>

#include <tracking/modules/cosmicsTrackMerger/CellularRecoTrack.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>

#include <vector>
#include <string>

namespace Belle2 {
  /// Links tracks based on a filter criterion. It is based on the TrackLinker in the TF_CDC package
  class CosmicsTrackMergerFindlet : public TrackFindingCDC::Findlet<> {
  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor adding the filter as a subordinary processing signal listener.
    CosmicsTrackMergerFindlet();

    /// Short description of the findlet
    std::string getDescription() final;

    /// Expose the parameters to a module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Init the store arrays
    void initialize() final;

    /// Main algorithm
    void apply() final;

  private:
    /// Creator of the track relations for linking
    TrackFindingCDC::WeightedRelationCreator<const CellularRecoTrack, PhiRecoTrackRelationFilter> m_trackRelationCreator;

    /// Instance of the cellular automaton path finder
    TrackFindingCDC::MultipassCellularPathFinder<const CellularRecoTrack> m_cellularPathFinder;

    /// Memory for the relations between tracks to be followed on linking
    std::vector<TrackFindingCDC::WeightedRelation<const CellularRecoTrack> > m_trackRelations;

    /// Memory for the track paths generated from the graph.
    std::vector<TrackFindingCDC::Path<const CellularRecoTrack>> m_trackPaths;

    /// Storage for the input tracks
    std::vector<CellularRecoTrack> m_inputTrackVector;

    /// StoreArray for the output tracks
    StoreArray<RecoTrack> m_outputTracks;

    /// StoreArray for the input tracks
    StoreArray<RecoTrack> m_inputTracks;

    // Parameters
    /// Parameter for the input reco tracks
    std::string m_param_inputRecoTracks = "UnmergedRecoTracks";

    /// Parameter for the output reco tracks
    std::string m_param_outputRecoTracks = "RecoTracks";
  };
}
