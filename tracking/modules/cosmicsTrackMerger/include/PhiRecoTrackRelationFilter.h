/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/modules/cosmicsTrackMerger/CellularRecoTrack.h>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// Relation filter that lets only possibilities with small phi distance pass
  class PhiRecoTrackRelationFilter : public TrackFindingCDC::RelationFilter<const CellularRecoTrack> {

  private:
    /// Type of the super class
    using Super = TrackFindingCDC::RelationFilter<const CellularRecoTrack>;

  public:
    /// Export all parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Implementation of the phi calculation.
    TrackFindingCDC::Weight operator()(const CellularRecoTrack& fromTrack, const CellularRecoTrack& toTrack) final;

    /// Copy the implementation from the base class
    using Super::operator();

  private:
    /// Parameter : The maximal deviation in phi between the trajectories of the track.
    double m_param_maximalPhiDistance = 0.2;
  };
}
