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

namespace Belle2 {
  class RecoTrack;
  class ModuleParamList;

  /// Relate the SVD and CDC tracks in the given relations also in the store array.
  class RelationApplier : public TrackFindingCDC::Findlet<const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> {
  public:
    /// The parent findlet
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>;

    void initialize() final;

    /// Copy the relations to the store array
    void apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>& relationsCDCToSVD) final;

    /// Expose parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

  private:
    // Parameters
    /// Write out the relations with a -1 as weight, indicating the reversal of the CDC track.
    bool m_param_reverseStoredRelations = false;
    /// Create relations from this store array.
    std::string m_param_fromRelationsStoreArrayName = "CDCRecoTracks";
    /// Create relations to this store array.
    std::string m_param_toRelationsStoreArrayName = "SVDRecoTracks";
  };
}