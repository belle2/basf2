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

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /// Findlet for applying filters for creating hit-hit and hit-seed relations
  template<class AState, class ASeedRelationFilter, class AHitRelationFilter = ASeedRelationFilter>
  class CKFRelationCreator : public TrackFindingCDC::Findlet<AState, AState, TrackFindingCDC::WeightedRelation<AState>> {
  public:
    using Super = TrackFindingCDC::Findlet<AState, AState, TrackFindingCDC::WeightedRelation<AState>>;

    /// Construct this findlet and add the subfindlet as listener
    CKFRelationCreator();

    /// Default destructor
    ~CKFRelationCreator();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Apply both filters for creating state-hit and hit-hit relations
    void apply(std::vector<AState>& seedStates, std::vector<AState>& states,
               std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations) override;

  private:
    /// Subfindlet for the relation checking between seed and hits
    ASeedRelationFilter m_seedFilter;
    /// Subfindlet for the relation checking between hits and hits
    AHitRelationFilter m_hitFilter;
  };
}