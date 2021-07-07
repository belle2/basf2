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
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

namespace Belle2 {
  class RecoTrack;
  class ModuleParamList;

  /// Relate the VTX and CDC tracks in the given relations also in the store array.
  class VTXRelationApplier : public
    TrackFindingCDC::Findlet<const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> {
  public:
    /// The parent findlet
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>;

    void initialize() final;

    /// Copy the relations to the store array
    void apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>&
               relationsCDCToVTX) final;

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
    std::string m_param_toRelationsStoreArrayName = "VTXRecoTracks";
  };
}
