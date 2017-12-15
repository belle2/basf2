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
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

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
    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_writeOutDirectionAsString = "both";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_writeOutDirection = TrackFindingCDC::EForwardBackward::c_Unknown;
    /// Create relations from this store array.
    std::string m_param_fromRelationsStoreArrayName = "CDCRecoTracks";
    /// Create relations to this store array.
    std::string m_param_toRelationsStoreArrayName = "SVDRecoTracks";
  };
}