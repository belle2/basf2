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
#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  class ModuleParamList;

  template<class AState, class ARelationFilter>
  class CKFRelationCreator : public TrackFindingCDC::Findlet<const AState, const AState, TrackFindingCDC::Relation<AState>> {
  public:
    using Super = TrackFindingCDC::Findlet<const AState, const AState, TrackFindingCDC::Relation<AState>>;

    /// Construct this findlet and add the subfindlet as listener
    CKFRelationCreator();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     */
    void apply(const std::vector<AState>& seedStates, const std::vector<AState>& states,
               std::vector<TrackFindingCDC::Relation<AState>>& relations) override;

  private:
    /// Subfindlet for the relation checking
    ARelationFilter m_filter;
  };
}