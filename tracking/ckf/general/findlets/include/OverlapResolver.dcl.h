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

namespace Belle2 {
  class ModuleParamList;

  template<class AFilter>
  class OverlapResolver : public TrackFindingCDC::Findlet<typename AFilter::Object> {
  public:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<typename AFilter::Object>;

    /// Construct this findlet and add the subfindlet as listener
    OverlapResolver();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     */
    void apply(std::vector<typename AFilter::Object>& results) override;

  private:
    /// Subfindlet for filtering
    AFilter m_filter;
  };
}